/*
 * Copyright (C) 2021 ATOS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Authors: Gregory Vaumourin (gregory.vaumourin@atos.net)
 */

#include "Gvsoc_Top.hpp"


using namespace std;
using namespace sc_core;


Gvsoc_Top::Gvsoc_Top(const sc_module_name name) :
	sc_module(name)
{
        initiator_socket.register_invalidate_direct_mem_ptr(this, &Gvsoc_Top::invalidate_direct_mem_ptr);
        target_socket.register_b_transport(this, &Gvsoc_Top::b_transport);
        target_socket.register_transport_dbg( this, &Gvsoc_Top::transport_dbg);
        target_socket.register_get_direct_mem_ptr( this, &Gvsoc_Top::get_direct_mem_ptr);

	//m_config_file = config_path;
	m_config_file = std::string(getenv("VDK_CONFIG_FILE"));
	assert(m_config_file != ""); 
	
	
	#ifdef IS_STANDALONE_EXEC
		SC_THREAD(dummy_thread);
	#endif

	SC_THREAD(gvsoc2vdk_process);
	sensitive << event_access;
	   
	// Open GVSOC and create the connection to the AXI proxy so that we can exchange IO requests
	// with Pulp side
	gvsoc = gv::gvsoc_new();
	gvsoc->open(m_config_file);
	gvsoc_binding = gvsoc->io_bind(this, "/chip/axi_proxy", "");
}


#ifdef IS_STANDALONE_EXEC

/* 
 * This thread does nothing but prevent the SystemC simulation
 * from stopping when simulation only progresses in GVSoC
 * To stop the simulation, the GVSoC part needs to write to END_SIM_ADDRESS
 */
void Gvsoc_Top::dummy_thread() {

	for(;;)
	{
		wait(sc_time(5, SC_MS));
	}

}
#endif

void Gvsoc_Top::gvsoc2vdk_process()
{
	tlm::tlm_generic_payload *p;
	tlm::tlm_phase phase;
	sc_time delay = SC_ZERO_TIME;
	tlm::tlm_sync_enum status;

	unsigned char *data = NULL;
	size_t dlen = 0;

	// Run the simulation and wait for incoming requests
	gvsoc->run();

	// Main process is to wait for request from GVSoC and forward them to VDK 
    for (;;)
    {
        sc_core::wait();

        std::unique_lock<std::mutex> lock(this->mutex);
        gv::Io_request *req = this->gvsoc2vdk_pending_requests.front();
        this->gvsoc2vdk_pending_requests.pop_back();
        lock.unlock();

		// A request corresponds to one or more transactions
		uint32_t nt = req->size / BYTES_PER_ACCESS;
		uint32_t n_trans = nt > 0 ? nt : 1;
		// std::cout << "Received io_request converted into " << n_trans << " transaction(s)" << std::endl;

		#ifdef IS_STANDALONE_EXEC
		if (req->addr == (uint64_t) END_SIM_ADDRESS)
			sc_stop();
		#endif

		for (auto t = 0; t < n_trans; t++) {

			// Get a generic payload from memory manager
			p = mm.palloc();
			p->acquire();

			// Convert packet to tlm generic payload
			bool last = (t == (n_trans - 1));
			req_to_gp(req, p, t, last);
			initiator_socket->b_transport(*p, delay);

			// Convert tlm transaction back to io_req 
			if (p->get_command() == tlm::TLM_READ_COMMAND)
				gp_to_req(req , p, true);
		}

        // Send the reply to the request, this will unblock the riscv core
        gvsoc_binding->reply(req);
    }

    gvsoc->stop();
    gvsoc->close();
}

void Gvsoc_Top::access(gv::Io_request *req)
{
    // Since we cannot call GVSOC API from callbacks, just enqueue the request
    std::unique_lock<std::mutex> lock(this->mutex);
    this->gvsoc2vdk_pending_requests.push_back(req);
    printf("Received access from Pulp side addr: 0x%lx, size: %ld, is_write: %d, value: 0x%x\n", req->addr, req->size, req->is_write, *(uint32_t *)req->data);
    lock.unlock();
	
	event_access.notify();
}

void Gvsoc_Top::grant(gv::Io_request *req)
{
	cout << name() << ": grant function is called, don't know what to do !!!!" << endl;
}

void Gvsoc_Top::reply(gv::Io_request *req)
{
	// Called when Pulp side has replied to our request, just delete it
    //printf("Received reply from Pulp side addr: 0x%lx, size: %ld, is_write: %d, value: 0x%x\n", req->addr, req->size, req->is_write, *(uint32_t *)req->data);

	int i = 0;
    std::unique_lock<std::mutex> lock(this->mutex1);
	for (auto io_req : vdk2gvsoc_pending_requests) {
		if (io_req == req) {
			/*
			std::cout << name() << ": Received reply from Pulp side addr=0x" << std::hex << req->addr << std::dec << std::endl;
			if (req->retval == gv::Io_request_ok)
				std::cout << name() << ": Io_request ok" << std::endl;
			else
				std::cout << name() << ": Io_request NOT ok" << std::endl;
			*/
			if (!io_req->is_write)

			vdk2gvsoc_pending_requests.erase(vdk2gvsoc_pending_requests.begin() + i);
		    break;
		}
		i++;
	}
    lock.unlock();
}

void Gvsoc_Top::req_to_gp(gv::Io_request *r, tlm::tlm_generic_payload *p, uint32_t tid, bool last)
{
	assert(r);
	assert(p);
	assert(p->has_mm());

	tlm::tlm_command cmd = r->is_write ? tlm::TLM_WRITE_COMMAND : tlm::TLM_READ_COMMAND;
	string a = r->is_write ? "TLM_WRITE_COMMAND" : "TLM_READ_COMMAND";
	uint32_t offset = tid * BYTES_PER_ACCESS;
	sc_dt::uint64 addr = r->addr + offset;
	
	unsigned char *data = r->data;
	unsigned int dlen = BYTES_PER_ACCESS;
	unsigned char *dptr = (unsigned char *) malloc(dlen);//&data[offset];
	memcpy(dptr, data, dlen);

	p->set_command(cmd);
	p->set_address(addr);
	p->set_data_ptr(dptr);
	p->set_data_length(dlen);
	p->set_streaming_width(dlen);
	p->set_byte_enable_ptr(nullptr);
	p->set_dmi_allowed(false);
	p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

}

void Gvsoc_Top::gp_to_req(gv::Io_request *r, tlm::tlm_generic_payload *p, bool is_reply)
{
	assert(r);
	assert(p);
	assert(p->has_mm());

	r->addr = p->get_address();
	r->size = p->get_data_length();

	// In case of a reply, the r->data buffer is already allocated, it should not be changed
	if (!is_reply)
		r->data = new uint8_t(r->size);
	memcpy(r->data, p->get_data_ptr() , r->size);

	if (p->get_command() ==  tlm::TLM_WRITE_COMMAND)
		r->is_write = true;
	else if (p->get_command() == tlm::TLM_READ_COMMAND)
		r->is_write = false; 
	else 
		assert(false && "TLM Command not supported");

}

void Gvsoc_Top::invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range)
{
}

void Gvsoc_Top::b_transport(tlm::tlm_generic_payload& tr, sc_time& delay)
{	
    gv::Io_request *io_req = new gv::Io_request();

    gp_to_req(io_req, &tr, false);
    
    
	// Enqueue the request and wait for reply, meanwhile the VDK side is stucked
    this->vdk2gvsoc_pending_requests.push_back(io_req);
	gvsoc_binding->access(io_req);

	printf("Received access from VDK addr: 0x%lx, size: %ld, is_write: %d, value: 0x%x\n", io_req->addr, io_req->size, io_req->is_write, *(uint32_t *)io_req->data);

	tr.set_response_status(tlm::TLM_OK_RESPONSE);
}

unsigned int Gvsoc_Top::transport_dbg(tlm::tlm_generic_payload& tr)
{
	return 0;
}

bool Gvsoc_Top::get_direct_mem_ptr(tlm::tlm_generic_payload& tr, tlm::tlm_dmi& dmi_data)
{
	dmi_data.allow_none();
	return false;
}


