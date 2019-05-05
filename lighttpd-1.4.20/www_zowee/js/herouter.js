function isAllNum(str)
{
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
			continue;
		return 0;
	}
	return 1;
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
function checkMtu(str)
{
	if(!isAllNum(str))
	{
		alert('必须填入0-9的数字');
		return false;
	}
d=parseInt(str, 10);
if (d > 1500 || d < 64)
{   
   alert('MTU必须在64到1500之间');
		return false;
		
}
	return true;
	
}

function checkRange(str, num, min, max)
{
	d = atoi(str, num);
	if (d > max || d < min)
		return false;
	return true;
}

function checkIpAddr(field, ismask)
{
	if (field.value == "") {
		alert("IP地址不能为空");
		field.value = field.defaultValue;
		field.focus();
		return false;
	}

	if (isAllNum(field.value) == 0) {
		alert('必须填入0-9的数字');
		field.value = field.defaultValue;
		field.focus();
		return false;
	}

	if (ismask) {
		if ((!checkRange(field.value, 1, 0, 255)) ||
				(!checkRange(field.value, 2, 0, 255)) ||
				(!checkRange(field.value, 3, 0, 255)) ||
				(!checkRange(field.value, 4, 0, 255)))
		{
			alert('IP地址格式错误');
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	else {
		if ((!checkRange(field.value, 1, 0, 255)) ||
				(!checkRange(field.value, 2, 0, 255)) ||
				(!checkRange(field.value, 3, 0, 255)) ||
				(!checkRange(field.value, 4, 1, 254)))
		{
			alert('IP地址格式错误');
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	return true;
}


function isLogin(ResPonseText){
    if(ResPonseText != null){
        if(-1 != ResPonseText.indexOf("mainPageFlag")){
            return 0;
        }
        else if(-1 != ResPonseText.indexOf("loginPageFlag")){
        	  return 1;   	
        }
        else 
        	  return -1;
    }
}

//重写alert方法  
function Alert(str, sec) {     
    var msgw,msgh,bordercolor;     
    msgw=200;//提示窗口的宽度     
    msgh=100;//提示窗口的高度     
    titleheight=25 //提示窗口标题高度     
    bordercolor="#0055e6";//提示窗口的边框颜色     
    titlecolor="#99CCFF";//提示窗口的标题颜色     
    var sWidth,sHeight;     
    //获取当前窗口尺寸     
    sWidth = document.body.offsetWidth;     
    sHeight = document.body.offsetHeight;     
    //背景div     
    var bgObj=document.createElement("div");     
    bgObj.setAttribute('id','alertbgDiv');     
    bgObj.style.position="absolute";     
    bgObj.style.top="0";     
    bgObj.style.background="white";//"#E8E8E8";     
    bgObj.style.filter="progid:DXImageTransform.Microsoft.Alpha(style=3,opacity=25,finishOpacity=75";     
    bgObj.style.opacity="0.6";     
    bgObj.style.left="0";     
    bgObj.style.width = sWidth + "px";     
    bgObj.style.height = sHeight + "px";     
    bgObj.style.zIndex = "10000";     
    document.body.appendChild(bgObj);     
    //创建提示窗口的div     
    var msgObj = document.createElement("div")     
    msgObj.setAttribute("id","alertmsgDiv");     
    msgObj.setAttribute("align","center");     
    msgObj.style.background="#ece9d8";     
    msgObj.style.border="1px solid " + bordercolor;     
    msgObj.style.position = "absolute";     
    msgObj.style.left = "50%";     
    msgObj.style.font="12px/1.6em Verdana, Geneva, Arial, Helvetica, sans-serif";     
    //窗口距离左侧和顶端的距离      
    msgObj.style.marginLeft = "-100px";     
    //窗口被卷去的高+（屏幕可用工作区高/2）-150     
    msgObj.style.top = document.body.scrollTop+(window.screen.availHeight/2)-280 +"px";     
    msgObj.style.width = msgw + "px";     
    msgObj.style.height = msgh + "px";     
    msgObj.style.textAlign = "center";     
    msgObj.style.lineHeight ="25px";     
    msgObj.style.zIndex = "10001";     
    document.body.appendChild(msgObj);     
    //提示信息标题     
    var title=document.createElement("h4");     
    title.setAttribute("id","alertmsgTitle");     
    title.setAttribute("align","left");     
    title.style.margin="0";     
    title.style.padding="3px";     
    title.style.background = bordercolor;     
    title.style.filter="progid:DXImageTransform.Microsoft.Alpha(startX=20, startY=20, finishX=100, finishY=100,style=1,opacity=75,finishOpacity=100);";     
    title.style.opacity="0.75";     
    title.style.border="1px solid " + bordercolor;     
    title.style.height="18px";     
    title.style.font="12px Verdana, Geneva, Arial, Helvetica, sans-serif";     
    title.style.color="white";     
    title.innerHTML="提示信息";     
    document.getElementById("alertmsgDiv").appendChild(title);     
    //提示信息     
    var txt = document.createElement("p");     
    txt.setAttribute("id","msgTxt");     
    txt.style.margin="16px 0";     
    txt.innerHTML = str;     
    document.getElementById("alertmsgDiv").appendChild(txt);     
//确定button  
//  var btn = document.createElement("button");  
//  btn.value="确   定";  
//  btn.setAttribute('id','btn1');  
//  btn.onclick=function(){  
//      bgObj.style.display = "none";  
//      msgObj.style.display = "none";  
//      title.style.display = "none";  
//        
//  }  
//  document.getElementById("alertmsgDiv").appendChild(btn);  
  
      
    //设置关闭时间     
    window.setTimeout("closewin()", sec*1000);     
}     

function closewin() {     
    document.body.removeChild(document.getElementById("alertbgDiv"));     
    document.getElementById("alertmsgDiv").removeChild(document.getElementById("alertmsgTitle"));     
    document.body.removeChild(document.getElementById("alertmsgDiv"));     
} 
