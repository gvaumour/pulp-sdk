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

// Clocks Register Configuration
#define CLK_CGM_0_HL          0x80c
#define CLK_CDM_0_GGCM_ENABLE 0x40c
#define CLK_CDM_0_HL          0x418
#define PLL_0_DIVIDERS        0x304
#define PLL_0_MODE            0x300

#define OUTER_WILDS  0x20000000
#define Q_TMB_OFFSET 0x0D200000 + OUTER_WILDS
#define PE_TMB_OFFSET(index)  (0x4000*(index+1)+ Q_TMB_OFFSET)
#define Q_TMB_ACCESS(address) (* ((uint32_t*) (Q_TMB_OFFSET + address)))
#define PE_TMB_ACCESS(index, address) (*((uint32_t*) (PE_TMB_OFFSET(index) + address)))


int test_entry()
{

  uint32_t data = 0x0;

  printf("###### Configuring the Q_TMB\n");

  Q_TMB_ACCESS(SW_RESET) = 0x0;
  printf("Q_TMB_ACCESS(SW_RESET) = %d\n", Q_TMB_ACCESS(SW_RESET));
  Q_TMB_ACCESS(SW_CLK_ENABLE) = 0xFFFF;
  printf("Q_TMB_ACCESS(SW_CLK_ENABLE) = %d\n", Q_TMB_ACCESS(SW_CLK_ENABLE));
 
  // Setting up the output clocks
  Q_TMB_ACCESS(CLK_CDM_0_HL) = 0x10000; //CGM_7.h=1/l=0
  Q_TMB_ACCESS(CLK_CGM_0_HL) = 0x10000; //CGM_7.h=1/l=0
  Q_TMB_ACCESS(CLK_CDM_0_GGCM_ENABLE) =  0x1; //CDM_1_GGCM.enable=true


  // Configuring the PLL 
  data = 1 //Refdiv, ref is 100M
         | 20 << 6   //FB div, aiming for 2G PLL out
         | 1  << 21  // Postdiv 1
         | 1  << 24; // Postdiv2
  Q_TMB_ACCESS(PLL_0_DIVIDERS) = data;
 
  data = 1<<3 | 1<<2 | 1 ; //all enables, no bypass
  Q_TMB_ACCESS(PLL_0_MODE) = data;

  // Releasing the reset 
  Q_TMB_ACCESS(RESET_ASYNC_SET) = 0x1FFFF ; // Setting the 17 resets of PE_TMB
  

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
