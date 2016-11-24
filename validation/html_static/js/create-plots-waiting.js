

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
function install_plotting_progress(rev_string, joined_revisions, progress_key, wait_time, rev_data) {

    // query every second
	var defaultWaitTime = 1000;

	setTimeout(function(){
		
		// ajax request to check for change
		$.ajax({
			url: "../check_comparison_status",
			type: "POST",
			timeout: 1600,
			contentType: "application/json",
			dataType: "json",
			data: JSON.stringify( { "input": progress_key} ) 
			})
			.success(function(ajax_result) {
				if (!ajax_result) {
					// no status yet, keep on querying
					install_plotting_progress(rev_string, joined_revisions,progress_key, defaultWaitTime, rev_data);
				} else {
					// is it only a status message or a message with detailed information
					// on the current progress ?
					if (ajax_result["status"] == "complete") {
						// is the plotting complete and the overlay can be hidden again ?
							//loadcontent(joined_revisions, true);
							//alert("complete !!");
							// load the newly generated plots

							// trigger complete reload !
							// todo: the revision selection seems to be kept, why ?
							//loadRevisions( rev_string, joined_revisions );
                            console.log("Plot creation for revisions " + rev_string + " complete");
                            setupRactiveFromRevision(rev_data, rev_string, rev_list);
							$("#outer").hide();
						}
					else {
						// detailed progress message
						
						// extract information from the json dict					
						var current_package = parseInt( ajax_result["current_package"] )
						var total_package = parseInt( ajax_result["total_package"] )
						var package_name = ajax_result["package_name"]
						
						// display infos on the waiting dialog						
						$("#plot_creation_progress").text("Processing " + current_package + " of " + total_package + " packages");
						$("#plot_creation_package").text("Current Package: " + package_name);
						
						// re-install the timer to check back on the progress
						install_plotting_progress(rev_string, joined_revisions,progress_key, defaultWaitTime, rev_data);
					}
				}
				
			});
	}, wait_time);
}


function beginCreatePlotWait(rev_string, joined_revisions, progress_key, rev_data) {

    // reset status display from previous execution
    $("#plot_creation_progress").text("");
    $("#plot_creation_package").text("");

    // Show the layer div that indicates that new plots are being created
    $("#outer").show();

    // check for a change already very quick, if the plots are already available
    install_plotting_progress( rev_string, joined_revisions,progress_key, 0, rev_data)
}
