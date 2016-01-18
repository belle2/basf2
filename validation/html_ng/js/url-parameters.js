$(document).ready(function() {

	// Function to parse the parameters passed in the URL
	function getUrlParameter(sParam) {
		var sPageURL = decodeURI(window.location.search.substring(1));
		var sURLVariables = sPageURL.split('&');
		for (var i = 0; i < sURLVariables.length; i++) {
			var sParameterName = sURLVariables[i].split('=');
			if (sParameterName[0] == sParam) {
				return sParameterName[1];
			}
		}
		return null;
	}

	// Get the contents of the GET-variable 'PackageOverview'
	// If this variable does not exist, getUrlParamater will return null
	var PackageOverview = getUrlParameter('PackageOverview');

	// If the variable exists and is false, turn of the Package overview
	if (PackageOverview == 'false') {
		$('input[type="checkbox"][name="overview"]').prop('checked', false);
		$(".nomatrix").each(function() {
			$(this).addClass("show");
		});
	}

	// Check if a set of revisions was specified in the URL
	var Revisions = getUrlParameter('Revisions');

	// If the user actually selected revisions
	if (Revisions != null) {

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
				"input": Revisions
			})

			// If the request was successful, we can reload the content.html,
			// packages.html and revisions.html
		}).done(function(json) {

			loadcontent(Revisions, true);

			// If however there was a problem with the AJAX request, print the
			// error message that the server sent back
		}).error(function(xhr, status, error) {

			$("#content").html(xhr.responseText);

			// In any case, hide the "Images are being created"-layer again
		}).always(function() {
			$("#outer").hide();

		});
	}

});
