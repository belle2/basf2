function resizeWindows() {
    if($(window).width() >= 1680){
	$("#content").css("width", "1462");
	$("#wrap").css("width", "1680");
	$('.floatingHeader').css("width", "1420");
    }
    else {
	$("#content").css("width", "732");
	$("#wrap").css("width", "920");    
	$('.floatingHeader').css("width", "690");
    }
}


$(document).ready(function() {
    resizeWindows();
});

$(window).resize(function() {
    resizeWindows();
});