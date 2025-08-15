/* wolf_console.c
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

#include "wolfssl/wolfcrypt/settings.h"
#include "wolfssl/ssl.h"
#include <stdio.h>
#include <stdint.h>
#include <r_t4_itcpip.h>
#include "wolf_demo.h"

extern int wolfSSL_TLS_server(void *v_ctx, func_args *args);
extern int wolfSSL_TLS_client(void *v_ctx, func_args *args);

static WOLFSSL_CTX *wolfSSL_sv_ctx;
static WOLFSSL_CTX *wolfSSL_cl_ctx;

static long tick;
 void timeTick(void *pdata)
{
    tick++;
}

#define FREQ 10000 /* Hz */

double current_time(int reset)
{
    if (reset) 
        tick = 0 ;
    return ((double)tick/FREQ) ;	
}

int wolfSSL_TLS_client_Wrapper(void) {
    func_args args = {0};
    int ret = 0;
    wolfSSL_cl_ctx = wolfSSL_TLS_client_init();
    if(wolfSSL_cl_ctx == NULL) {
    	printf("wolfSSL client initialization failure\n");
        return 	E_SYS;
    }
	printf("Start TLS Client\n");

	ret = wolfSSL_TLS_client(wolfSSL_cl_ctx, &args);
    wolfSSL_CTX_free(wolfSSL_cl_ctx);
    wolfSSL_Cleanup();

	return ret;
}
int wolfSSL_TLS_server_Wrapper(void) {
    func_args args = {0};
    int ret = 0;
    wolfSSL_sv_ctx = wolfSSL_TLS_server_init();
    if(wolfSSL_sv_ctx == NULL) {
    	printf("wolfSSL server initialization failure\n");
        return 	E_SYS;
    }
	printf("Start TLS Server\n");

	ret = wolfSSL_TLS_server(wolfSSL_sv_ctx, &args);
    wolfSSL_CTX_free(wolfSSL_sv_ctx);
    wolfSSL_Cleanup();

	return ret;
}
