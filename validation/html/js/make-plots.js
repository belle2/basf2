$(document).ready(function() {
    
    // Load the list of revisions
    $("#revisions").load("revisions.html", function(){
	
	// If we click the regenerate-button:
	$("#regenerate").click(function() {
	    
	    // Get an array with the values of all checked checkboxes
	    var checkedValues = $('[name=revisions]:checked').map(function() {
		return this.value;
	    }).get();
	    
	    // Show the layer div that indicates that new plots are being created
	    $("#outer").show()
	    
	    // Now make an AJAX-Request with these values
	    $.ajax({ 
		cache: false,
		url: "/ajax/makeplots",
		type: "POST",
		contentType: "application/json",
		dataType: "json",
		data: JSON.stringify({"input": checkedValues})
		
		// Once the request is done (and was successful)
	    }).done(function(json) {

		// Load the 'content.html' which has been modified by the AJAX request
		$("#content").load("content.html", function() {
		    
		    // After we have loaded the content.html into the website, we open the file with
		    // JavaScript and read it linewise
		    $.get("content.html",function(data) {

			// Read the lines
			var lines = data.split("\n");

			// Extract the first line and remove the comment symbols
			var json = jQuery.parseJSON( lines[0].replace("<!-- ", "").replace(" -->", "") );

			// Read out when content.html was modified the last time and write that
			// onto the page (under the navigation bar)
			$(".lastmodified").html(json.lastModified);

			// Now we check which revisions are included in the 'content.html' file we have just
			// loaded. We loop over all revision checkboxes:
			$('input[type="checkbox"][name="revisions"]').each(function(){
			    // If the revision that belongs to the checkbox is contained in the 'content.html'
			    // we activate it
			    if($.inArray($(this).attr("value"), json.revisions) >= 0){
				$(this).prop('checked', true);
			    }
			    // Otherwise we deactivate it
			    else{
				$(this).prop('checked', false);
			    }			
			});
		    });	

		    // Now we loop over all packages checkboxes and hide those packages that are not selected for display
		    $('[name=packages]:not(:checked)').each(function() {
			$("#"+$(this).attr("value")).hide();
		    });
		    
		    // Finally, we call MathJax again to re-typeset all equations in the 'content.html'
		    MathJax.Hub.Queue(["Typeset",MathJax.Hub]);
		});
		
		// If there was a problem with the AJAX request:
	    }).error(function(json) {
		$("#content").html('There was a problem!');

		// Always hide the layer
	    }).always(function() {		
		$("#outer").hide()
	    });
	    
	});    

    }); 
});
