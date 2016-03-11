/*************************************************************************
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
* All rights reserved. All use of this software and documentation is     *
* subject to the License Agreement located at the end of this file below.*
**************************************************************************
* Description:                                                           *
* The following is a simple hello world program running MicroC/OS-II.The * 
* purpose of the design is to be a very simple application that just     *
* demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
* for issues such as checking system call return codes. etc.             *
*                                                                        *
* Requirements:                                                          *
*   -Supported Example Hardware Platforms                                *
*     Standard                                                           *
*     Full Featured                                                      *
*     Low Cost                                                           *
*   -Supported Development Boards                                        *
*     Nios II Development Board, Stratix II Edition                      *
*     Nios Development Board, Stratix Professional Edition               *
*     Nios Development Board, Stratix Edition                            *
*     Nios Development Board, Cyclone Edition                            *
*   -System Library Settings                                             *
*     RTOS Type - MicroC/OS-II                                           *
*     Periodic System Timer                                              *
*   -Know Issues                                                         *
*     If this design is run on the ISS, terminal output will take several*
*     minutes per iteration.                                             *
**************************************************************************/


#include <stdio.h>
#include "includes.h"
#include "altera_up_avalon_rs232.h"
#include"altera_up_avalon_rs232_regs.h"
#include <system.h>

/* Definition of Task Stacks */
#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK taskRS232_stk[TASK_STACKSIZE];
OS_STK RS232Q_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY      2
#define TASK2_PRIORITY      3
#define TASKRS232_PRIORITY 1

#define SW_READ 1
#define SW_WRITE 2
#define WRITE_FIFO_EMPTY 0x80
#define READ_FIFO_EMPTY 0x0

OS_EVENT *RS232Q;
/* UART task: read and write */

void taskRS232(void* pdata) {
	alt_u32 write_FIFO_space;
	alt_u16 read_FIFO_used;
	alt_u8 data_W8;
	alt_u8 data_R8;
	int enter = 0;
	unsigned p_error;
	alt_up_rs232_dev* rs232_dev;
	// open the RS232 UART port
	rs232_dev = alt_up_rs232_open_dev("/dev/rs232_0");
	if (rs232_dev == NULL)
		alt_printf("Error: could not open RS232 UART\n");
	else
		alt_printf("Opened RS232 UART device\n");
	alt_up_rs232_enable_read_interrupt(rs232_dev);
	int x = 0;
	while (1) {

		/*
		if(x == 1){
			alt_up_rs232_disable_read_interrupt(rs232_dev);
			if (enter == 0) {
				data_W8 = 'A';
				enter = 1;
			} else if (enter == 1) {
				data_W8 = '\n';
				enter = 0;
			}
			write_FIFO_space = alt_up_rs232_get_available_space_in_write_FIFO(rs232_dev);
			if (write_FIFO_space >= WRITE_FIFO_EMPTY) {
				alt_up_rs232_write_data(rs232_dev, data_W8);
				alt_printf("write %c to RS232 UART\n", data_W8);
			}
			x = 0;
			alt_up_rs232_enable_read_interrupt(rs232_dev);
			OSTimeDlyHMSM(0, 0, 1, 0);

		}
		else if(x==0){
			read_FIFO_used = alt_up_rs232_get_used_space_in_read_FIFO(rs232_dev);
			//int result = alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
			//printf("%d\n", result);
			alt_printf("char stored in read_FIFO: %x\n", read_FIFO_used);
			if (read_FIFO_used > READ_FIFO_EMPTY) {
				alt_printf("char stored in read_FIFO: %x\n", read_FIFO_used);
				alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
				alt_printf("read %c from RS232 UART\n", data_R8);
			}
			x= 1;
		OSTimeDlyHMSM(0, 0, 1, 0);
		}*/

		read_FIFO_used = alt_up_rs232_get_used_space_in_read_FIFO(rs232_dev);
		//int result = alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
		//printf("%d\n", result);
		//alt_printf("char stored in read_FIFO: %x\n", read_FIFO_used);
		if (read_FIFO_used > READ_FIFO_EMPTY) {
			alt_printf("char stored in read_FIFO: %x\n", read_FIFO_used);
			alt_up_rs232_read_data(rs232_dev, &data_R8, &p_error);
			alt_printf("read %c from RS232 UART\n", data_R8);
		}

		OSTimeDlyHMSM(0, 0, 1, 0);

	}
}
alt_up_rs232_dev* serial_port;
/* Prints "Hello World" and sleeps for three seconds */
void task1(void* pdata)
{
  while (1)
  { 
	alt_u8 test;
	alt_u8 error = 0;
    printf("Hello from task1\n");

    int result = alt_up_rs232_read_data(serial_port, &test,&error);
    printf("%d is the result\n", result);
    printf("%c\n", test);
    OSTimeDlyHMSM(0, 0, 3, 0);
  }
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void* pdata)
{
  while (1)
  { 
    printf("Hello from task2\n");
    OSTimeDlyHMSM(0, 0, 3, 0);
  }
}
/* The main function creates two task and starts multi-tasking */
int main(void)
{

	serial_port = alt_up_rs232_open_dev(RS232_0_NAME);
	alt_up_rs232_enable_read_interrupt(serial_port);
  
	alt_u8 test;
	alt_u8 error = 0;
	while(1){
/*
		 int result = alt_up_rs232_read_data(serial_port, &test,&error);
		 printf("%d is the result\n", result);
		 printf("%c\n", test);
	}
  OSTaskCreateExt(task1,
                  NULL,
                  (void *)&task1_stk[TASK_STACKSIZE-1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
              
               
  OSTaskCreateExt(task2,
                  NULL,
                  (void *)&task2_stk[TASK_STACKSIZE-1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSStart();
  return 0;
  */
		OSInit();
		RS232Q = OSQCreate(RS232Q_stk, TASK_STACKSIZE);
		OSTaskCreateExt(taskRS232,
						NULL,
						(void *)&taskRS232_stk[TASK_STACKSIZE- 1],
						TASKRS232_PRIORITY,
						TASKRS232_PRIORITY,
						taskRS232_stk,
						TASK_STACKSIZE,
						NULL,
						0);
		OSStart();
		return 0;
	}
}

/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2004 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/
