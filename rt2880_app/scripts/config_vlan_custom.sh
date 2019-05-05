#!/bin/sh

###实现WAN的VLAN

. /sbin/config.sh


VLANEnable=`nvram_get 2860 VLANEnable`
VLANID=`nvram_get 2860 VLANID`
VLANPriority=`nvram_get 2860 VLANPriority`

removeVlan()
{
	##删除以前的vlan
	cat /proc/net/vlan/config | grep  eth2.2 | while read LINE
		#while read LINE
	do
		vlan_inter=`echo $LINE | cut -d "|" -f 1`
		#echo "dddd=[$vlan_inter]"
		if [ "$vlan_inter" = "eth2.2 " ] ; then
			continue
		fi	
		echo "cc=$vlan_inter"
		ifconfig $vlan_inter down
		brctl delif br0 $vlan_inter 
		vconfig rem $vlan_inter
	done
}
configRouter()
{
	#removeVlan

	if [ "$VLANEnable" == "1" ]; then
		vconfig add eth2.2 $VLANID
		#vconfig set_ingress_map eth2.2.$VLANID 0 $VLANPriority
		vconfig set_egress_map eth2.2.$VLANID 0 $VLANPriority
		ifconfig eth2.$VLANID 0.0.0.0 
		ifconfig eth2.2.$VLANID down
		wan_mac=`nvram_get 2860 WAN_MAC_ADDR`
		if [ "$wan_mac" != "FF:FF:FF:FF:FF:FF" ]; then
				ifconfig eth2.2.$VLANID hw ether $wan_mac
		fi
		ifconfig eth2.2.$VLANID  0.0.0.0
		ifconfig eth2.2.$VLANID up
	fi 
}



configBridge()
{
	#removeVlan
	
	if [ "$VLANEnable" == "1" ]; then
		vconfig add eth2.2 $VLANID
		#vconfig set_ingress_map eth2.2.$VLANID 0 $VLANPriority
		vconfig set_egress_map eth2.2.$VLANID 0 $VLANPriority
		ifconfig eth2.$VLANID 0.0.0.0 
		ifconfig eth2.2.$VLANID down
		wan_mac=`nvram_get 2860 WAN_MAC_ADDR`
		if [ "$wan_mac" != "FF:FF:FF:FF:FF:FF" ]; then
				ifconfig eth2.2.$VLANID hw ether $wan_mac
		fi
		ifconfig eth2.2.$VLANID  0.0.0.0
		ifconfig eth2.2.$VLANID up
	fi 
	brctl addif br0 eth2.2.$VLANID
}

if [ "$1"  = "route" ] ;then
	configRouter
elif [ "$1" = "bridge" ] ;then
	configBridge
else
	echo "++++error $0 $1"
fi

