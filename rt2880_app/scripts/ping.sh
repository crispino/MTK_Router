#!/bin/sh
#
# $Id: //WIFI_SOC/MP/SDK_4_3_0_0/RT288x_SDK/source/user/rt2880_app/scripts/wan.sh#1 $
#
# usage: wan.sh
#
	ping $1 -s $2 -c $3 > /tmp/pingtest & sleep 3; killall ping