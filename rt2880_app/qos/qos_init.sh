#!/bin/sh 

find_imq_dev=`iptables -t mangle -nvL | grep  todev`

VLANEnable=`nvram_get 2860 VLANEnable`
VLANID=`nvram_get 2860 VLANID`
if [ "$VLANEnable" == "1" ]; then
	inter_wan="eth2.2.$VLANID"
else
	inter_wan="eth2.2"
fi
#如果链存在，则不用创建
if [ -z "${find_imq_dev}"  ]; then
	iptables -t mangle -N qos_postrouting_imq_chain
	iptables -t mangle -N qos_postrouting_rule_chain
	iptables -t mangle -N qos_prerouting_imq_chain
	iptables -t mangle -N qos_prerouting_rule_chain
	iptables -t mangle -A qos_postrouting_imq_chain -o eth2.2 -j IMQ --todev 1
	iptables -t mangle -A qos_prerouting_imq_chain -i eth2.2 -j IMQ --todev 0

	iptables -t mangle -A PREROUTING -j qos_prerouting_rule_chain
	iptables -t mangle -A PREROUTING -j qos_prerouting_imq_chain

	iptables -t mangle -A POSTROUTING -j qos_postrouting_rule_chain
	iptables -t mangle -A POSTROUTING -j qos_postrouting_imq_chain
else
	echo "-----zero_debug------have chain,don't create!!!"
fi

