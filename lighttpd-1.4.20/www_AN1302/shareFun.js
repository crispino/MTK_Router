function display(expr, div)
{
	if (expr != "none")
		document.getElementById(div).style.visibility = "visible";
	else
		document.getElementById(div).style.visibility = "hidden";
	try
	{
		document.getElementById(div).style.display = expr;
	}
	catch (e)
	{
		document.getElementById(div).style.display = "block";
	}
}

function atoi(str, num)
{
	i = 1;
	if (num != 1) {
		while (i != num && str.length != 0) {
			if (str.charAt(0) == '.') {
				i++;
			}
			str = str.substring(1);
		}
		if (i != num)
			return -1;
	}

	for (i=0; i<str.length; i++) {
		if (str.charAt(i) == '.') {
			str = str.substring(0, i);
			break;
		}
	}
	if (str.length == 0)
		return -1;
	return parseInt(str, 10);
}

function checkRange(str, num, min, max)
{
	d = atoi(str, num);
	if (d > max || d < min)
		return false;
	return true;
}

function checkMac(str){
    var len = str.length;
    if(len!=17)
        return false;

    for (var i=0; i<str.length; i++) {
        if((i%3) == 2){
            if(str.charAt(i) == ':')
                continue;
        }else{
            if (    (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
                    (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
                    (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
                continue;
        }
        return false;
    }
    return true;
}

function checkIpAddr(field, ismask)
{
	var ip_addr = field.value.split(".");
	var i;

	if (field.value == "")
	{
		alert("IP鍦板潃涓嶈兘涓虹┖.");
		field.value = field.defaultValue;
		field.focus();
		return false;
	}
	if (ip_addr.length != 4)
	{
			alert('IP鍦板潃鏍煎紡閿欒, 姝ｇ‘渚嬪瓙涓�192.168.0.254.');
			field.value = field.defaultValue;
			field.focus();
			return false;
	}
	for (i=0; i<4 ; i++)
	{
		if (isNaN(ip_addr[i]) == true)
		{
			alert('IP鍦板潃搴旇鏄�0-9涔嬮棿鐨勬暟瀛�.');
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	if (ismask) {
		for (i=0; i<4; i++)
		{
			if ((ip_addr[i] > 255) || (ip_addr[i] < 0))
			{
				alert('瀛愮綉鎺╃爜閿欒, 姝ｇ‘渚嬪瓙涓猴細255.255.255.0.');
				field.value = field.defaultValue;
				field.focus();
				return false;
			}
		}
	}
	else {
		for (i=0; i<3; i++)
		{
			if ((ip_addr[i] > 255) || (ip_addr[i] < 0))
			{
				alert('IP鍦板潃鏍煎紡閿欒, 姝ｇ‘渚嬪瓙涓�192.1168.0.254.');
				field.value = field.defaultValue;
				field.focus();
				return false;
			}
		}
		if ((ip_addr[i] > 254) || (ip_addr[i] < 0))
		{
				alert('IP鍦板潃鏍煎紡閿欒.');
				field.value = field.defaultValue;
				field.focus();
				return false;
		}
	}

	return true;
}

var http_request = false;
function makeRequest(url, content, handler) {
	http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert('鏀惧純:( 鏃犳硶鍒涘缓XMLHTTP瀹炰緥');
		return false;
	}
	http_request.onreadystatechange = handler;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function makeRequest2(url, content, handler, args) {
	var http_request2 = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request2 = new XMLHttpRequest();
		if (http_request2.overrideMimeType) {
			http_request2.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request2 = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
				http_request2 = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (http_request2 == null) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}
	http_request2.onreadystatechange = function(){
		if (http_request2.readyState == 4) {
			if (http_request2.status == 200 || http_request2.status == 304) {
				handler(http_request2, args);
			} else {
				//alert("Call " + url + " failed!!");
			}
		}
	}

	http_request2.open('POST', url, true);
	http_request2.send(content);
}

function submit_reload()
{
	setTimeout("window.location.reload()", 100);
}

function cooldown_time(cool_sec)
{
	document.getElementById("cooldown_time_text").innerHTML = cool_sec;
	if(cool_sec > 1)
		setTimeout(cooldown_time, 1000, cool_sec - 1);
}

function show_waiting(mode)
{
	if(mode == "ON"){
		display("table", "div_waiting_img");
	} else {
		display("none", "div_waiting_img");
	}
}

function disabled_buttons(buttons)
{
	var len = 0;
	var i = 0;

	if (!Array.isArray) {
		Array.isArray = function(arg) {
			return Object.prototype.toString.call(arg) === '[object Array]';
		};
	}

	if(!Array.isArray(buttons))
		return;

	len = buttons.length;
	for(i = 0; i < len; i++){
		document.getElementById(buttons[i]).disabled = true;
	}
}

function submit_form(this_button, reload_sec, buttons,  pre_check_fun)
{
	//console.log("pre_check_fun=["+ pre_check_fun +"]");
	if(pre_check_fun){
		//console.log("pre_check_fun!=NULL");
		if(!pre_check_fun()){
			//console.log("after pre_check_fun");
			return;
		}
	}else{
		//console.log("pre_check_fun==NULL");
	}

	this_button.disabled = true;
	show_waiting("ON");
	disabled_buttons(buttons);
	setTimeout("window.location.reload()", reload_sec * 1000);
	cooldown_time(reload_sec);

	this_button.form.submit();
}

var globalCheckTypeFlag = 1;/*涓轰簡淇濈暀浜ゆ崲鏈轰互鍙�520w鐨勪袱绉嶅垽鏂鏍笺€傚鍔犳鍙橀噺(閲囩敤姝ゅ彉閲忔柟娉曚笉濂姐€備絾鍙妭鐪佹椂闂�)*/

function inputRight(obj) {
	if( typeof(obj) != 'object' ) return;
    obj.style.backgroundImage = "";
    //obj.style.borderColor="";
    obj.setAttribute("title", '');
}

document.getElementsByAttribute = function(attName, attValue, eleRoot, tagName) {
    this.d = eleRoot || document;
    this.tName = tagName || '*';
    var children = this.d.getElementsByTagName(this.tName) || document.all;
    var elements = new Array();
    for (var i = 0; i < children.length; i++) {
        //涓轰簡鍏煎鐏嫄绛夋祻瑙堝櫒鏁呯敤eval("children[i]."+attName)
        //鍥犱负鏌愪簺娴忚鍣ㄧ殑getAttribute涓庨€氳繃.寮曠敤鏃剁敤鐨刟ttName鏄笉涓€鏍风殑濡俢lass涓巆lassName鐨勫尯鍒�
        if ((children[i].getAttribute(attName) || eval("children[i]." + attName)) == attValue) {
            elements.push(children[i]);
        }
    }
    return elements;
}

function allInputRight(formObj) {
    var inputTxt = document.getElementsByAttribute("type", "text", formObj, "input");
    for (var i = 0; i < inputTxt.length; i++) {
        inputRight(inputTxt[i]);
    }

}

function isNum(str) {
	str=str.toString();
    if (str.length == 0 || str == null) return false;

    for (i = 0; i < str.length; i++) {
        if (48 > str.charCodeAt(i) || 57 < str.charCodeAt(i)) {
            return false;
        }
    }

    return true;
}

function isInt(theInt) {

    theInt = trimstr(theInt);
    if ((theInt.length > 1 && theInt.substring(0, 1) == "0") || !isNum(theInt)) return false;
    else return true;
}

function toNumber(str, start, end) {
	str=str.toString();
	if(!end) end = str.length;
	if(!start) start = 0;
    var tempVal = 0;
    var mySign = 1;
    for (i = start; i < end; i++) {
        c = str.charAt(i);
        if (c < '0' || c > '9') {
            if (i != start && (c != '-' || c != '+')) //?
            return - 1;
            if (c == '-') mySign = -1;
        } else tempVal = tempVal * 10 + (c - '0');
    }
    tempVal *= mySign;
    return tempVal;
}

function trimstr(str) {
    str = str.toString();
    if (str == "0") return "0";
    if (str == "") return "";

    while (str.length > 0 && str.indexOf(' ') == 0) {
        str = str.substring(1, str.length);
    }
    while (str.length > 0 && str.lastIndexOf(' ') == str.length - 1) {
        str = str.substring(0, str.length - 1);
    }

    return str;
}

function showfiledtip(tips,tdname){
        if(globalCheckTypeFlag) allInputRight();
	globalCheckTypeFlag = 0;/*璋冪敤璇ユ柟娉曘€傚垯涓嶉噰鐢ㄤ氦鎹㈡満鐨勯敊璇彁绀洪鏍笺€備互闃查噸鍙�*/
	tdname.innerHTML="";//alert(tips);
	if(tips!="") tdname.innerHTML+="<font align=right style='font-size:7px;color=#333333'><img src='images/err.gif'>"+tips+"</font>";
	else  tdname.innerHTML="";
}

function verifyDottedIP(ip_str) {
    if (ip_str == '0.0.0.0') return true;
    var val = 0;
    var i = index = 0;
    while (index < ip_str.lastIndexOf(".")) {
        k = index;
        index = ip_str.indexOf(".", index);
        val = toNumber(ip_str, k, index);
        if (k == 0) if ((val == 0) || (val == 127) || (val >= 224)) {
            return false;
        }
        if (!isInt(ip_str.substring(k, index))) break;
        if (val < 0 || val > 255) break;
        i++;
        index++;
    }
    if (i != 3) return false;
    else {
        if (index == ip_str.length) return false;
        else {
            val = toNumber(ip_str, index, ip_str.length);
            if ((val < 0 || val > 255) || !isInt(ip_str.substring(index, ip_str.length))) return false;
        }
    }

    return true;
}

function re_checkIP(cntrl, tips) {
    var retips = "";
    ip_str = trimstr(cntrl.value);
    if (ip_str != "") {
        if (!verifyDottedIP(ip_str)) {
            retips = tips + "杈撳叆閿欒";
        }
    } else {
        retips = tips + "涓虹┖";
    }
    return retips;
}

/**
* 鍑芥暟鍔熻兘锛氳繑鍥炰紶鍏ュ弬鏁板搴旂殑8浣嶄簩杩涘埗鍊�
* 浼犲叆鍙傛暟锛歩p:鐐瑰垎鍗佽繘鍒剁殑鍊�(0~255),int绫诲瀷鐨勫€硷紝
* 涓昏皟鍑芥暟锛歷alidateMask
* 璋冪敤鍑芥暟锛氭棤
* 杩斿洖鍊�:   ip瀵瑰簲鐨勪簩杩涘埗鍊�(濡傦細浼犲叆255锛岃繑鍥�11111111;浼犲叆1,杩斿洖00000001)
**/
function _checkIput_fomartIP(ip) {
    return (ip + 256).toString(2).substring(1); //鏍煎紡鍖栬緭鍑�(琛ラ浂)
}

/**
* 鍑芥暟鍔熻兘锛氶獙璇佸瓙缃戞帺鐮佺殑鍚堟硶鎬�
* 鍑芥暟浣滆€咃細236F(fuwei236#gmail.com)
* 浼犲叆鍙傛暟锛歁askStr:鐐瑰垎鍗佽繘鍒剁殑瀛愮綉鎺╃爜(濡傦細255.255.255.192) 
* 璋冪敤鍑芥暟锛歘checkIput_fomartIP(ip) 
* 杩斿洖鍊硷細  true:  MaskStr涓哄悎娉曞瓙缃戞帺鐮�
            false: MaskStr涓洪潪娉曞瓙缃戞帺鐮�
**/
function validateMask(MaskStr) {
    /* 鏈夋晥鎬ф牎楠� */
    var IPPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/
    if (!IPPattern.test(MaskStr)) return false;

    /* 妫€鏌ュ煙鍊� */
    var IPArray = MaskStr.split(".");
    var ip1 = parseInt(IPArray[0]);
    var ip2 = parseInt(IPArray[1]);
    var ip3 = parseInt(IPArray[2]);
    var ip4 = parseInt(IPArray[3]);
    if (ip1 < 0 || ip1 > 255
    /* 姣忎釜鍩熷€艰寖鍥�0-255 */
    || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255 || ip4 < 0 || ip4 > 255) {
        return false;
    }
    /* 妫€鏌ヤ簩杩涘埗鍊兼槸鍚﹀悎娉� */
    //鎷兼帴浜岃繘鍒跺瓧绗︿覆
    var ip_binary = _checkIput_fomartIP(ip1) + _checkIput_fomartIP(ip2) + _checkIput_fomartIP(ip3) + _checkIput_fomartIP(ip4);

    if ( - 1 != ip_binary.indexOf("01")) return false;
    return true;
}

function re_checkMask(cntrl, tips) {
    var retips = "";
    ip_str = trimstr(cntrl.value);
    if (ip_str != "") {
        if (!validateMask(ip_str)) {
            retips = tips + "杈撳叆閿欒";
        }
    } else {
        retips = tips + "涓虹┖";
    }
    return retips;
}
