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

#ifndef __TLM_LOGGER_HPP__
#define __TLM_LOGGER_HPP__

#include <string>
#include <stdint.h>
#include <tlm.h>

#include <scml2.h>

#include "../src/ems_mm.h"

class TlmLogger : public sc_core::sc_module
{

    public:
        scml2::simple_target_socket<TlmLogger, 32, tlm::tlm_base_protocol_types> in;
        scml2::simple_initiator_socket<TlmLogger, 32, tlm::tlm_base_protocol_types> out;

        unsigned char data_buffer[4];

        ems::mm mm;

        std::map<uint64_t, uint32_t> map_registers;

        SC_HAS_PROCESS(TlmLogger);

        TlmLogger(const sc_module_name& name)
                : sc_module(name)
        {
                in.register_b_transport       (this, &TlmLogger::b_transport);
                in.register_transport_dbg     (this, &TlmLogger::transport_dbg);
                in.register_get_direct_mem_ptr(this, &TlmLogger::get_direct_mem_ptr);
        }

        void b_transport( tlm::tlm_generic_payload& tr, sc_time& delay )
        {
                uint64_t address= tr.get_address();
                tlm::tlm_command cmd = tr.get_command();
                std::string cmd_str = "UNDEFINED";
                unsigned char *data = tr.get_data_ptr();

                if (cmd ==  tlm::TLM_WRITE_COMMAND) {
                        cmd_str = "TLM_WRITE_COMMAND";
                }
                else if (cmd == tlm::TLM_READ_COMMAND) {
                        cmd_str = "TLM_READ_COMMAND";
                }

                if (map_registers.count(address) == 0) {
                        uint32_t buffer = cmd == tlm::TLM_WRITE_COMMAND ? *(uint32_t *)data : 0;
                        map_registers.insert(pair<uint64_t,uint32_t>(address, buffer));
                }

                if (cmd == tlm::TLM_READ_COMMAND){
                        memcpy(data_buffer , &(map_registers[address]), sizeof(uint32_t));
                        tr.set_data_ptr(data_buffer);
                        tr.set_data_length(4);
               }
 
                std::cout << name() << ": Received blocking access addr=0x" << std::hex << address << ", data = 0x" \
                      << *(uint32_t *)data << std::dec << ", cmd = " << cmd << std::endl;
                tr.set_response_status(tlm::TLM_OK_RESPONSE);
        }

        unsigned int transport_dbg( tlm::tlm_generic_payload& tr )
        {
                uint64_t address = tr.get_address();
                cout << name() << ": Debug Access@0x" << std::hex << address << std::endl;
                tr.set_response_status(tlm::TLM_OK_RESPONSE);
                return 4;
        }

        bool get_direct_mem_ptr(tlm::tlm_generic_payload& tr, tlm::tlm_dmi& dmi_data)
        {
                dmi_data.allow_none();
                return false;
        }
};

#endif

