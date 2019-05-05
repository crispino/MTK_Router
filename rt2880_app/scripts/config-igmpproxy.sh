#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/config-igmpproxy.sh#1 $
#
# usage: config-igmpproxy.sh <wan_if_name> <lan_if_name>
#

. /sbin/global.sh
                
igmp=`nvram_get 2860 igmpEnabled`

if [ "$igmp" = "1" ]; then
	igmpproxy.sh $wan_ppp_if $lan_if
	killall -q igmpproxy
	igmpproxy
fi

