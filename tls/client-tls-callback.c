/* client-tls-callback.c
 *
 * Copyright (C) 2006-2020 wolfSSL Inc.
 *
 * This file is part of wolfSSL. (formerly known as CyaSSL)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* the usual suspects */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* socket includes */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

/* wolfSSL */
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#define DEFAULT_PORT 11111

#define CERT_FILE "../certs/ca-cert.pem"


int my_IORecv(WOLFSSL* ssl, char* buff, int sz, void* ctx)
{
    /* By default, ctx will be a pointer to the file descriptor to read from.
     * This can be changed by calling wolfSSL_SetIOReadCtx(). */
    int sockfd = *(int*)ctx;
    int recvd;


    /* Receive message from socket */
    if ((recvd = recv(sockfd, buff, sz, 0)) == -1) {
        /* error encountered. Be responsible and report it in wolfSSL terms */

        fprintf(stderr, "IO RECEIVE ERROR: ");
        switch (errno) {
        #if EAGAIN != EWOULDBLOCK
        case EAGAIN: /* EAGAIN == EWOULDBLOCK on some systems, but not others */
        #endif
        case EWOULDBLOCK:
            if (!wolfSSL_dtls(ssl) || wolfSSL_get_using_nonblock(ssl)) {
                fprintf(stderr, "would block\n");
                return WOLFSSL_CBIO_ERR_WANT_READ;
            }
            else {
                fprintf(stderr, "socket timeout\n");
                return WOLFSSL_CBIO_ERR_TIMEOUT;
            }
        case ECONNRESET:
            fprintf(stderr, "connection reset\n");
            return WOLFSSL_CBIO_ERR_CONN_RST;
        case EINTR:
            fprintf(stderr, "socket interrupted\n");
            return WOLFSSL_CBIO_ERR_ISR;
        case ECONNREFUSED:
            fprintf(stderr, "connection refused\n");
            return WOLFSSL_CBIO_ERR_WANT_READ;
        case ECONNABORTED:
            fprintf(stderr, "connection aborted\n");
            return WOLFSSL_CBIO_ERR_CONN_CLOSE;
        default:
            fprintf(stderr, "general error\n");
            return WOLFSSL_CBIO_ERR_GENERAL;
        }
    }
    else if (recvd == 0) {
        printf("Connection closed\n");
        return WOLFSSL_CBIO_ERR_CONN_CLOSE;
    }

    /* successful receive */
    printf("my_IORecv: received %d bytes from %d\n", sz, sockfd);
    return recvd;
}


int my_IOSend(WOLFSSL* ssl, char* buff, int sz, void* ctx)
{
    /* By default, ctx will be a pointer to the file descriptor to write to.
     * This can be changed by calling wolfSSL_SetIOWriteCtx(). */
    int sockfd = *(int*)ctx;
    int sent;


    /* Receive message from socket */
    if ((sent = send(sockfd, buff, sz, 0)) == -1) {
        /* error encountered. Be responsible and report it in wolfSSL terms */

        fprintf(stderr, "IO SEND ERROR: ");
        switch (errno) {
        #if EAGAIN != EWOULDBLOCK
        case EAGAIN: /* EAGAIN == EWOULDBLOCK on some systems, but not others */
        #endif
        case EWOULDBLOCK:
            fprintf(stderr, "would block\n");
            return WOLFSSL_CBIO_ERR_WANT_WRITE;
        case ECONNRESET:
            fprintf(stderr, "connection reset\n");
            return WOLFSSL_CBIO_ERR_CONN_RST;
        case EINTR:
            fprintf(stderr, "socket interrupted\n");
            return WOLFSSL_CBIO_ERR_ISR;
        case EPIPE:
            fprintf(stderr, "socket EPIPE\n");
            return WOLFSSL_CBIO_ERR_CONN_CLOSE;
        default:
            fprintf(stderr, "general error\n");
            return WOLFSSL_CBIO_ERR_GENERAL;
        }
    }
    else if (sent == 0) {
        printf("Connection closed\n");
        return 0;
    }

    /* successful send */
    printf("my_IOSend: sent %d bytes to %d\n", sz, sockfd);
    return sent;
}



int main(int argc, char** argv)
{
    int                ret; 
    int                sockfd = SOCKET_INVALID;
    struct sockaddr_in servAddr;
    char               buff[256];
    size_t             len;

    /* declare wolfSSL objects */
    WOLFSSL_CTX* ctx = NULL;
    WOLFSSL*     ssl = NULL;



    /* Check for proper calling convention */
    if (argc != 2) {
        printf("usage: %s <IPv4 address>\n", argv[0]);
        return 0;
    }



    /* Initialize wolfSSL */
    wolfSSL_Init();



    /* Create a socket that uses an internet IPv4 address,
     * Sets the socket to be stream based (TCP),
     * 0 means choose the default protocol. */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "ERROR: failed to create the socket\n");
        ret = -1; 
        goto exit;
    }



    /* Create and initialize WOLFSSL_CTX */
#ifdef USE_TLSV13
    ctx = wolfSSL_CTX_new(wolfTLSv1_3_client_method());
#else
    ctx = wolfSSL_CTX_new(wolfTLSv1_2_client_method());
#endif
    if (ctx == NULL) {
        fprintf(stderr, "ERROR: failed to create WOLFSSL_CTX\n");
        ret = -1;
        goto exit;
    }

    /* Load client certificates into WOLFSSL_CTX */
    if ((ret = wolfSSL_CTX_load_verify_locations(ctx, CERT_FILE, NULL))
        != WOLFSSL_SUCCESS) {
        fprintf(stderr, "ERROR: failed to load %s, please check the file.\n",
                CERT_FILE);
        goto exit;
    }

    /* Register callbacks */
    wolfSSL_SetIORecv(ctx, my_IORecv);
    wolfSSL_SetIOSend(ctx, my_IOSend);



    /* Initialize the server address struct with zeros */
    memset(&servAddr, 0, sizeof(servAddr));

    /* Fill in the server address */
    servAddr.sin_family = AF_INET;             /* using IPv4      */
    servAddr.sin_port   = htons(DEFAULT_PORT); /* on DEFAULT_PORT */

    /* Get the server IPv4 address from the command line call */
    if (inet_pton(AF_INET, argv[1], &servAddr.sin_addr) != 1) {
        fprintf(stderr, "ERROR: invalid address\n");
        ret = -1; 
        goto exit;
    }



    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr))
        == -1) {
        fprintf(stderr, "ERROR: failed to connect\n");
        ret = -1; 
        goto exit;
    }



    /* Create a WOLFSSL object */
    if ((ssl = wolfSSL_new(ctx)) == NULL) {
        fprintf(stderr, "ERROR: failed to create WOLFSSL object\n");
        ret = -1; 
        goto exit;
    }

    /* Attach wolfSSL to the socket */
    wolfSSL_set_fd(ssl, sockfd);

    /* Connect to wolfSSL on the server side */
    if ((ret = wolfSSL_connect(ssl)) != WOLFSSL_SUCCESS) {
        fprintf(stderr, "ERROR: failed to connect to wolfSSL\n");
        goto exit;
    }



    /* Get a message for the server from stdin */
    printf("Message for server: ");
    memset(buff, 0, sizeof(buff));
    if (fgets(buff, sizeof(buff), stdin) == NULL) {
        fprintf(stderr, "ERROR: failed to get message for server\n");
        ret = -1; 
        goto exit;
    }
    len = strnlen(buff, sizeof(buff));

    /* Send the message to the server */
    if ((ret = wolfSSL_write(ssl, buff, len)) != len) {
        fprintf(stderr, "ERROR: failed to write\n");
        goto exit;
    }



    /* Read the server data into our buff array */
    memset(buff, 0, sizeof(buff));
    if ((ret = wolfSSL_read(ssl, buff, sizeof(buff)-1)) == -1) {
        fprintf(stderr, "ERROR: failed to read\n");
        goto exit;
    }

    /* Print to stdout any data the server sends */
    printf("Server: %s\n", buff);

    ret = 0;

exit:
    /* Cleanup and return */
    if (ssl)
        wolfSSL_free(ssl);      /* Free the wolfSSL object              */
    if (sockfd != SOCKET_INVALID)
        close(sockfd);           /* Close the connection to the server   */
    if (ctx)
        wolfSSL_CTX_free(ctx);  /* Free the wolfSSL context object          */
    wolfSSL_Cleanup();          /* Cleanup the wolfSSL environment          */

    return ret;               /* Return reporting a success               */
}
