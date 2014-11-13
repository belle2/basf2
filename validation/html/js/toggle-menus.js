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
});

$(document).ready(function(){
    $('input[type="checkbox"][name="options"]').click(function(){
        $(".wrap_boxes").toggle();
    });
});