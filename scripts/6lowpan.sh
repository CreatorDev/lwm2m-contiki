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
    echo "6LoWPAN helper script."
    echo     
    echo "Usage: sudo ./6lowpan.sh [OPTIONS]..."
    echo 
    echo "  -p <wpan_phy>         Specify wpan phy interface                     (default=phy0)"
    echo "  -w <wpan>             Specify wpan network interface                 (default=wpan0)"
    echo "  -l <lowpan>           Specify lowpan network interface               (default=lowpan0)"
    echo "  -e <interface>        Specify IPv4 interface for NAT                 (default=eth0)"
    echo "  -a <addr>             Address to be assigned to lowpan               (default=2001:1418:100::1)"
    echo "  -n <addr>             Subnet address to be assigned to nat64         (default=2001:1418:200::1)"
    echo "  -s                    Stop 6LoWPAN and tayga"
    echo
    echo "Example:"
    echo "  sudo ./6lowpan.sh -p phy0 -w wpan0 -l lowpan0 -e eth0 -a 2001:1418:100::1 -n 2001:1418:200::1"
}


if [ "$(id -u)" != "0" ]; then
    echo "Please run as root (sudo)."
    exit 1
fi

phy_interface=phy0
wpan_interface=wpan0
lowpan_interface=lowpan0
lowpan_addr=2001:1418:100::1
eth_interface=eth0
nat64_subnet=2001:1418:200::1
start=1

while getopts "h?p:w:l:e:a:n:s" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    p) phy_interface=$OPTARG
        ;;
    w) wpan_interface=$OPTARG
        ;;
    l) lowpan_interface=$OPTARG
        ;;
    e) eth_interface=$OPTARG
        ;;
    a) lowpan_addr=$OPTARG
        ;;
    n) nat64_subnet=$OPTARG
        ;;
    s) start=0
    esac
done

if [ $start == 1 ]; then
	echo "Setting up 6LoWPAN and tayga."
	start_lowpan $phy_interface $wpan_interface $lowpan_interface $lowpan_addr
	start_tayga $eth_interface $nat64_subnet
else
	echo "Stopping 6LoWPAN and tayga."
	stop_lowpan $wpan_interface $lowpan_interface
	stop_tayga
fi
