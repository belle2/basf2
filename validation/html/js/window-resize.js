function resizeWindows() {
    if($(window).width() >= 1700){
	$("#content").css("width", "1462");
	$("#wrap").css("width", "1680");
    }
    else {
	$("#content").css("width", "732");
	$("#wrap").css("width", "920");
    }
}


$(document).ready(function() {
    resizeWindows();
});

$(window).resize(function() {
    resizeWindows();
});