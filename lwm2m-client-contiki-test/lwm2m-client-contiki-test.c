/**
 * @file
 * LightWeightM2M LWM2M test client
 *
 * @author Imagination Technologies
 *
 * @copyright Copyright (c) 2015, Imagination Technologies Limited
 *
 * All rights reserved.
 *
 * Redistribution and use of the Software in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. The Software (including after any modifications that you make to it) must support the
 *    FlowCloud Web Service API provided by Licensor and accessible at http://ws-uat.flowworld.com
 *    and/or some other location(s) that we specify.
 *
 * 2. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 3. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 4. Neither the name of the copyright holder nor the names of its contributors may be used to
 *    endorse or promote products derived from this Software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/************************************************************************************************************
 * Includes
 ************************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"
#include "shell.h"
#include "serial-shell.h"
#include "process.h"

#ifdef CONTIKI_TARGET_MIKRO_E
#include <p32xxxx.h>
#endif

#include "b64.h"

#include <awa/static.h>
#include "lwm2m-client-flow-object.h"
#include "lwm2m-client-flow-access-object.h"
#include "lwm2m-client-device-object.h"

#include "cmds/init-cmdline.h"
#include "cmds/factory_bootstrap-cmdline.h"
#include "cmds/define_object-cmdline.h"
#include "cmds/define_resource-cmdline.h"
#include "cmds/create_object-cmdline.h"
#include "cmds/create_resource-cmdline.h"
#include "cmds/get_resource_value-cmdline.h"
#include "cmds/set_resource_value-cmdline.h"


const char * version = VERSION; /* from Makefile */

static volatile bool running;
static AwaStaticClient * awaClient;

PROCESS(lwm2m_client, "LwM2M Client");
PROCESS(lwm2m_shell_process, "Lwm2m Shell");
AUTOSTART_PROCESSES(&lwm2m_shell_process);

PROCESS_THREAD(lwm2m_client, ev, data)
{
    PROCESS_BEGIN();
    PROCESS_PAUSE();

    printf("\n");

    printf("[INFO] Starting AwaLWM2M Client\n");

    //Add default route to tap0 interface.
    uip_ipaddr_t ipaddr;
    uip_ip6addr(&ipaddr, 0x2001, 0x1418, 0x0100, 0, 0, 0, 0, 0x1);
    uip_ds6_defrt_add(&ipaddr, 0);

    printf("LWM2M client - version %s\n", version);

    /* Define application-specific events here. */
    while (running)
    {
        static struct etimer et;
        static int waitTime;

        waitTime = AwaStaticClient_Process(awaClient);

        etimer_set(&et, (waitTime * CLOCK_SECOND) / 1000);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        waitTime = 0;
    }

    AwaStaticClient_Free(&awaClient);

    printf("AwaLWM2M Client stopped\n");

    PROCESS_END();
}

PROCESS(shell_echo_process, "echo");
SHELL_COMMAND(echo_command,
          "echo",
          "echo - Echo serial input",
          &shell_echo_process);
PROCESS_THREAD(shell_echo_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

    printf("%s\n", data);

    PROCESS_END();
}

PROCESS(shell_reset_process, "reset");
SHELL_COMMAND(reset_command,
          "reset",
          "reset - Execute a software reset",
          &shell_reset_process);
PROCESS_THREAD(shell_reset_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

#ifdef CONTIKI_TARGET_MIKRO_E
    /* The following code illustrates a software Reset */
    // assume interrupts are disabled
    // assume the DMA controller is suspended
    // assume the device is locked
    /* perform a system unlock sequence */
    // starting critical sequence
    SYSKEY = 0x00000000; //write invalid key to force lock
    SYSKEY = 0xAA996655;  //write key1 to SYSKEY
    SYSKEY = 0x556699AA;  //write key2 to SYSKEY
    // OSCCON is now unlocked
    /* set SWRST bit to arm reset */
    RSWRSTSET = 1;
    /* read RSWRST register to trigger reset */
    unsigned int dummy;
    dummy = RSWRST;
    /* prevent any unwanted code execution until reset occurs*/
    while(1);
#else
    printf("UNSUPPORTED\n");
#endif

    PROCESS_END();
}

PROCESS(shell_init_process, "init");
SHELL_COMMAND(init_command,
	      "init",
	      "init - Initialize AwaLWM2M Static",
	      &shell_init_process);
PROCESS_THREAD(shell_init_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

    struct init_args cmdline_info;

    if (init_parser_string(data, &cmdline_info, "client") == 0)
    {
        if (cmdline_info.help_given != 0)
        {
            init_parser_print_help();
        }
        else
        {
            printf("[INFO] Bootstrap URI: %s\n", cmdline_info.bootstrap_arg);
            printf("[INFO] Client ID: %s\n", cmdline_info.clientID_arg);
            printf("[INFO] CoAP Port: %d\n", cmdline_info.port_arg);
            printf("[INFO] Verbose: %d\n", cmdline_info.verbose_flag);

            awaClient  = AwaStaticClient_New();

#ifdef CONTIKI_TARGET_MINIMAL_NET
            FILE * logFile = NULL;
            logFile = fopen("contiki.log", "at");
            Lwm2m_SetOutput(logFile);
#endif

            if(cmdline_info.verbose_flag == 1)
                AwaStaticClient_SetLogLevel(AwaLogLevel_Debug);
            else
                AwaStaticClient_SetLogLevel(AwaLogLevel_None);

            AwaStaticClient_SetEndPointName(awaClient, cmdline_info.clientID_arg);
            AwaStaticClient_SetCoAPListenAddressPort(awaClient, "::", cmdline_info.port_arg);
            AwaStaticClient_SetBootstrapServerURI(awaClient, cmdline_info.bootstrap_arg);

            AwaStaticClient_Init(awaClient);

            if (DefineDeviceObject(awaClient) != 0)
                printf("[ERROR] Device Object definition failed.\n");
        }
    }
    else
    {
        init_parser_print_help();
    }

    PROCESS_END();
}

PROCESS(shell_factory_bootstrap_process, "factory-boostrap");
SHELL_COMMAND(factory_bootstrap_command,
          "factory_bootstrap",
          "factory_bootstrap - set AwaLWM2M factory boostrap information",
          &shell_factory_bootstrap_process);
PROCESS_THREAD(shell_factory_bootstrap_process, ev, data)
{
    PROCESS_BEGIN();


    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

    struct factory_bootstrap_args cmdline_info;

    if (factory_bootstrap_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
    {

        if (cmdline_info.help_given != 0)
        {
            factory_bootstrap_parser_print_help();
        }
        else
        {
            if (awaClient != NULL)
            {
                if (running == false)
                {
                    static AwaFactoryBootstrapInfo bootstrapinfo = { 0 };

                    printf("[INFO] ServerURI: %s\n", cmdline_info.serverURI_arg);
                    printf("[INFO] Lifetime: %d\n", cmdline_info.lifetime_arg);


                    strcpy(bootstrapinfo.SecurityInfo.ServerURI, cmdline_info.serverURI_arg);
                    bootstrapinfo.ServerInfo.Lifetime = cmdline_info.lifetime_arg;
                    bootstrapinfo.ServerInfo.DefaultMinPeriod = 1;
                    bootstrapinfo.ServerInfo.DefaultMaxPeriod = -1;
                    bootstrapinfo.ServerInfo.DisableTimeout = 86400;
                    bootstrapinfo.ServerInfo.Notification = false;
                    sprintf(bootstrapinfo.ServerInfo.Binding, "U");
                    bootstrapinfo.SecurityInfo.SecurityMode = AwaSecurityMode_NoSec;
                    sprintf(bootstrapinfo.SecurityInfo.PublicKeyOrIdentity, "[PublicKey]");
                    sprintf(bootstrapinfo.SecurityInfo.SecretKey, "[SecretKey]");

                    if (AwaStaticClient_SetFactoryBootstrapInformation(awaClient, &bootstrapinfo) != AwaError_Success)
                    {
                        printf("[ERROR] Failed to set AwaLWM2M factory bootstrap information\n");
                    }
                }
                else
                {
                    printf("[ERROR] AwaLWM2M already running\n");
                }
            }
            else
            {
                printf("[ERROR] AwaLWM2M not initialized - please use init\n");
            }
        }
    }
    else
    {
        factory_bootstrap_parser_print_help();
    }

    PROCESS_END();
}

PROCESS(shell_define_object_process, "define_object");
SHELL_COMMAND(define_object_command,
          "define_object",
          "define_object - defines a AwaLWM2M Static Object",
          &shell_define_object_process);
PROCESS_THREAD(shell_define_object_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

     struct define_object_args cmdline_info;

     if (define_object_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
     {

         if (cmdline_info.help_given != 0)
         {
             define_object_parser_print_help();
         }
         else
         {
             if (awaClient != NULL)
             {
                 if (running == false)
                 {
                     AwaError result = AwaStaticClient_DefineObject(awaClient, cmdline_info.objectID_arg, "", cmdline_info.min_arg, cmdline_info.max_arg);
                     printf("[RESULT] %d\n", result);
                 }
                 else
                 {
                     printf("[ERROR] AwaLWM2M already running\n");
                 }
             }
             else
             {
                 printf("[ERROR] AwaLWM2M not initialized - please use init\n");
             }
         }
     }
     else
     {
         define_object_parser_print_help();
     }

     PROCESS_END();
}

PROCESS(shell_define_resource_process, "define_resource");
SHELL_COMMAND(define_resource_command,
          "define_resource",
          "define_resource - defines a AwaLWM2M Static Object",
          &shell_define_resource_process);
PROCESS_THREAD(shell_define_resource_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
     }

     struct define_resource_args cmdline_info;

     if (define_resource_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
     {

         if(cmdline_info.help_given != 0)
         {
             define_resource_parser_print_help();
         }
         else
         {
             if (awaClient != NULL)
             {
                 if (running == false)
                 {
                     AwaError result = AwaError_Unspecified;

                     size_t resourceInstancesSize = cmdline_info.resourceSize_arg * cmdline_info.max_arg;
                     void * resourceInstances = malloc(resourceInstancesSize);
                     memset(resourceInstances, 0, resourceInstancesSize);

                     if (resourceInstances != NULL)
                     {
                         result = AwaStaticClient_DefineResource(awaClient, cmdline_info.objectID_arg, cmdline_info.resourceID_arg, cmdline_info.resourceName_arg, cmdline_info.resourceType_arg, cmdline_info.min_arg, cmdline_info.max_arg, cmdline_info.operations_arg);

                         if (result == AwaError_Success)
                         {
                             result = AwaStaticClient_SetResourceStorageWithPointer(awaClient, cmdline_info.objectID_arg, cmdline_info.resourceID_arg, resourceInstances, cmdline_info.resourceSize_arg, 0);
                         }
                     }
                     else
                     {
                         result = AwaError_OutOfMemory;
                     }

                     printf("[RESULT] %d\n", result);
                 }
                 else
                 {
                     printf("[ERROR] AwaLWM2M already running\n");
                 }
             }
             else
             {
                 printf("[ERROR] AwaLWM2M not initialized - please use init\n");
             }
         }
     }
     else
     {
         define_resource_parser_print_help();
     }

     PROCESS_END();
}

PROCESS(shell_get_resource_value_process, "get_resource_value");
SHELL_COMMAND(get_resource_value_command,
          "get_resource_value",
          "get_resource_value - defines a AwaLWM2M Static Object",
          &shell_get_resource_value_process);
PROCESS_THREAD(shell_get_resource_value_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

    struct get_resource_value_args cmdline_info;

    if (get_resource_value_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
    {

        if(cmdline_info.help_given != 0)
        {
            get_resource_value_parser_print_help();
        }
        else
        {
            if (awaClient != NULL)
            {
                size_t resourceInstanceSize = 0;
                const uint8_t * resourceInstancesValue = AwaStaticClient_GetResourceInstancePointer(awaClient, cmdline_info.objectID_arg, cmdline_info.objectInstanceID_arg, cmdline_info.resourceID_arg, cmdline_info.resourceInstanceID_arg, &resourceInstanceSize);

                if (resourceInstancesValue != NULL)
                {
                    int b64ResourceInstanceValueSize = (((resourceInstanceSize + 2) * 4) / 3) + 2;
                    char * b64ResourceInstanceValue = (char *) malloc(b64ResourceInstanceValueSize);

                    b64ResourceInstanceValue[b64ResourceInstanceValueSize-1] = '\0';
                    b64Encode(b64ResourceInstanceValue, b64ResourceInstanceValueSize, resourceInstancesValue, resourceInstanceSize);
                    printf("[RESULT] %s\n", b64ResourceInstanceValue);

                    free(b64ResourceInstanceValue);
                }
                else
                {
                    printf("[ERROR] Failed to read resource instance value\n");
                }
            }
            else
            {
                printf("[ERROR] AwaLWM2M not initialized - please use init\n");
            }
        }
    }
    else
    {
        get_resource_value_parser_print_help();
    }

    PROCESS_END();
}


PROCESS(shell_set_resource_value_process, "set_resource_value");
SHELL_COMMAND(set_resource_value_command,
          "set_resource_value",
          "set_resource_value - defines a AwaLWM2M Static Object",
          &shell_set_resource_value_process);
PROCESS_THREAD(shell_set_resource_value_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
        PROCESS_EXIT();
    }

    struct set_resource_value_args cmdline_info;

    if (set_resource_value_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
    {

        if(cmdline_info.help_given != 0)
        {
         set_resource_value_parser_print_help();
        }
        else
        {
            if (awaClient != NULL)
            {
                AwaError result = AwaError_Unspecified;
                size_t resourceInstanceSize = 0;
                uint8_t * resourceInstancesValue = AwaStaticClient_GetResourceInstancePointer(awaClient, cmdline_info.objectID_arg, cmdline_info.objectInstanceID_arg, cmdline_info.resourceID_arg, cmdline_info.resourceInstanceID_arg, &resourceInstanceSize);

                if (resourceInstancesValue != NULL)
                {
                    int B64EncodedResourceInstanceValueSize = strlen(cmdline_info.value_arg);
                    int B64DecodedResourceInstanceValueSize = 0;
                    char * b64DecodedResourceInstanceValue = (char *) malloc(B64EncodedResourceInstanceValueSize);
                    memset(b64DecodedResourceInstanceValue, 0, B64DecodedResourceInstanceValueSize);

                    if((B64DecodedResourceInstanceValueSize = b64Decode(b64DecodedResourceInstanceValue,B64EncodedResourceInstanceValueSize, cmdline_info.value_arg, B64EncodedResourceInstanceValueSize)) > 0)
                    {
                        memcpy(resourceInstancesValue, b64DecodedResourceInstanceValue, B64DecodedResourceInstanceValueSize);
                        result = AwaError_Success;
                    }
                    else
                    {
                        result = AwaError_Unspecified;
                    }

                    printf("[RESULT] %d\n", result);

                    free(b64DecodedResourceInstanceValue);
                }
                else
                {
                    printf("[ERROR] Failed to read resource instance value\n");
                }
            }
            else
            {
                printf("[ERROR] AwaLWM2M not initialized - please use init\n");
            }
        }
    }
    else
    {
     set_resource_value_parser_print_help();
    }

    PROCESS_END();
}


PROCESS(shell_start_process, "start");
SHELL_COMMAND(start_command,
          "start",
          "start - starts AwaLWM2M Static",
          &shell_start_process);
PROCESS_THREAD(shell_start_process, ev, data)
{
    PROCESS_BEGIN();


    printf("\n");

    if(data == NULL)
    {
       PROCESS_EXIT();
    }

    if (awaClient != NULL)
    {
        if (running == false)
        {
            running = true;
            process_start(&lwm2m_client, NULL);
        }
        else
        {
            printf("[ERROR] AwaLWM2M already running\n");
        }
    }
    else
    {
        printf("[ERROR] AwaLWM2M not initialized - please use init\n");
    }

    PROCESS_END();
}

PROCESS(shell_create_object_process, "create_object");
SHELL_COMMAND(create_object_command,
          "create_object",
          "create_object - defines a AwaLWM2M Static Object",
          &shell_create_object_process);
PROCESS_THREAD(shell_create_object_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
       PROCESS_EXIT();
    }

    struct create_object_args cmdline_info;

    if (create_object_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
    {

        if (cmdline_info.help_given != 0)
        {
            create_object_parser_print_help();
        }
        else
        {
            if (awaClient != NULL)
            {
                AwaError result = AwaStaticClient_CreateObjectInstance(awaClient, cmdline_info.objectID_arg, cmdline_info.instanceID_arg);
                printf("[RESULT] %d\n", result);
            }
            else
            {
                printf("[ERROR] AwaLWM2M not initialized - please use init\n");
            }
        }
    }
    else
    {
        create_object_parser_print_help();
    }

    PROCESS_END();
}

PROCESS(shell_create_resource_process, "create_object");
SHELL_COMMAND(create_resource_command,
          "create_resource",
          "create_resource - defines a AwaLWM2M Static Object",
          &shell_create_resource_process);
PROCESS_THREAD(shell_create_resource_process, ev, data)
{
    PROCESS_BEGIN();

    printf("\n");

    if(data == NULL)
    {
       PROCESS_EXIT();
    }

    struct create_resource_args cmdline_info;

    if (create_resource_parser_string(data, &cmdline_info, "factory_bootstrap") == 0)
    {

        if (cmdline_info.help_given != 0)
        {
            create_resource_parser_print_help();
        }
        else
        {
            if (awaClient != NULL)
            {
                AwaError result = AwaStaticClient_CreateResource(awaClient, cmdline_info.objectID_arg, cmdline_info.instanceID_arg, cmdline_info.resourceID_arg);
                printf("[RESULT] %d\n", result);
            }
            else
            {
                printf("[ERROR] AwaLWM2M not initialized - please use init\n");
            }
        }
    }
    else
    {
        create_resource_parser_print_help();
    }

    PROCESS_END();
}

PROCESS(shell_stop_process, "stop");
SHELL_COMMAND(stop_command,
	      "stop",
	      "stop - stops AwaLWM2M Static",
	      &shell_stop_process);
PROCESS_THREAD(shell_stop_process, ev, data)
{
    PROCESS_BEGIN();

    running = false;

    PROCESS_END();
}


PROCESS_THREAD(lwm2m_shell_process, ev, data)
{
    PROCESS_BEGIN();

    serial_shell_init();

    shell_file_init();

    shell_register_command(&init_command);
    shell_register_command(&echo_command);
    shell_register_command(&reset_command);
    shell_register_command(&factory_bootstrap_command);
    shell_register_command(&define_object_command);
    shell_register_command(&define_resource_command);
    shell_register_command(&start_command);
    shell_register_command(&create_object_command);
    shell_register_command(&create_resource_command);
    shell_register_command(&get_resource_value_command);
    shell_register_command(&set_resource_value_command);
    shell_register_command(&stop_command);

    PROCESS_END();
}












