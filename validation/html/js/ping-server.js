$(document).ready(function() {
    
    // Send and AJAX Request to check if there is a webserver running
    $.ajax({
        cache: false,
        url: "/ajax/pingserver",
        type: "GET",
        contentType: "application/json",
	
	// If the request fails = there is no webserver running
    }).fail(function() {

	// Warn that some features will not be available
	alert('Warning: No Webserver found! Please note that you can only choose the displayed revisions of basf2 and read the logs if there is a webserver running in the background!');

	// Put a layer div over the revisions menu so it can't be accessed anymore
	$('#revisions').append('<div style="width: 100% !important; height: 100% !important; background:rgba(0,0,0,0.3); position: absolute !important; top: 0; left: 0;">&nbsp;</div>');
	
	// The read logs field is no longer clickable
	$('.readlogs').unbind('click');

    });
});
