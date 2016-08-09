/************************************************************************************************************************
 Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
 following conditions are met:
     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
        following disclaimer.
     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
        following disclaimer in the documentation and/or other materials provided with the distribution.
     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
        products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
************************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include <awa/static.h>

#define ROUTING_PROXY_TEST
#define DEFAULT_COAP_PORT (6000)
#define DEFAULT_IPC_PORT (12345)

const char * version = VERSION; /* from Makefile */

typedef struct
{
    int CoapPort;
    bool Verbose;
    char * EndPointName;
    char * BootStrap;
    char * FactoryBootstrapInformation;

} Options;

static const Options options =
{
    .CoapPort = DEFAULT_COAP_PORT,
    .Verbose = true,
#ifdef ROUTING_PROXY_TEST
    .BootStrap = "coap://[2001:1418:100::1]:15683/",
#else
    .BootStrap = "coap://[fe80::1]:15685",
#endif
    .EndPointName = "imagination1",
    .FactoryBootstrapInformation = NULL,
};

PROCESS(lwm2m_client, "Awa LWM2M Client");
AUTOSTART_PROCESSES(&lwm2m_client);

PROCESS_THREAD(lwm2m_client, ev, data)
{
    PROCESS_BEGIN();

    printf("Starting Awa LWM2M Example Client\n");

#ifdef RF_CHANNEL
    printf("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
    printf("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

    printf("uIP buffer: %u\n", UIP_BUFSIZE);
    printf("LL header: %u\n", UIP_LLH_LEN);
    printf("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
    //printf("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);

#ifdef ROUTING_PROXY_TEST
    uip_ipaddr_t ipaddr;
    uip_ip6addr(&ipaddr, 0x2001, 0x1418, 0x0100, 0, 0, 0, 0, 0x1);
    uip_ds6_defrt_add(&ipaddr, 0);
#endif

    //Lwm2m_SetOutput(stdout);
    //Lwm2m_SetLogLevel((options->Verbose) ? DebugLevel_Debug : DebugLevel_Info);
    //Lwm2m_PrintBanner();

    printf("LWM2M client - version %s\n", version);
    printf("LWM2M client - CoAP port %d\n", options.CoapPort);

    static AwaStaticClient * awaClient;

    awaClient  = AwaStaticClient_New();

    AwaStaticClient_SetEndPointName(awaClient, options.EndPointName);
    AwaStaticClient_SetCoAPListenAddressPort(awaClient, "::", options.CoapPort);
    AwaStaticClient_SetBootstrapServerURI(awaClient, options.BootStrap);

    AwaStaticClient_Init(awaClient);

    /* Define application-specific events here. */
    while (1)
    {
        static struct etimer et;
        static int waitTime;

        waitTime = AwaStaticClient_Process(awaClient);

        etimer_set(&et, (waitTime * CLOCK_SECOND) / 1000);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        waitTime = 0;
    }

    AwaStaticClient_Free(&awaClient);

    PROCESS_END();
}
