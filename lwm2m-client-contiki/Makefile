#/************************************************************************************************************************
# Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
#        following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#        following disclaimer in the documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
#        products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#************************************************************************************************************************/


LWM2M_DIR=../AwaLWM2M

CONTIKI_PROJECT=lwm2m-client-contiki
VERSION?="$(shell cat $(LWM2M_DIR)/VERSION)"
CONTIKI=../contiki

CFLAGS += -DPROJECT_CONF_H='"project-conf.h"' -DVERSION='\"$(VERSION)\"'

CFLAGS += -Wall -Wno-pointer-sign -DLWM2M_CLIENT
CFLAGS += -I$(LWM2M_DIR)/api/include

ifneq (,$(filter  $(TARGET),seedeye mikro-e)) 
  CFLAGS += -fno-short-double
  LDFLAGS += -Wl,--defsym,_min_heap_size=64000
endif

APPS += erbium

# fixme
APPS += client
APPS += common

APPS += jsmn
APPS += b64
APPS += hmac

APPDIRS += $(LWM2M_DIR)/lib
APPDIRS += $(LWM2M_DIR)/core/src

CONTIKI_WITH_IPV6 = 1
CONTIKI_WITH_RPL = 0

all: lwm2m-client-contiki

include $(CONTIKI)/Makefile.include

