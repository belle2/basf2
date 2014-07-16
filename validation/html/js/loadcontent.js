function loadcontent() {
    $("#content").load("content.html", function() {	
	$.get("content.html",function(data){
	    var lines = data.split("\n");
	    var json = jQuery.parseJSON( lines[0].replace("<!-- ", "").replace(" -->", "") );
	    $(".lastmodified").html(json.lastModified);
	    $('input[type="checkbox"][name="revisions"]').each(function(){
		if($.inArray($(this).attr("value"), json.revisions) >= 0){
		    $(this).prop('checked', true);
		}
		else{
		    $(this).prop('checked', false);
		}
	    });
	});		 
    });
}
