/* 
 * Copyright (C) 2017 ETH Zurich, University of Bologna and GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */

#include "pmsis.h"
#include "stdio.h"

int test_entry()
{
  *(volatile int *)0x1c000000 = 0;

  printf("###### Testing Writing to AXI_PROXY\n");
  for (int i=0; i<3; i++)
  {
      *(volatile int *)0x20000000 = 0x12345678 + i;

      while(*(volatile int *)0x1c000000 == 0);
      
      volatile uint32_t* a = (volatile uint32_t *)0x20000000;
      printf("PULP reading at addr 0x20000000 = 0x%x\n", *a);

      *(volatile int *)0x1c000000 = 0;
  }

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
