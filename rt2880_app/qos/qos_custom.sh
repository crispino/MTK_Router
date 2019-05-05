#!/bin/sh 


# by Luozf 2017.11.8 
#qos 启用开关，0关闭，1为开启
qos_enable=`nvram_get 2860 QoSEnable_zw`
#上传和下载的总流量（Bits/sec），当QoSUploadBandwidth_zw为custom，QoSUploadBandwidth_custom_zw为总流量（Bits/sec）
QoSUploadBandwidth=`nvram_get 2860 QoSUploadBandwidth_zw`
QoSDownloadBandwidth=`nvram_get 2860 QoSDownloadBandwidth_zw`
QoSUploadBandwidth_custom=`nvram_get 2860 QoSUploadBandwidth_custom_zw`
QoSDownloadBandwidth_custom=`nvram_get 2860 QoSDownloadBandwidth_custom_zw`

VLANEnable=`nvram_get 2860 VLANEnable`
VLANID=`nvram_get 2860 VLANID`
if [ "$VLANEnable" == "1" ]; then
	inter_wan="eth2.2.$VLANID"
else
	inter_wan="eth2.2"
fi
#每一个client的限制规则，name,开关（status）,ip_address,max_upload_speed,max_download_speed;
QoSRules=`nvram_get 2860 QoSRules_zw`
#在字符串后面追加;
QoSRules=${QoSRules}";"

echo "qos_enable=$qos_enable,QoSUploadBandwidth=$QoSUploadBandwidth,QoSDownloadBandwidth=$QoSDownloadBandwidth"
echo "QoSUploadBandwidth_custom=$QoSUploadBandwidth_custom,QoSDownloadBandwidth_custom=$QoSDownloadBandwidth_custom"
echo "QoSRules=[$QoSRules]"

#当用户自定义需要特别处理
if [ ${QoSDownloadBandwidth} == "custom" ]; then
	downSpeed=$QoSDownloadBandwidth_custom
else
	downSpeed=$QoSDownloadBandwidth
fi

if [ ${QoSUploadBandwidth} == "custom" ]; then
	upSpeed=$QoSUploadBandwidth_custom
else
	upSpeed=$QoSUploadBandwidth
fi

echo  "downSpeed=${downSpeed},upSpeed=${upSpeed}"

#启用IMQ虚拟网卡
##imq0 是下载队列
ifconfig  imq0 up
#imq1是上传队列
ifconfig  imq1 up

#删除旧队列
tc qdisc del dev imq0 root 2>/dev/null  
tc qdisc del dev imq1 root 2>/dev/null  

#删除mangle的qos_prerouting_rule_chain和qos_postrouting_rule_chain链
iptables -t mangle -F qos_prerouting_rule_chain
iptables -t mangle -F qos_postrouting_rule_chain
#增加默认规则，有利于提高网速
iptables -t mangle -I qos_prerouting_rule_chain -i !$inter_wan -j RETURN
iptables -t mangle -I qos_postrouting_rule_chain -o !$inter_wan -j RETURN

#开启QoS
if [ ${qos_enable} == "1" ]; then
	#下载设置
	
	#增加下载根队列，未标记数据默认走24
	tc qdisc add dev imq0 handle 11: root htb default 51
	tc class add dev imq0 parent 11: classid 11:1 htb rate ${downSpeed}bit ceil ${downSpeed}bit
	
	#增加上传根队列，未标记数据默认走24
	tc qdisc add dev imq1 handle 12: root htb default 52
	tc class add dev imq1 parent 12: classid 12:1 htb rate ${upSpeed}bit ceil ${upSpeed}bit
	
	num=16
	i=1
	while [ $i -le $num ] 
	do
		rule=`echo ${QoSRules} | cut -d ';' -f $i`

		echo "========QoSRules=${rule}========="
		if [ -z "${rule}"  ]; then
			break;
		fi
		
		name=`echo ${rule} | cut -d "," -f 1`
		status=`echo ${rule}  | cut -d "," -f 2`
		mac_address=`echo ${rule}  | cut -d "," -f 3`
		max_up_speed=`echo ${rule}  | cut -d "," -f 4`
		max_dl_speed=`echo ${rule}  | cut -d "," -f 5`
		ip_address=`web 2860 qos $mac_address`
		if [ ${ip_address} = "Fail" ] ; then
			echo "mac_address=$mac_address not find ip address"
			i=`expr $i + 1`
			continue
		fi
		
		echo "name=$name,status=$status,ip_address=$ip_address,mac_address=$mac_address,max_up_speed=$max_up_speed,max_dl_speed=$max_dl_speed"
		if [ -n "${status}" -a  "${status}" == "1" ]; then
		
		echo "===============debug====================i=[$i]"
		
			#添加子类
			tc class add dev imq0 parent 11:1 classid 11:2$i htb rate ${max_dl_speed}kbps ceil ${max_dl_speed}kbps
			tc class add dev imq1 parent 12:1 classid 12:3$i htb rate ${max_up_speed}kbps ceil ${max_up_speed}kbps

			#为子类添加SFQ公平队列
			tc qdisc add dev imq0 parent 11:2$i handle 2$i: sfq perturb 10
			tc qdisc add dev imq1 parent 12:3$i handle 3$i: sfq perturb 10
			
			#过滤规则
			tc filter add dev imq0 parent 11:0 protocol ip handle 2$i fw flowid 11:2$i
			tc filter add dev imq1 parent 12:0 protocol ip handle 3$i fw flowid 12:3$i
			
			#分类标记数据
			iptables -t mangle -A qos_prerouting_rule_chain -d $ip_address -j MARK --set-mark 2$i
			iptables -t mangle -A qos_postrouting_rule_chain -s $ip_address -j MARK --set-mark 3$i
			#限制特定IP下载速度
			iptables -t mangle -A qos_prerouting_rule_chain -d $ip_address -j RETURN
			iptables -t mangle -A qos_postrouting_rule_chain -s $ip_address -j RETURN
		fi
		
		i=`expr $i + 1`
	done
		
fi

