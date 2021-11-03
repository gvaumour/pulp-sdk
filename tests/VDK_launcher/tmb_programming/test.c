/* 
 * Copyright (C) 2021 ATOS
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Gregory Vaumourin, Atos/Bull (gregory.vaumourin@atos.net)
 */
#include "pmsis.h"
#include "stdio.h"

#define END_SIM_ADDRESS 0x2FFFFFFF

#define SW_RESET        0x190
#define SW_CLK_ENABLE   0x194
#define RESET_ASYNC_SET 0xb00
#define RESET_ASYNC_CLR 0xb04
#define RESET_ASYNC     0xb08



#define Q_TMB_OFFSET 0x0D200000
#define PE_TMB_OFFSET(index)  (0x4000*(index+1)+ Q_TMB_OFFSET)
#define Q_TMB_ACCESS(address) (* ((uint32_t*) (Q_TMB_OFFSET + address)))
#define PE_TMB_ACCESS(index, address) (*((uint32_t*) (PE_TMB_OFFSET(index) + address)))


int test_entry()
{
  printf("###### Configuring the Q_TMB\n");

  Q_TMB_ACCESS(SW_RESET) = 0xFFFE;
  Q_TMB_ACCESS(SW_CLK_ENABLE) = 0xFFFF;


  // Setting up the output clocks 

  // Releasing the reset 
  Q_TMB_ACCESS

  // Configure each TMB of PE_TMB
  for (int i = 0; i < 17 ; i++) {

  }

  *(volatile int *)END_SIM_ADDRESS = 1; // Will stop the systemC simulation 

  return 0;
}

void test_kickoff(void *arg)
{
  int ret = test_entry();
  pmsis_exit(ret);
}

int main()
{
  return pmsis_kickoff((void *)test_kickoff);
}
