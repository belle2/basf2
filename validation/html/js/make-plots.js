$(document).ready(function() {

	// The on-click events if a user requests a new set of revisions:
	$(document.body).on('click', '#regenerate', function() {

		// Get an array with the values of all checked checkboxes, i.e. the
		// revisions requested by the user, and sort it alphabetically
		var selected_revisions = $('[name=revisions]:checked').map(function() {
			return this.value;
		}).get();
		selected_revisions.sort();

		// Join that array into a string, separated by underscores
		var joined_revisions = selected_revisions.join('_');

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
		}).done(function(json) {

			loadcontent(joined_revisions);

			// If however there was a problem with the AJAX request, print the
			// error message that the server sent back
		}).error(function(xhr, status, error) {

			$("#content").html(xhr.responseText);

			// In any case, hide the "Images are being created"-layer again
		}).always(function() {

			$("#outer").hide();

		});

	});

});
