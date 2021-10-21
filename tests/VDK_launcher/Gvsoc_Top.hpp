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

#ifndef __GVSOC_TOP_HPP__
#define __GVSOC_TOP_HPP__

#include <vector>
#include <mutex>
#include <condition_variable>

#include <scml.h>
#include <scml2.h>

#include <tlm.h> 
//#include <tlm_utils/simple_initiator_socket.h>
//#include <tlm_utils/simple_target_socket.h>

#include "vp/vp.hpp"
#include "vp/itf/io.hpp"
#include "gv/gvsoc.hpp"
#include "ems_mm.h"


#define BYTES_PER_ACCESS    64
#define ACCEPT_DELAY_PS     1000

#ifdef IS_STANDALONE_EXEC
#define END_SIM_ADDRESS     0x2FFFFFFF
#endif

class ThreadSafeEventIf : public sc_interface {
    virtual void notify(sc_time delay = SC_ZERO_TIME) = 0;
    virtual const sc_event &default_event(void) const = 0;
protected:
    virtual void update(void) = 0;
};

/*
 * This class is to able to send systemc events from different POSIX threads 
 * without trouble for the systemc simulation 
 */ 
class ThreadSafeEvent : public sc_prim_channel, public ThreadSafeEventIf {
public:
    ThreadSafeEvent(const char *name = ""): event(name) {}

    void notify(sc_time delay = SC_ZERO_TIME) {
        this->delay = delay;
        async_request_update();
    }

    const sc_event &default_event(void) const {
        return event;
    }
protected:
    virtual void update(void) {
        event.notify(delay);
    }
    sc_event event;
    sc_time delay;
};

class Gvsoc_Top : public sc_module, public gv::Io_user
{

	public: 

		SC_HAS_PROCESS(Gvsoc_Top);
		Gvsoc_Top(const sc_module_name name);

		void gvsoc2vdk_process();

		#ifdef IS_STANDALONE_EXEC
		void dummy_thread();
		#endif

		/* API to gv::Io_user */ 
		void access(gv::Io_request *req);
		void grant(gv::Io_request *req);
		void reply(gv::Io_request *req);

		/* API for TLM sockets */ 
		void invalidate_direct_mem_ptr(sc_dt::uint64 start_range, sc_dt::uint64 end_range);
		void b_transport(tlm::tlm_generic_payload& tr, sc_time& delay);
		unsigned int transport_dbg(tlm::tlm_generic_payload& tr);
		bool get_direct_mem_ptr(tlm::tlm_generic_payload& tr, tlm::tlm_dmi& dmi_data);

		/* Util functions */ 
		void req_to_gp(gv::Io_request *r, tlm::tlm_generic_payload *p, uint32_t tid, bool last);
		void gp_to_req(gv::Io_request *r, tlm::tlm_generic_payload *p, bool is_reply);

		// Incoming requests from Gvsoc or from VDK are enqueued in these arrays
		std::vector<gv::Io_request *> gvsoc2vdk_pending_requests;
		std::vector<gv::Io_request *> vdk2gvsoc_pending_requests;

		scml2::simple_target_socket<Gvsoc_Top, 32, tlm::tlm_base_protocol_types> target_socket;
		scml2::simple_initiator_socket<Gvsoc_Top, 32, tlm::tlm_base_protocol_types> initiator_socket;

		std::string m_config_file;
		gv::Gvsoc *gvsoc;
		gv::Io_binding *gvsoc_binding;
		ems::mm mm;
		std::mutex mutex, mutex1;

		ThreadSafeEvent event_access;

};

#endif 
