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


function stop_lowpan()
{
	wpan_interface=$1
	lowpan_interface=$2

	ifconfig $lowpan_interface down
	ip link del $lowpan_interface
	ifconfig $wpan_interface down
}


function start_lowpan()
{
	phy_interface=$1
	wpan_interface=$2
	lowpan_interface=$3
	lowpan_addr=$4
	

	if [ -a /sys/class/net/$lowpan_interface ] ; then stop_lowpan $wpan_interface $lowpan_interface; fi;
	iwpan dev $wpan_interface set pan_id 0xabcd
	iwpan phy $phy_interface set channel 0 26
	ifconfig $wpan_interface up
	ip link add link $wpan_interface name $lowpan_interface type lowpan
	ifconfig $lowpan_interface up
	ifconfig $lowpan_interface add $lowpan_addr/48
}

function stop_tayga()
{
	if [ -a /tmp/tayga.pid ] ; then  kill -9 `cat /tmp/tayga.pid`; fi;
	sleep 1
	rm -rf /tmp/tayga.pid
	ip link del nat64
	iptables --flush
        iptables --table nat --flush
        iptables --delete-chain
 	iptables --table nat --delete-chain
}

function start_tayga()
{
	eth_interface=$1
	nat64_subnet=$2
	nat64_address=::1
	nat64_subnetaddress=:ffff::/96
	script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

    	if [ -a /sys/class/net/nat64 ] ; then stop_tayga; fi;
	sysctl -w net.ipv6.conf.all.forwarding=1
	sysctl -w net.ipv4.conf.all.forwarding=1
	tayga --mktun
	sleep 1
	ip link set nat64 up
	ip addr add 192.168.0.1 dev nat64
	ip addr add $nat64_subnet$nat64_address dev nat64
	ip route add 192.168.255.0/24 dev nat64
	ip route add $nat64_subnet$nat64_subnetaddress dev nat64
	tayga --config $script_dir/tayga.conf --pidfile /tmp/tayga.pid
	sleep 1
	iptables --flush
	iptables --table nat --flush
	iptables --delete-chain
	iptables --table nat --delete-chain
	iptables --table nat --append POSTROUTING --out-interface $eth_interface -j MASQUERADE
	iptables --append FORWARD --in-interface $eth_interface --out-interface nat64 -m state --state RELATED,ESTABLISHED -j ACCEPT
	iptables --append FORWARD --in-interface nat64 --out-interface $eth_interface -j ACCEPT
}

