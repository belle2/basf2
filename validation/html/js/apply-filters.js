$(document).ready(function(){
    $('input[type="checkbox"][name="filters"]').click(function(){

	var checkedValues = $('[name="filters"]:checked').map(function() {
	    return this.value;
	}).get();

	if(jQuery.isEmptyObject(checkedValues)){
	    $('.object_wrap').each(function() {
		$(this).show();
	    });	    
	}
	else {
	    $('.object_wrap').each(function() {
		$(this).hide();
	    });
	    $.each(checkedValues, function() {
		$( '.'+this ).show();
	    });	
	}
    });
});