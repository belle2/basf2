function loadcontent(joined_revisions, reload_checkboxes) {

	// Check if we were given a set if revisions.
	// If this is the case, we will load all .html files from the corresponding
	// plots folder.
	// If no revisions are explicitly given (e.g. when the site is loaded
	// initially), we use the default files in the base directory
	if (joined_revisions !== undefined) {
		path = "./plots/" + joined_revisions + "/";
	} else {
		path = "./";
	}

	// Load the list of revisions
	// This does not depend on the selected revisions, hence we can always use
	// the version from the base directory
	$("#revisions").load('./revisions.html');

	// Load the list of packages
	// If the function is called with the reload_checkboxes parameter, that 
	// means the call is coming from make-plots.js, and we might need to
	// restore the selection
	// All this is necessary due to the asynchronous nature of the load()-call.
	// Hence we can't put this into the make-plots.js where it would make more
	// sense.
	if (reload_checkboxes !== undefined) {

		// Remember which packages were selected by the user
		var selected = [];
		$('input[type="checkbox"][name="packages"]').each(function() {
			if ($(this).is(':checked')) {
				selected.push($(this).attr('value'));
			}
		});

		// Now load the packages.html file
		$("#packages").load(path + 'packages.html', function() {

			// Unselect packages that were unselected before (and hide the
			// corresponding packages)
			$('input[type="checkbox"][name="packages"]').each(function() {
				// If it was not selected before, deactive it
				if ($.inArray($(this).attr('value'), selected) == -1) {
					$(this).prop('checked', false);
					$('#' + $(this).attr('value')).hide();
				}
			});
		});
	}
	// If there is no reload_checkboxes parameter, it is because the page is
	// loaded for the first time and we want all checkboxes to be checked!
	else {
		$("#packages").load(path + 'packages.html');
	}

	// Load the contents, i.e. the plots and tables
	$("#content").load(path + 'content.html', function() {

		// After the contents where loaded, we need to update some things
		// that require information from the content.html file 
		$.get(path + 'content.html', function(data) {

			// 1. UPDATE THE "LAST MODIFIED"-INFORMATION
			// Read in all lines from content.html
			var lines = data.split("\n");

			// Extract the first line and remove the comment symbols, then
			// parse the information into an associative JSON array
			var json = jQuery.parseJSON(lines[0].replace("<!-- ", "").replace(" -->", ""));

			// Read out when content.html was modified the last time and write
			// that onto the page (under the navigation bar)
			$(".lastmodified").html(json.lastModified);

			// 2. MAKE SURE THAT PLOTS SHOWN AND PLOTS IN MENU ARE SYNCHRONIZED
			// Now we check which revisions are included in the 'content.html'
			// file we have just loaded. We loop over all revision checkboxes:
			$('input[type="checkbox"][name="revisions"]').each(function() {
				// If the revision that belongs to the checkbox is contained in
				// the 'content.html' we activate it
				if ($.inArray($(this).attr("value"), json.revisions) >= 0) {
					$(this).prop('checked', true);
				}
				// Otherwise we deactivate it
				else {
					$(this).prop('checked', false);
				}
			});

			// 3. HIDE PACKAGES NOT SELECTED FOR DISPLAY
			// Loop over all packages checkboxes and hide those packages that
			// are not selected for display
			$('[name=packages]:not(:checked)').each(function() {
				$("#" + $(this).attr("value")).css("display", "none");
			});

			// 4. SHOW/HIDE BOXES FROM MATRIX VIEW AS SPECIFIED IN OPTIONS
			// If the content is reloaded, make sure that the description boxes
			// are displayed or not, in accordance with the checkbox
			if ($('.displaydescriptions').is(':checked')) {
				$('.wrap_boxes').each(function() {
					$(this).css("display", "block");
				});
			} else {
				$('.wrap_boxes').each(function() {
					$(this).css("display", "none");
				});
			}
			if ($('.package_overview').is(':checked')) {
				$('.nomatrix').each(function() {
					$(this).removeClass("show");
				});
			} else {
				$('.nomatrix').each(function() {
					$(this).addClass("show");
				});
			}

			// 5. UPDATE THE SHARE-PLOT INPUT FIELDS, AND PREPEND THE CURRENT
			//    HOSTNAME ETC. TO THE PATH
			var url = $(location).attr('href');
			var n_index = url.indexOf('index.html');
			var n_hashchar = url.indexOf('#');
			var n_question = url.indexOf('?');

			// If 'index.html' in URL:
			if (n_index != -1){
				var n = n_index;
			} else {
				// First occurence of either '#' or '?'
				if (((n_hashchar==-1) && (n_question!=-1)) || ((n_hashchar!=-1) && (n_question==-1))){
    					var n = Math.max(n_hashchar, n_question);
				} else if (n_hashchar > -1 && n_question > -1) {
    					var n = Math.min(n_hashchar, n_question);
				} else {
    					var n = url.length;
				}
			}

			// Cut off URL after first occurence of '#' or '?'
			var currentURL = url.substr(0, n);

			$('.share_plot').each(function(){
				var curVal = $(this).attr('value');
				$(this).val(currentURL+curVal);
			
			});

			// 6. DO A LOT OF STUFF REGARDING THE MATRIX VIEW THAT NOBODY
			//    BOTHERED TO COMMENT...
			$("input.matrix-toggle").click(function() {
				$("span#" + this.value).toggle();
			});

			$("input.plot-size").on("input", function() {
				var range_val = this.value;
				var range_id = this.id;
				$(".imidzes_" + range_id).css("width", (range_val - 2) + "%");
				$("#slidernumber_" + range_id).text(range_val);
				$("input.slidernumber_" + range_id).val(range_val);
			});

			$(".hide").on("click", function() {
				$(".wrap_" + this.id).toggle();
				$(this).text($(this).text() == "(hide)" ? "(show)" : "(hide)");
			});
			$('.selectall').on("click", function() {
				var checked = this.checked;
				$(this).closest('form').find(':checkbox').each(function() {
					$(this).prop("checked", checked);
					if (checked) {
						$("#" + this.value).css("display", "block");
					} else {
						$("#" + this.value).css("display", "none");
					}
				});
			});
			$('.imidz').on("click", function() {
				if (!$(this).attr("id")) {
					var plot_show_id = this.name;
				} else {
					var plot_show_id = this.id;
				}
				if ($("." + plot_show_id).css("display") == "none") {
					if ($('input[type="checkbox"][name="overview"]').prop('checked')) {
						$(".nomatrix").each(function() {
							$(this).removeClass("show");
						});
					}
					$("." + plot_show_id).addClass("show");
				}

			});
			$(".hidebutton").on("click", function() {
				$(this).closest(".nomatrix").removeClass("show");
			});

		});
	});
}
