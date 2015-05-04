$(document).ready(function(){    
    $('input[type="checkbox"][name="filters"]').click(function(){

	var checkedValues = $('[name="filters"]:checked').map(function() {
	    return this.value;
	}).get();
	if(!jQuery.isEmptyObject(checkedValues)){
		$('.fltr').each(function() {
			$(this).hide();
		});
		if(jQuery.inArray("p_value_leq_1", checkedValues)){
			$('.p_value_leq_0_01.hlaska').each(function() {
				$(this).show();
	    	});	
			if($('input[type="checkbox"][name="overview"]').prop("checked")) {
				$('.p_value_leq_0_01.matrix-img').each(function() {
					$(this).show();
	    		});	
			} else {
				$('.p_value_leq_0_01').each(function() {
					$(this).show();
	    		});	
	    	}    
		}
		else if(jQuery.inArray("p_value_leq_0_01", checkedValues)) {
			$('.p_value_leq_1.hlaska').each(function() {
				$(this).show();
			});
			if($('input[type="checkbox"][name="overview"]').prop("checked")) {
	    		$('.p_value_leq_1.matrix-img').each(function() {
					$(this).show();
	    		});
	    	} else {
				$('.p_value_leq_1').each(function() {
					$(this).show();
	    		});
	    	}
	    		
		} else { $('.fltr').each(function(){$(this).hide()}); }
	} else { 
		if($('input[type="checkbox"][name="overview"]').prop("checked")) {
			$('.fltr.matrix-img').each(function(){$(this).show()});
		} else {
			$('.fltr').each(function(){$(this).show()}); 
		}
			$('.p_value_leq_1.hlaska').each(function() {
				$(this).hide();
			});
			$('.p_value_leq_0_01.hlaska').each(function() {
				$(this).hide();
			});
	}
    });
});
