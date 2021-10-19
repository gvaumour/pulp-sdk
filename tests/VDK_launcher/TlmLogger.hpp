/**
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
#include <string>
#include <stdint.h>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include "ems_mm.h"

class TlmLogger : public sc_core::sc_module
{

    public:
        tlm_utils::simple_target_socket<TlmLogger> in;
        tlm_utils::simple_initiator_socket<TlmLogger> out;

        unsigned char data_buffer[4];
        const unsigned char data_buffer_const[4] = {0x0, 0x1, 0x2, 0x3};

        ems::mm mm;
        sc_core::sc_event event_start_write{"write_event"};

        SC_HAS_PROCESS(TlmLogger);

        TlmLogger(const sc_module_name& name)
                : sc_module(name)
        {
                in.register_b_transport       (this, &TlmLogger::b_transport);
                in.register_transport_dbg     (this, &TlmLogger::transport_dbg);
                in.register_get_direct_mem_ptr(this, &TlmLogger::get_direct_mem_ptr);

                SC_THREAD(generate_traffic);
                sensitive << event_start_write;
        }

        void b_transport( tlm::tlm_generic_payload& tr, sc_time& delay )
        {
                uint64_t address= tr.get_address();
                std::string cmd = "UNDEFINED";
                if (tr.get_command() ==  tlm::TLM_WRITE_COMMAND) {
                        cmd = "TLM_WRITE_COMMAND";
                        unsigned char *data = tr.get_data_ptr();
                        memcpy(data_buffer, data, 4);
                }
                else if (tr.get_command() == tlm::TLM_READ_COMMAND) {
                        cmd = "TLM_READ_COMMAND";
                        memcpy(data_buffer, data_buffer_const, 4);
                        tr.set_data_ptr(data_buffer);
                        tr.set_data_length(4);
                }
                unsigned char *data = tr.get_data_ptr();

                std::cout << name() << ": Blocking access addr=0x" << std::hex << address << ", data = 0x" << std::hex << *(uint32_t *)data << std::dec << ", cmd = " << cmd << std::endl;
                tr.set_response_status(tlm::TLM_OK_RESPONSE);

                if (tr.get_command() == tlm::TLM_WRITE_COMMAND)
                        event_start_write.notify();
        }

        unsigned int transport_dbg( tlm::tlm_generic_payload& tr )
        {
                uint64_t address = tr.get_address();
                cout << name() << ":Debug Access@" << std::hex << address << std::endl;
                tr.set_response_status(tlm::TLM_OK_RESPONSE);
                return 4;
        }

        bool get_direct_mem_ptr(tlm::tlm_generic_payload& tr, tlm::tlm_dmi& dmi_data)
        {
                dmi_data.allow_none();
                return false;
        }

        void generate_traffic()
        {
                for(;;){
                        wait(event_start_write);
                        tlm::tlm_generic_payload *p;
                        sc_time delay = SC_ZERO_TIME;

                        std::cout << name() << ": Sending request " << std::endl;
                        p = mm.palloc();
                        p->acquire();

                        unsigned char data[4] = {0xef, 0xbe, 0xad, 0xde};
                        p->set_command(tlm::TLM_WRITE_COMMAND);
                        p->set_address(0x1c000000);
                        p->set_data_ptr(data);
                        p->set_data_length(4);
                        p->set_byte_enable_ptr(nullptr);
                        p->set_dmi_allowed(false);
                        p->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

                        out->b_transport(*p, delay);
                        if (p->get_response_status() == tlm::TLM_OK_RESPONSE) 
                                std::cout << name() << ": Response is OK" << std::endl;
                        else 
                                std::cout << name() << ": Response is not OK" << std::endl;

                }
        }

};

