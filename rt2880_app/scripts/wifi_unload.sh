#!/bin/sh

# Description:
# unload wifi driver and apps to free memory for firmware upload purpose

# Pitfall:
# In 2.4.x the ip of br0 was determined by min(raxx, eth2x),
# so two possible condtions:
# 1) br0 mac = eth2x mac .... just unload wifi driver
# 2) br0 mac = raxx mac  .... unload br0 and wifi driver
#                             then mirror br0 to eth2x
#
# In 2.6.17 later the kernel supports to change br0 MAC with 
# ifconfig command so this script is not needed anymore.
#

. /sbin/global.sh

kill_apps="udhcpd udhcpc syslogd klogd zebra ripd wscd rt2860apd rt61apd inadyn \
iwevent stupid-ftpd smbd ated ntpclient lld2d igmpproxy dnsmasq telnetd miniupnpd \
long_loop"

is_ra0_in_br0=`brctl show | sed -n '/ra0/p'`
is_eth21_in_br0=`brctl show | sed -n '/eth2\.1/p'`
is_usb0_in_br0=`brctl show | sed -n '/usb0/p'`
br0_mirror=eth2
opmode=`nvram_get OperationMode`
ifRaxDown()
{
	ifconfig wds0 down 1>/dev/null 2>&1
	ifconfig wds1 down 1>/dev/null 2>&1
	ifconfig wds2 down 1>/dev/null 2>&1
	ifconfig wds3 down 1>/dev/null 2>&1

	ifconfig apcli0 down 1>/dev/null 2>&1

	ifconfig mesh0 down 1>/dev/null 2>&1
	num=`nvram_get 2860 BssidNum`
	while [ "$num" -gt 0 ]
	do
		num=`expr $num - 1`
		ifconfig ra$num down
	done
	echo -e "\n##### disable 1st wireless interface #####"
}

ifRaixDown()
{
	ifconfig wdsi0 down 1>/dev/null 2>&1
	ifconfig wdsi1 down 1>/dev/null 2>&1
	ifconfig wdsi2 down 1>/dev/null 2>&1
	ifconfig wdsi3 down 1>/dev/null 2>&1
	num=`nvram_get rtdev BssidNum`
	while [ "$num" -gt 0 ]
	do
		num=`expr $num - 1`
		ifconfig rai$num down
	done
	echo -e "\n##### disable 2nd wireless interface #####"
}

unload_ra0()
{
	ifRaxDown
	ifRaixDown
	if [ "$CONFIG_RAETH_SPECIAL_TAG" = "y" ]; then
		if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
			ifconfig eth2.1 down
		else
			ifconfig eth2.5 down
		fi
	else
		ifconfig eth2.2 down
	fi

	rmmod rt2860v2_ap_net 2>/dev/null
	rmmod rt2860v2_ap 2>/dev/null
	rmmod rt2860v2_ap_util 2>/dev/null
	rmmod rt2860v2_sta_net 2>/dev/null
	rmmod rt2860v2_sta 2>/dev/null
	rmmod rt2860v2_sta_util 2>/dev/null
}

unload_ra0br0()
{
	br0_mac=`ifconfig br0 | sed -n '/HWaddr/p' | sed -e 's/.*HWaddr \(.*\)/\1/'`
	br0_ip=`ifconfig br0 | sed -n '/inet addr:/p' | sed -e 's/ *inet addr:\(.*\)  Bcast.*/\1/'`
	br0_netmask=`ifconfig br0 | sed -n '/inet addr:/p' | sed -e 's/.*Mask:\(.*\)/\1/'`
	ra0_mac=`ifconfig ra0 | sed -n '/HWaddr/p' | sed -e 's/.*HWaddr\ \(.*\)/\1/'`

	if [ "$ra0_mac" = "$br0_mac" ]; then
		#destory br0
		ifconfig br0 down
		brctl delbr br0

		unload_ra0

		#mirror br0 to eth2x
		ifconfig $1 down
		ifconfig $1 hw ether $br0_mac
		ifconfig $1 $br0_ip netmask $br0_netmask
		ifconfig $1 up
		if [ "$opmode" = "1" ]; then
			if [ "$CONFIG_WAN_AT_P0" = "y" ]; then
				echo '##### config External Switch vlan partition (WLLLL) #####'
				if [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" -o "$CONFIG_RALINK_MT7621" = "y" ]; then
					echo "initialize external switch (WLLLL)"
				else
					config-vlan.sh 1 WLLLL
				fi
			else
				echo '##### config External Switch vlan partition (LLLLW) #####'
				if [ "$CONFIG_RALINK_RT6855" = "y" -o "$CONFIG_RALINK_RT6855A" = "y" -o "$CONFIG_RALINK_MT7620" = "y" -o "$CONFIG_RALINK_MT7621" = "y" ]; then
					echo "initialize external switch (LLLLW)"
				else
					config-vlan.sh 1 LLLLW
				fi
			fi
		else
			config-vlan.sh 2 0
		fi
	else
		unload_ra0
	fi
}

# unload apps
for apps in $kill_apps
do
	killall -q $apps
done
for apps in $kill_apps
do
	killall -q -9 $apps
done


# unload wifi driver
if [ "$is_ra0_in_br0" == "" ]; then
	unload_ra0
else
	if [ "$is_usb0_in_br0" != "" ]; then
		exit 1
	fi

	if [ "$is_eth21_in_br0" != "" ]; then
		br0_mirror=eth2.1
	fi
    unload_ra0br0 $br0_mirror
fi

echo 3 > /proc/sys/vm/drop_caches
sleep 1

