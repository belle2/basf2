$(document).ready(function(){    
    $('input[type="checkbox"][name="filters"]').click(function(){

	var checkedValues = $('[name="filters"]:checked').map(function() {
	    return this.value;
	}).get();
	if(!jQuery.isEmptyObject(checkedValues)){
		$('.fltr').each(function() {
			$(this).css("display", "none").removeClass("show");
		});
		if(jQuery.inArray("p_value_leq_0_01", checkedValues)>=0){
			$('.p_value_leq_0_01').each(function() {
				$(this).addClass("show");
	    	});	
	    	if($('input[type="checkbox"][name="overview"]').prop("checked")) {
				$('.nomatrix').each(function() {
					$(this).removeClass("show");
	    		});
	    	}   
		}
		else if(jQuery.inArray("p_value_leq_1", checkedValues)>=0) {
	        $('.p_value_leq_1').each(function() {
				$(this).addClass("show");
	    	});
	        if($('input[type="checkbox"][name="overview"]').prop("checked")) {
				$('.nomatrix').each(function() {
					$(this).removeClass("show");
	    		}); 
	    	} 
	    	
	    		
		} else { $('.fltr').each(function(){$(this).addClass("show")}); }
	} else { 
		if($('input[type="checkbox"][name="overview"]').prop("checked")) {
			$('.fltr.matrix-img').each(function(){$(this).addClass("show")});
		} else {
			$('.fltr').each(function(){$(this).addClass("show")}); 
		}
			$('.p_value_leq_1.hlaska').each(function() {
				$(this).removeClass("show");
			});
			$('.p_value_leq_0_01.hlaska').each(function() {
				$(this).removeClass("show");
			});
	}
    });
});
