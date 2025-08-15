 /* wolf_main.c
 *
 * Copyright (C) 2006-2023 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */
#ifndef WOLFSSLDEMO_H_
#define WOLFSSLDEMO_H_

#include "r_sys_time_rx_if.h"
#include "r_cmt_rx_if.h"
#include "Pin.h"
#include "r_t4_itcpip.h"

//#define DEBUG_PRINT

#if defined(DEBUG_PRINT)
    #define debug_print(...) printf(__VA_ARGS__)
#else
    #define debug_print(...)
#endif

void main(void);
bool init_ether(void);

/******************************************************************************
Typedef definitions
******************************************************************************/

extern ER		dly_tsk(UW) ;
extern void dhcp_check(void);
extern void print_dhcp(VP param);
extern void startw(void);
extern void waisem_ether_wrapper(void);
extern void sigsem_ether_wrapper(void);
extern void timer_interrupt(void *pdata);
extern void cmt_isr_common2 (uint32_t channel);
extern ER wolfSSL_TLS_client_Wrapper(void) ;
extern ER wolfSSL_TLS_server_Wrapper(void) ;
extern void timeTick(void *pdata);
extern void cmt1_isr (void);

#define LED_CTL(x)  \
		PORT4.PODR.BYTE = x;	\
		PORT4.PDR.BYTE = 0x01;

#endif /* WOLFSSLDEMO_H_ */
