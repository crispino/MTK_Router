#!/bin/sh

. /sbin/config.sh
. /sbin/global.sh

ALG_ftp=`nvram_get 2860 ALG_ftp`
ALG_sip=`nvram_get 2860 ALG_sip`
ALG_rtsp=`nvram_get 2860 ALG_rtsp`
ALG_l2tp=`nvram_get 2860 ALG_l2tp`

if [ "$CONFIG_ALG" = "y" ]; then

	rmmod nf_nat_ftp 1>/dev/null 2>&1
	rmmod nf_conntrack_ftp 1>/dev/null 2>&1
	rmmod nf_nat_sip 1>/dev/null 2>&1
	rmmod nf_conntrack_sip 1>/dev/null 2>&1
	rmmod nf_nat_rtsp 1>/dev/null 2>&1
	rmmod nf_conntrack_rtsp 1>/dev/null 2>&1
	rmmod nf_nat_proto_gre 1>/dev/null 2>&1
	rmmod nf_conntrack_proto_gre 1>/dev/null 2>&1
	rmmod nf_nat_pptp 1>/dev/null 2>&1
	rmmod nf_conntrack_pptp 1>/dev/null 2>&1

	if [ "$ALG_ftp" = "1" ]; then
		insmod -q nf_conntrack_ftp
		insmod -q nf_nat_ftp
	fi
	
	if [ "$ALG_sip" = "1" ]; then
		insmod -q nf_conntrack_sip
		insmod -q nf_nat_sip
	fi
	
	if [ "$ALG_rtsp" = "1" ]; then
		insmod -q nf_conntrack_rtsp
		insmod -q nf_nat_rtsp
	fi
	
	if [ "$ALG_l2tp" = "1" ]; then
		insmod -q nf_conntrack_proto_gre
		insmod -q nf_nat_proto_gre
		insmod -q nf_conntrack_pptp
		insmod -q nf_nat_pptp
	fi

else
	echo "ALG not support"
fi

