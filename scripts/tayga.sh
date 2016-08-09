#!/bin/bash

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

source network_functions.sh

function show_help()
{
    echo "Tayga helper script for simulated contiki environment."
    echo     
    echo "Usage: sudo ./tayga.sh [OPTIONS]..."
    echo 
    echo "  -c <tap>              Specify contiki network interface          (default=tap0)"
    echo "  -e <interface>        Specify IPv4 interface for NAT             (default=eth0)"
    echo "  -a <addr>             Address to be assigned to contiki network"
    echo "                        interface (note interface must be"
    echo "                        specified first)                           "
    echo "  -n <addr>             Subnet address to be assigned to nat64     (default=2001:1418:200)"
    echo "  -s			  Stop tayga and reset NAT"
    echo
    echo "Example:"
    echo "  sudo ./tayga.sh -c tap -a 2001:1418:100::1 -a aaaa::1 -e eth0 -n 2001:1418:200"
}


if [ "$(id -u)" != "0" ]; then
    echo "Please run as root (sudo)."
    exit 1
fi

contiki_interface=tap0
nat64_subnet=2001:1418:200
eth_interface=eth0
start=1

OPTIND=1

while getopts "h?c:e:a:n:s" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    c) contiki_interface=$OPTARG
        ;;
    e) eth_interface=$OPTARG
        ;;
    a) contiki_address=$OPTARG
       ip addr add $contiki_address/64 dev $contiki_interface
        ;;
    n) nat64_subnet=$OPTARG
        ;;
    s) start=0
        ;;
    esac
done

if [ $start == 1 ]; then
	echo "Setting up NAT64 to use IPv4 interface $eth_interface"
	start_tayga $eth_interface $nat64_subnet
else
	echo "Stopping NAT64 "
	stop_tayga
fi


