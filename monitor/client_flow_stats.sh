#!/bin/sh
get_macaddress() 
{ 
	#echo "get_macaddress"
	cat /proc/net/arp | grep br | grep -v 00:00:00:00:00:00 | awk '{print $1}' > $1
}

set_rules() 
{
	#echo "set_rules"
	iptables -N UPLOAD_CLIENT
	iptables -N DOWNLOAD_CLIENT
	while read line;do iptables -A FORWARD -s $line -j UPLOAD_CLIENT;done < $1
	while read line;do iptables -A FORWARD -d $line -j DOWNLOAD_CLIENT;done < $1
	echo 1 > $2
}

get_statics_and_del_rule()
{	
	###get statics
	##echo "Download speed:"
	if [ "$3" == "" ];then
		time_inter=$3
	else
		time_inter=10
	fi
	iptables -nvx -L FORWARD | grep DOWNLOAD_CLIENT | awk -v nvar="$time_inter" '{print $9" "int($2/nvar)}' | sort -n -r > /var/DownloadSpeed
	##echo "Upload speed:"
	iptables -nvx -L FORWARD | grep UPLOAD_CLIENT | awk -v nvar="$time_inter" '{print $8" "int($2/nvar)}' | sort -n -r > /var/UploadSpeed
	
	##del ruls
	while read line;do iptables -D FORWARD -s $line -j UPLOAD_CLIENT;done < $1
	while read line;do iptables -D FORWARD -d $line -j DOWNLOAD_CLIENT;done < $1
	iptables -F UPLOAD_CLIENT
	iptables -F DOWNLOAD_CLIENT
	iptables -X UPLOAD_CLIENT
	iptables -X DOWNLOAD_CLIENT
	rm $1
	rm $2
}

if [ "$1" = get_macaddress  ];then 
	if [ $# == 2 ] ; then
		get_macaddress $2
	else
		echo "param number is error"
		exit
	fi
	
fi 

if [ "$1" = set_rules  ] ;then
	if [ $# == 3 ] ; then
		set_rules $2 $3
	else
		echo "param number is error"
		exit
	fi
fi

if [ "$1" = get_statics_and_del_rule  ];then 
	if [ $# == 3 ] ; then
		get_statics_and_del_rule $2 $3
	elif [ $# == 4 ] ;then
		get_statics_and_del_rule $2 $3 $4
	else
		echo "param number is error"
		exit
	fi
	
fi 

