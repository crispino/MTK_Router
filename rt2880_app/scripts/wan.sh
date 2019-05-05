#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/wan.sh#1 $
#
# usage: wan.sh
#

. /sbin/global.sh

# stop all
killall -q syslogd
killall -q udhcpc
killall -q pppd
killall -q l2tpd
killall -q openl2tpd

##中继模式uhcpc进程没有杀死
udhcpcID=`ps | grep "udhcpc" | grep -v "grep" | awk '{print $1}'`

for id in $udhcpcID  
do  
kill -9 $id  
echo "killed $id"  
done  

if [ "$opmode" = "3" ]; then
	iwpriv apcli0 set ApCliAutoConnect=1
fi

if [ "$opmode" = "0" -o "$opmode" = "2" -o "$opmode" = "3" ]; then
	hn=`nvram_get 2860 HostName`
	udhcpc -i br0 -h $hn -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
fi

clone_en=`nvram_get 2860 macCloneEnabled`
clone_mac=`nvram_get 2860 macCloneMac`
#MAC Clone: bridge mode doesn't support MAC Clone
if [ "$opmode" != "0" -a "$clone_en" = "1" ]; then
	ifconfig $wan_if down
	if [ "$opmode" = "2" ]; then
		if [ "$CONFIG_RT2860V2_STA" == "m" ]; then
		rmmod rt2860v2_sta_net
		rmmod rt2860v2_sta
		rmmod rt2860v2_sta_util

		insmod -q rt2860v2_sta_util
		insmod -q rt2860v2_sta mac=$clone_mac
		insmod -q rt2860v2_sta_net
		elif [ "$CONFIG_RLT_STA_SUPPORT" == "m" ]; then
			rmmod rlt_wifi_sta
			insmod -q rlt_wifi_sta mac=$clone_mac
		fi
	else
		ifconfig $wan_if hw ether $clone_mac
	fi
	ifconfig $wan_if up
fi

if [ "$wanmode" = "STATIC" -o "$opmode" = "0" -o "$opmode" = "3" ]; then
	#always treat bridge mode having static wan connection
	ip=`nvram_get 2860 wan_ipaddr`
	nm=`nvram_get 2860 wan_netmask`
	gw=`nvram_get 2860 wan_gateway`
	pd=`nvram_get 2860 wan_primary_dns`
	sd=`nvram_get 2860 wan_secondary_dns`

	#lan and wan ip should not be the same except in bridge mode
	if [ "$opmode" != "0" ]; then
		lan_ip=`nvram_get 2860 lan_ipaddr`
		if [ "$ip" = "$lan_ip" ]; then
			echo "wan.sh: warning: WAN's IP address is set identical to LAN"
			exit 0
		fi
	else
		#use lan's ip address instead
		ip=`nvram_get 2860 lan_ipaddr`
		nm=`nvram_get 2860 lan_netmask`
	fi
	ifconfig $wan_if $ip netmask $nm
	route del default
	if [ "$gw" != "" ]; then
	route add default gw $gw
	fi
	config-dns.sh $pd $sd
elif [ "$wanmode" = "DHCP" ]; then
	hn=`nvram_get 2860 wan_dhcp_hn`
	if [ "$hn" != "" ]; then
		udhcpc -i $wan_if -h $hn -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
	else
		udhcpc -i $wan_if -s /sbin/udhcpc.sh -p /var/run/udhcpc.pid &
	fi
elif [ "$wanmode" = "PPPOE" ]; then
	u=`nvram_get 2860 wan_pppoe_user`
	pw=`nvram_get 2860 wan_pppoe_pass`
	pppoe_opmode=`nvram_get 2860 wan_pppoe_opmode`
	pppoe_mtu=`nvram_get 2860 wan_pppoe_mtu`
	# ifconfig $wan_if mtu $ppoe_mtu
	if [ "$pppoe_opmode" = "" ]; then
		echo "pppoecd $wan_if -u $u -p $pw"
		pppoecd $wan_if -u "$u" -p "$pw"
	else
		pppoe_optime=`nvram_get 2860 wan_pppoe_optime`
		echo "config-pppoe.sh $u $pw $wan_if $pppoe_opmode $pppoe_mtu $pppoe_optime"
		config-pppoe.sh $u $pw $wan_if $pppoe_opmode $pppoe_mtu $pppoe_optime 
	fi
elif [ "$wanmode" = "L2TP" ]; then
	srv=`nvram_get 2860 wan_l2tp_server`
	u=`nvram_get 2860 wan_l2tp_user`
	pw=`nvram_get 2860 wan_l2tp_pass`
	mode=`nvram_get 2860 wan_l2tp_mode`
	l2tp_opmode=`nvram_get 2860 wan_l2tp_opmode`
	l2tp_optime=`nvram_get 2860 wan_l2tp_optime`
	if [ "$mode" = "0" ]; then
		ip=`nvram_get 2860 wan_l2tp_ip`
		nm=`nvram_get 2860 wan_l2tp_netmask`
		gw=`nvram_get 2860 wan_l2tp_gateway`
		if [ "$gw" = "" ]; then
			gw="0.0.0.0"
		fi
		config-l2tp.sh static $wan_if $ip $nm $gw $srv $u $pw $l2tp_opmode $l2tp_optime
	else
		config-l2tp.sh dhcp $wan_if $srv $u $pw $l2tp_opmode $l2tp_optime
	fi
elif [ "$wanmode" = "PPTP" ]; then
	srv=`nvram_get 2860 wan_pptp_server`
	u=`nvram_get 2860 wan_pptp_user`
	pw=`nvram_get 2860 wan_pptp_pass`
	mode=`nvram_get 2860 wan_pptp_mode`
	pptp_opmode=`nvram_get 2860 wan_pptp_opmode`
	pptp_optime=`nvram_get 2860 wan_pptp_optime`
	if [ "$mode" = "0" ]; then
		ip=`nvram_get 2860 wan_pptp_ip`
		nm=`nvram_get 2860 wan_pptp_netmask`
		gw=`nvram_get 2860 wan_pptp_gateway`
		if [ "$gw" = "" ]; then
			gw="0.0.0.0"
		fi
		config-pptp.sh static $wan_if $ip $nm $gw $srv $u $pw $pptp_opmode $pptp_optime
	else
		config-pptp.sh dhcp $wan_if $srv $u $pw $pptp_opmode $pptp_optime
	fi
elif [ "$wanmode" = "3G" ]; then
	autoconn3G.sh connect &
else
	echo "wan.sh: unknown wan connection type: $wanmode"
	exit 1
fi

#启动ntp客户端
	echo "start ntpclient"
	ntp.sh

