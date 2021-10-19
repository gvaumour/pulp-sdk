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
#include <string>
#include <tlm.h>
#include <tlm_utils/simple_target_socket.h>

#include <systemc>
#include "Gvsoc_Top.hpp"
#include "TlmLogger.hpp"

int sc_main(int argc, char* argv[])
{
    char *config_path = NULL;
    if (argc != 2) {
        fprintf(stderr, "Wrong format parameters\n");
        return -1;
    }

    config_path = argv[1];
    if (config_path == NULL)
    {
        fprintf(stderr, "No configuration specified, please specify through option --config=<config path>.\n");
        return -1;
    }
    TlmLogger logger("logger");
    Gvsoc_Top top("top", string(config_path) );

/*
    scml_clock pclk {"pclk", sc_time(2, SC_NS)};
    sc_signal<bool> preset;
*/

    top.initiator_socket.bind(logger.in);
    logger.out.bind(top.target_socket);
/*
    top.p_clk(pclk);
    top.p_reset(preset);
*/
    sc_start();
    std::cout << "Main function , leaving application " << std::endl;
    return 0;
 }
