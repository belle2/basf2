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
	    
	    /* If the content is reloaded, make sure that the description boxes are displayed or not,
	       in accordance with the checkbox  */
	    if( $('.displaydescriptions').is(':checked') ){
		$('.wrap_boxes').each(function(){
			$(this).show();
	        });
	    }
	    else {
            	$('.wrap_boxes').each(function(){
			$(this).hide();
	      	});
	    }

	});		 
    });
    $("#outer").hide()
}
