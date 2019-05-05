#!/bin/sh
. /sbin/config.sh
. /sbin/global.sh
##本脚本用来做无线访客网络，连接guest网络的客户端可以正常上网，但不能登录管理界面
# 1）创建一个br1桥，把访客网络接口ra1加入到br1桥上
# 2）启一个br1接口DHCP服务器
# 3）对br桥的web访问进行过滤
# 4）对br1桥的网段NAT

guest_onoff=`nvram_get 2860 Guest_off`
guest_log_web=`nvram_get 2860 Guest_login_web`
lan_ip=`web 2860 sys lanIpAddr`
lan_netmask=`web 2860 sys lanNetmask`
opmode=`nvram_get 2860 OperationMode`

if [ "$CONFIG_RTDEV" != "" ] ;then
	guest_onoff_5G=`nvram_get rtdev Guest_off`
fi

br1_lan_ip1=`echo $lan_ip | cut -d . -f 1`
br1_lan_ip2=`echo $lan_ip | cut -d . -f 2`
br1_lan_ip3=`echo $lan_ip | cut -d . -f 3`
br1_lan_ip4=`echo $lan_ip | cut -d . -f 4`
br1_lan_ip3=`expr $br1_lan_ip3 + 1`

WanMode = 
if [ "$WanMode" = "PPPOE" ]; then
	inter_wan="ppp0"
else	
	inter_wan="eth1"
fi



VLANEnable=`nvram_get 2860 VLANEnable`
VLANID=`nvram_get 2860 VLANID`
if [ "$wanmode" = "DHCP" ]; then
	if [ "$VLANEnable" == "1" ]; then
		inter_wan="eth2.2.$VLANID"
	else
		inter_wan="eth2.2"
	fi
elif [ "$wanmode" = "PPPOE" ]; then
	inter_wan="ppp0"
else	
	inter_wan="eth2.2"
fi

#echo "++++++$br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4"
###保证只起一个进程
udhcpID=`ps | grep udhcp_guest.conf | grep -v "grep" | awk '{print $1}'`

for id in $udhcpID  
do  
kill -9 $id  
echo "killed $id"  
done 

if [ "$guest_onoff" == "0" -o "$guest_onoff_5G" == "0" ] && [ "$opmode" == "1" ]; then
	ifconfig br1 down
	if [ "$guest_log_web" == "1" ]; then
		###2.4G ra1 加入 br1
		if [ "$guest_onoff" == "0" ] ;then
			brctl delif br0 ra1
			brctl addbr br1
			brctl addif br1 ra1
			ifconfig ra1 up
		else
			brctl delif br1 ra1
			brctl addif br0 ra1
			ifconfig  ra1 down
		fi
		
		###5G rai1 加入 br1
		if [ "$CONFIG_RTDEV" != "" ] ;then
			if [ "$guest_onoff_5G" == "0" ] ;then
				brctl delif br0 rai1
				brctl addbr br1
				brctl addif br1 rai1
				ifconfig rai1 up
			else
				brctl delif br1 rai1
				brctl addif br0 rai1
				ifconfig  rai1 down
			fi
		fi
		ifconfig br1 $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4 netmask $lan_netmask up
		
		iptables  -D INPUT -d $lan_ip -i br1 -j DROP 
		iptables  -D INPUT -d $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4 -i br1 -j DROP 
		iptables -t nat -D POSTROUTING -s $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.0/24 -o $inter_wan -j MASQUERADE
		
		iptables  -A INPUT -d $lan_ip -i br1 -j DROP 
		iptables  -A INPUT -d $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4 -i br1 -j DROP 
		iptables -t nat -A POSTROUTING -s $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.0/24 -o $inter_wan -j MASQUERADE
		
		echo "start $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.100" > /var/udhcp_guest.conf
		echo "end $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.200" >>/var/udhcp_guest.conf
		echo "interface br1" >> /var/udhcp_guest.conf
		echo "option subnet 255.255.255.0" >> /var/udhcp_guest.conf
		echo "option dns 168.95.1.1 8.8.8.8" >> /var/udhcp_guest.conf
		echo "option router $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4" >> /var/udhcp_guest.conf
		echo "option lease 86400" >> /var/udhcp_guest.conf
		echo "lease_file /var/udhcpd_guset.leases" >> /var/udhcp_guest.conf
		
		/usr/sbin/udhcpd /var/udhcp_guest.conf
	else 
		echo "wireless guest cat log web"
	fi 
else
	if [ "$guest_log_web" == "1" ]; then
		ifconfig  ra1 down
		ifconfig  rai1 down
		iptables  -D INPUT -d $lan_ip -i br1 -j DROP 
		iptables  -D INPUT -d $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.$br1_lan_ip4 -i br1 -j DROP 
		iptables -t nat -D POSTROUTING -s $br1_lan_ip1.$br1_lan_ip2.$br1_lan_ip3.0/24 -o $inter_wan -j MASQUERADE
		ifconfig  br1 down
		
		brctl delbr br1
	fi
fi 
