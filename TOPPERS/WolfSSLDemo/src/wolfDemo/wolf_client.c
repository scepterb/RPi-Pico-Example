/* wolf_client.c
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
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/ssl.h>
#include <stdio.h>
#include <string.h>
#include "r_t4_itcpip.h"
#include "wolfssl/certs_test.h"
#include "wolf_demo.h"

ER    t4_tcp_callback(ID cepid, FN fncd , VP p_parblk);

static int my_IORecv(WOLFSSL* ssl, char* buff, int sz, void* ctx)
{
    int ret;
    ID  cepid = 0; 

    if (ctx != NULL)
        cepid = *(ID *)ctx;
    else 
        return WOLFSSL_CBIO_ERR_GENERAL;

    ret = tcp_rcv_dat(cepid, buff, sz, TMO_FEVR);
    if (ret > 0)
        return ret;
    else         
        return WOLFSSL_CBIO_ERR_GENERAL;
}

static int my_IOSend(WOLFSSL* ssl, char* buff, int sz, void* ctx)
{
    int ret;
    ID  cepid = 0 ;

    if (ctx != NULL)
        cepid = *(ID *)ctx;
    else 
        return WOLFSSL_CBIO_ERR_GENERAL;

    ret = tcp_snd_dat(cepid, buff, sz, TMO_FEVR);
    if (ret == sz)
        return ret;
    else         
        return WOLFSSL_CBIO_ERR_GENERAL;
}

static int getIPaddr(char *arg)
{
    int a1, a2, a3, a4;
    if (sscanf(arg, "%d.%d.%d.%d", &a1, &a2, &a3, &a4) == 4)
        return (a1 << 24) | (a2 << 16) | (a3 << 8) | a4;
    else 
        return 0;
}


WOLFSSL_CTX *wolfSSL_TLS_client_init(void)
{

    WOLFSSL_CTX* ctx;
#ifndef NO_FILESYSTEM
    #ifdef USE_ECC_CERT
        char *cert       = "./certs/ca-ecc-cert.pem";
    #else
        char *cert       = "./certs/ca-cert.pem";
    #endif
#else
    #ifdef USE_ECC_CERT
        const unsigned char *cert       = ca_ecc_der_256;
        #define  SIZEOF_CERT sizeof_ca_ecc_der_256
    #else
        const unsigned char *cert       = ca_cert_der_2048;
        #define  SIZEOF_CERT sizeof_ca_cert_der_2048
    #endif
#endif

#ifdef DEBUG_WOLFSSL
    wolfSSL_Debugging_ON();
#endif

    /* Create and initialize WOLFSSL_CTX */
#ifdef WOLFSSL_TLS13
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method_ex((void *)NULL))) == NULL) 
#else
    if ((ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method_ex((void *)NULL))) == NULL) 
#endif
    {
        printf("ERROR: failed to create WOLFSSL_CTX\n");
        return NULL;
    }

#if !defined(NO_FILESYSTEM)
    if (wolfSSL_CTX_load_verify_locations(ctx, cert, 0) != WOLFSSL_SUCCESS) {
        printf("ERROR: can't load \"%s\"\n", cert);
        return NULL;
    }
#else
    if (wolfSSL_CTX_load_verify_buffer(ctx, cert, SIZEOF_CERT, WOLFSSL_FILETYPE_ASN1) != WOLFSSL_SUCCESS){
           printf("ERROR: can't load certificate data\n");
       return NULL;
    }
#endif


    /* Register callbacks */
    wolfSSL_SetIORecv(ctx, my_IORecv);
    wolfSSL_SetIOSend(ctx, my_IOSend);
    return (void *)ctx;

}

int wolfSSL_TLS_client(void *v_ctx, func_args *args)
{
    ID cepid = 3; /* Not used ID 3 Used */
    ER ercd;
    int ret = 0;
    WOLFSSL_CTX *ctx = (WOLFSSL_CTX *)v_ctx;
    WOLFSSL *ssl;
    #define BUFF_SIZE 256
    static const char sendBuff[]= "Hello Server\n" ;
    char    rcvBuff[BUFF_SIZE] = {0};
    static T_IPV4EP my_addr = { 0, 0 };
    T_IPV4EP dst_addr;

    dst_addr.ipaddr = getIPaddr(SERVER_IP);
    dst_addr.portno = SERVER_PortNo;
    if ((ercd = tcp_con_cep(cepid, &my_addr, &dst_addr, TMO_FEVR)) != E_OK) {
        printf("ERROR TCP Connect: %d\n", ercd);
        ret = -1;
        goto exit_;
    }

    if ((ssl = wolfSSL_new(ctx)) == NULL) {
        printf("ERROR wolfSSL_new: %d\n", wolfSSL_get_error(ssl, 0));
        ret = -1;
        goto exit_;
    }
    /* set client certificate */
#if defined(USE_ECC_CERT)
    if (ret == 0) {
        ret = wolfSSL_use_certificate_buffer(ssl,
                                        cliecc_cert_der_256,
                                        sizeof_cliecc_cert_der_256,
                                        WOLFSSL_FILETYPE_ASN1);
        if(err != SSL_SUCCESS) {
            printf("ERROR: can't load client-certificate\n");
            ret = -1;
            goto exit_;
        }
    }
#else
    ret = wolfSSL_use_certificate_buffer(ssl,
                                     client_cert_der_2048,
                                     sizeof_client_cert_der_2048,
                                     WOLFSSL_FILETYPE_ASN1);
    if (ret != SSL_SUCCESS) {
        printf("ERROR: can't load client-certificate\n");
        ret = -1;
        goto exit_;
    }
#endif /* USE_ECC_CERT */
#if defined(USE_ECC_CERT)
    ret = wolfSSL_use_PrivateKey_buffer(ssl,
                                ecc_clikey_der_256,
                                sizeof_ecc_clikey_der_256,
                                WOLFSSL_FILETYPE_ASN1);
    if (ret) {
        printf("ERROR wolfSSL_use_PrivateKey_buffer: %d\n",
                                              wolfSSL_get_error(ssl, 0));
        ret = -1;
        goto exit_;
    }

#else
   ret = wolfSSL_use_PrivateKey_buffer(ssl, client_key_der_2048,
                         sizeof_client_key_der_2048, WOLFSSL_FILETYPE_ASN1);
   if (ret != SSL_SUCCESS) {
       printf("ERROR wolfSSL_use_PrivateKey_buffer: %d\n",
                                            wolfSSL_get_error(ssl, 0));
       ret = -1;
       goto exit_;
   }
#endif /* USE_ECC_CERT */

    /* set callback context */
    wolfSSL_SetIOReadCtx(ssl, (void *)&cepid);
    wolfSSL_SetIOWriteCtx(ssl, (void *)&cepid);

    if ((ret = wolfSSL_connect(ssl)) != WOLFSSL_SUCCESS) {
        printf("ERROR SSL connect: %d\n",  wolfSSL_get_error(ssl, 0));
        goto exit_;
    }

    if ((ret = wolfSSL_write(ssl, sendBuff, strlen(sendBuff)))
    		!= strlen(sendBuff)) {
        printf("ERROR SSL write: %d\n", wolfSSL_get_error(ssl, 0));
        goto exit_;
    }

    if ((ret = wolfSSL_read(ssl, rcvBuff, BUFF_SIZE)) < 0) {
        printf("ERROR SSL read: %d\n", wolfSSL_get_error(ssl, 0));
        goto exit_;
    }
    ret = (ret >= BUFF_SIZE) ? BUFF_SIZE - 1 : ret ;
    rcvBuff[ret] = '\0' ;
    printf("Received: %s\n", rcvBuff);
    ret = 0;
exit_:
    /* frees all data before client termination */
    if (ssl) {
       wolfSSL_shutdown(ssl);
       wolfSSL_free(ssl);
       ssl = NULL;
    }
    tcp_sht_cep(cepid);
    tcp_cls_cep(cepid, TMO_FEVR);

    return ret;
}
