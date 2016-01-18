

// this function installs a timer with wait_time ( in ms ) which 
// will check for the plot creation progress on the server 
// To identify a specific plotting request, the progress_key
// handed out by the server must be passed here
// The joined_revisions parameter is needed ot reload the 
// correct combination of revisions.
//
// This method will either start a new time, if the plots are 
// not complete yet or will hide the wait dialog if the plotting
// is complete.
function install_plotting_progress(joined_revisions,progress_key, wait_time) {

	var defaultWaitTime = 2000;

	setTimeout(function(){	
		$.ajax({
			url: "/validation/ajax/makeplots-status",
			type: "POST",
			timeout: 1600,
			contentType: "application/json",
			dataType: "json",
			data: JSON.stringify( { "input": progress_key} ) 
			})
			.success(function(ajax_result) {
				if (!ajax_result) {
					// no status yet, keep on querying
					install_plotting_progress(joined_revisions,progress_key, defaultWaitTime);
				} else {
					// is it only a status message or a message with detailed information
					// on the current progress ?
					if ('status' in ajax_result) {
						// only status message
					
						// is the plotting complete and the overlay can be hidden again ?
						if (ajax_result["status"] == "complete") {
							loadcontent(joined_revisions, true);
							$("#outer").hide();
						}
					} else {
						// detailed progress message
						
						// extract information from the json dict					
						var current_package = parseInt( ajax_result["current_package"] )
						var total_package = parseInt( ajax_result["total_package"] )
						var package_name = ajax_result["package_name"]
						
						// display infos on the waiting dialog						
						$("#plot_creation_progress").text("Processing " + current_package + " of " + total_package + " packages");
						$("#plot_creation_package").text("Current Package: " + package_name);
						
						// re-install the timer to check back on the progress
						install_plotting_progress(joined_revisions,progress_key, defaultWaitTime);
					}
				}
				
			});
	}, wait_time);
}


$(document).ready(function() {

	// The on-click events if a user requests a new set of revisions:
	$('#menu').on('click', '#regenerate', function() {

		// Get an array with the values of all checked checkboxes, i.e. the
		// revisions requested by the user, and sort it alphabetically
		var selected_revisions = $('[name=revisions]:checked').map(function() {
			return this.value;
		}).get();
		selected_revisions.sort();

		// Join that array into a string, separated by underscores
		var joined_revisions = selected_revisions.join('_');

		// reset status display from previous execution
		$("#plot_creation_progress").text("");
		$("#plot_creation_package").text("");

		// Show the layer div that indicates that new plots are being created
		$("#outer").show();

		// Now make an AJAX request to the server to create the selected
		// combination of revisions
		$.ajax({
			url: "/validation/ajax/makeplots",
			type: "POST",
			contentType: "application/json",
			dataType: "json",
			data: JSON.stringify({
				"input": selected_revisions
			})

			// If the request was successful, we can reload the content.html,
			// packages.html and revisions.html
		}).done(function(ajax_result) {

			// check for a change already very quick, if the plots are already available
			install_plotting_progress(joined_revisions,ajax_result, 0)
			
		}).error(function(xhr, status, error) {

			$("#content").html(xhr.responseText);

			// In any case, hide the "Images are being created"-layer again
		});

	});

});
