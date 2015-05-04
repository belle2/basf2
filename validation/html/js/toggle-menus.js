$(document).ready(function(){
    loadcontent();
    $('.reportbug').click( function(){
	window.open('https://belle2.cc.kek.jp/redmine/projects/support/issues/new');
    });
});

$(document).ready(function() {

    $('.revisions').click(function() {
            $('#revisions').slideToggle("fast");
    });
    $('.packages').click(function() {
            $('#packages').slideToggle("fast");
    });
    $('.filters').click(function() {
            $('#filters').slideToggle("fast");
    });
    $('.options').click(function() {
            $('#options').slideToggle("fast");
    });
});


$(document).ready(function(){
    $('#packages').load('packages.html', function() {
	$('input[type="checkbox"][name="packages"]').click(function(){
            $("#"+$(this).attr("value")).toggle();
	});
    });

    $('input[type="checkbox"][name="options"]').click(function(){
        $(".descriptions").toggle();
    });

    $('input[type="checkbox"][name="overview"]').click(function(){
		if($(this).prop("checked")){
			$(".object_wrap").hide();
			$(".plot_matrix").show();
			$(".noplots").show();
			$(".hidebutton").show();
		} else {
			$(".object_wrap").show();
			$(".noplots").hide();
			$(".hidebutton").hide();
		}
    });
    $('input[type="checkbox"][name="showlists"]').click(function(){
		if(!$(this).prop("checked")){
			$(".hideme").hide();
			$(".hidetext").text("(show)");
		} else {
			$(".hideme").show();
			$(".hidetext").text("(hide)");
		}
	});

});
