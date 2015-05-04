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
		if( $('.package_overview').is(':checked') ){
		$('.object_wrap').each(function(){
			$(this).hide();
			$(".plot_matrix").show();
			$(".noplots").show();
			$(".hidebutton").show();
	        });
	    }
	    else {
            $('.object_wrap').each(function(){
			$(this).show();
			$(".noplots").hide();
			$(".hidebutton").hide();
	      	});
	    }
	    if($(window).width() >= 1680){
				if( !$('.show_plot_lists').prop('checked') ){
					$('.show_plot_lists').prop('checked', true);
				$('.hideme').each(function(){
					$(this).show();
				});
				$(".hidetext").each(function(){
					$(this).text("(hide)");
				});
				}
			} else {
	    if( $('.show_plot_lists').prop('checked') ){
			$('.hideme').each(function(){
				$(this).show();
			});
			$(".hidetext").each(function(){
				$(this).text("(hide)");
			});
	    } else {
			$('.hideme').each(function(){
				$(this).hide();
			});
			$(".hidetext").each(function(){
				$(this).text("(show)");
			});
	     }
	 }
	    });




	    $("input.matrix-toggle").on("change", function(){
		
			var checkbox_val = $(this).attr("value");
			$("span#"+checkbox_val).toggle();

	    });
	    
	    $("input.plot-size").on("input", function(){
	    
			var range_val = $(this).val();
			var range_id = $(this).attr("id");
			$(".imidzes_"+range_id).width((range_val-2)+"%");
			$("#slidernumber_"+range_id).text(range_val);
			$("input.slidernumber_"+range_id).val(range_val);

        });
        
        $(".hide").click(function(){

			var hide_id = $(this).attr("id");
			var text = $("#"+hide_id).text();
			$(".wrap_"+hide_id).toggle();
			$("#"+hide_id).text(text == "(hide)" ? "(show)" : "(hide)");

	    });
	    $('.selectall').click(function() { 
			var selectall_id = $(this).attr("id");
			if(!$("input.matrix-"+selectall_id).prop("checked")) { 
				$('.matrix-'+selectall_id).each(function() { 
					$(this).prop('checked', true);              
				});
				$('.imidz-'+selectall_id).each(function(){
					$(this).show();
				});

			}
		});
		$('.selectnone').click(function() {  
			var snone_id = $(this).attr("id");
			if($("input.matrix-"+snone_id).prop("checked")) {
				$('.matrix-'+snone_id).each(function() { 
					$(this).prop('checked', false);             
				});
				$('.imidz-'+snone_id).each(function(){
					$(this).hide();
				});

			}
		});
		$('.imidz').click(function() {
			if($('input[type="checkbox"][name="overview"]').prop('checked')) {
				
				if(!$(this).attr("id")){
					var plot_show_id = $(this).attr("name");
				} else {
					var plot_show_id = $(this).attr("id");
				}
				if($("."+plot_show_id).css("display") == "none") {
					$(".object_wrap").hide();
					$(".plot_matrix").show();
					$("."+plot_show_id).toggle("slow");
				} 
				
			}
		});
		$(".hidebutton").click(function(){
			$(".object_wrap").each(function(){$(this).hide();});
			$(".plot_matrix").each(function(){$(this).show();});
		});
		 
    });
    $("#outer").hide()
}
