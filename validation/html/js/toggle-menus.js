$(document).ready(function() {

	// Load contents: revisions, packages, plots+tables
	loadcontent();

	// Set the link to report a new bug
	$('.reportbug').click(function() {
		window.open('https://belle2.cc.kek.jp/redmine/projects/development/issues/new');
	});

	// Enable toggling of the menus
	$('.revisions').click(function() {
		$('#revisions').slideToggle("fast");
	});
	$('.packages').click(function() {
		$('#packages').slideToggle("fast");
	});
	$('.filters').click(function() {
		$('#filters').slideToggle("fast");
	});
	$('.options').click(function() {
		$('#options').slideToggle("fast");
	});

	// Show/hide packages
	$(document.body).on('click', 'input[type="checkbox"][name="packages"]', function() {

		if ($(this).is(':checked')) {
			$("#" + $(this).attr("value")).show();
		} else {
			$("#" + $(this).attr("value")).hide();
		}
	});

	// Show/hide descriptions
	$('input[type="checkbox"][name="options"]').click(function() {
		$(".descriptions").toggle();
	});

	// Enable/disable matrix view
	$('input[type="checkbox"][name="overview"]').click(function() {
		if ($(this).prop("checked")) {
			$(".nomatrix").each(function() {
				$(this).removeClass("show");
			});
		} else {
			$(".nomatrix").each(function() {
				$(this).addClass("show");
			});
		}
	});

	// Hide/how the description boxes for the plot matrix (with download
	// functionality etc.)
	$('input[type="checkbox"][name="showlists"]').click(function() {
		if (!$(this).prop("checked")) {
			$(".hideme").each(function() {
				$(this).css("display", "none");
			});
			$(".hidetext").each(function() {
				$(this).text("(show)");
			});
		} else {
			$(".hideme").each(function() {
				$(this).css("display", "block");
			});
			$(".hidetext").each(function() {
				$(this).text("(hide)");
			});
		}
	});
	
	// Hide/how the expert plots
	$('input[type="checkbox"][name="showexpert"]').click(function() {
		if (!$(this).prop("checked")) {
			$(".expert_plot").each(function() {
				$(this).css("display", "none");
			});
		} else {
			$(".expert_plot").each(function() {
				$(this).css("display", "block");
			});
		}
	});

	// End inheritance of the click-event
	$('[name=selectall]').click(function(e) {
		e.stopPropagation();
	});

	// Give the option to select/deselect all packages at 
	$('[name=selectall]').change(function() {
		$('[name=packages]').each(function() {
			$(this).prop('checked', $('[name=selectall]').is(':checked'));
			if ($(this).is(':checked')) {
				$("#" + $(this).attr("value")).show();
			} else {
				$("#" + $(this).attr("value")).hide();
			}
		});
	});

	// Toggle/untoggle list of ROOT files under packages
        $(document.body).on('click', '.pkg_container_head', function() {
		$('.pkg_container').each( function(){
			$(this).hide();
		});
		$('.pkg_' + $(this).attr('name')).show();
	});

});
