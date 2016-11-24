


function getDefaultPackageName( package_list ) {
    if (package_list.length == 0)
        return false;

   first_package_name = package_list[0].name
   if (first_package_name !='undefined') {
        return first_package_name;
   }else {
        return false;
   }

}

function get_storage_id(keypath) {
    storage_id = "validation_config_" + keypath
    return storage_id
}

/* the popup must be installed when the user clicks the item. If the popup
is installed once Ractive is done creating the template the popups do not work 
any more if the user clicked on the "Overview" checkbox because new DOM items
get created */
function trigger_popup( item_id) {
    $('#' + item_id).magnificPopup({
      type:'inline',
      midClick: true // Allow opening popup on middle mouse click. Always set it to true if you don't provide alternative source in href.
    });
}

function loadValidationPlots(package_load_name, data) {
    console.log("Loading plots for package " + package_load_name);

    // make dynamic
    var revCompare = get_selected_revs_string()
    var revCompare_list = get_selected_revs_list()
    var comparisonLoadPath = "../comparisons/" + revCompare
    var createComparisonUrl = "../create_comparison"

        loaded_package = null

        console.log("Comparison data for package " + package_load_name + " loaded");


        selected_list = get_selected_revs_list();
        // update the already displayed revision labels with the correct colors
        $( ".revision-label" ).each(function( index ) {
            
            label = $( this ).text();
            // find the revision with the same label
            for ( i in data["revisions"]) {
                if (data["revisions"][i].label == label) {
                    $(this).css("color", data["revisions"][i].color);
                }
            }

            if (selected_list.indexOf(label) < 0) {
                // the one which are not selected will be grayed out
                $(this).css("color", "grey");            
            }
        });

        if (package_load_name == ""){
            package_load_name = getDefaultPackageName(data["packages"]);
        }

        for ( i in data["packages"]) {
            if (data["packages"][i].name == package_load_name) {
                loaded_package = data["packages"][i];
                break;
            }
        }

        if (loaded_package == null) {
        }

        // create unique ids for each plot, which can be used to create
        // links to individual plot images
        uniq_plot_id = 1
        for ( var i in loaded_package["plotfiles"] ) {
            for (var ploti in loaded_package["plotfiles"][i]["plots"]) {
                loaded_package["plotfiles"][i]["plots"][ploti]["unique_id"] = uniq_plot_id++;
            }
        }        

        wrapped_package = {packages: [loaded_package]};

        setupRactive("plot_container", '#content', wrapped_package, null,
            // on complete
            function(ractive) {
                // todo: make sure the empty entries in the script accordion are properly filled
                ractive_value_recover(ractive, "show_overview");
                ractive_value_recover(ractive, "show_expert_plots");

                // setup the jquery ui toggle buttons
                // this can only be done here, otherwise the initial values of the toggle buttons 
                // will not be correct
                /* 
                do not enable jquery ui buttons atm, because the toggle option
                cannot be properly initialized with color
                $("#check_show_overview").button();
                $("#check_show_expert_plots").button();*/

                // make sure changes to the viewing settings are stored right away
                ractive.observe( 'show_overview', function ( newValue, oldValue, keypath ) {
                      ractive_value_preserve(ractive,"show_overview");
                });
                ractive.observe( 'show_expert_plots', function ( newValue, oldValue, keypath ) {
                      ractive_value_preserve(ractive,"show_expert_plots");
                });

                // check if an "empty" entry needs to be added to the script accordion
                var count = $('.failed_script').length;
                if (count > 0 ){
                    $("#no_failed_scripts").hide();
                } 

                var count = $('.finished_script').length;
                if (count > 0 ){
                    $("#no_finished_scripts").hide();
                } 

                var count = $('.skipped_script').length;
                if (count > 0 ){
                    $("#no_skipped_scripts").hide();
                } 

            },
            // on teardown
            function(ractive) { 
            },
            // on render
            function(ractive) {
                $("#accordion_script_files").accordion({
                     heightStyle: "content"
                });
            },
            // on change
            function(ra) {
            }
        );
}


// Load the Ntuple json file from a server and transfer
// it into a HTML table
function fill_ntuple_table(dom_id, json_loading_path ) {
    // move out of the static folder 
    $.getJSON( "../" + json_loading_path, function( data ) {
      var items = [];

        // add header 
        items.push("<tr>");
        items.push("<th>tag</th>");

        // get the name of each value which is plotted
        for ( var rev in data) {
        	for ( var fig in data[rev]) {
        		val_pair = data[rev][fig];
        		console.log(val_pair);
        		items.push("<th>"+val_pair[0]+"</th>");
        	}
            break;
        };

        items.push("</tr>");

        	// reference first, if available
          $.each( data, function( key, val ) {
        	  
        	if (key == "reference") {
            items.push("<tr>");
            items.push("<td>" + key + "</td>");
        	for ( var fig in data[key]) {
        		val_pair = data[key][fig];
        		console.log(val_pair);
        		items.push("<td>" + val_pair[1] + "</td>");    
        	}            
            items.push("</tr>");}
          });
        
          // now the rest
          $.each( data, function( key, val ) {
        	if (key != "reference") {
            items.push("<tr>");
            items.push("<td>" + key + "</td>");
        	for ( var fig in data[key]) {
        		val_pair = data[key][fig];
        		console.log(val_pair);
        		items.push("<td>" + val_pair[1] + "</td>");    
        	}            
            items.push("</tr>");}
          });

        $("#" + dom_id).after(items);
    });
}

function get_selected_revs_list() {
    var selected_rev = [];
    $('.reference-checkbox').each(function(i, obj) {
        if (obj.checked == true ) {
            selected_rev.push(obj.value)
        }
    });
    selected_rev.sort();
    return selected_rev;
}

function get_selected_revs_string() {
    var rev_string = ""
    selected_rev = get_selected_revs_list();
    for (var i in selected_rev) {
        if ( i > 0)
            rev_string = rev_string + "_";
        rev_string = rev_string + selected_rev[i];
    }
    return rev_string;
}

function getNewestRevision( rev_data) {
    newest = null
    newest_date = "2000-00-00 00:00:00"
    rev_list = rev_data["revisions"];


    for (var i in rev_list) {
        // todo: have a is_reference entry
        if (rev_list[i]["label"] != "reference") {
            if (rev_list[i]["creation_date"] > newest_date ) {
                newest_date = rev_list[i]["creation_date"]
                newest = rev_list[i]
            }
        }
    }

    return newest
}

function setupRactiveFromRevision(rev_data, rev_string, rev_list)
{
    // make dynamic
    var comparisonLoadPath = "../comparisons/" + rev_string
    var createComparisonUrl = "../create_comparison"

    console.log("Loading Comparison " + comparisonLoadPath);

    $.get(comparisonLoadPath).done(function(data) {
        loaded_package = null

        // get the newest revision within the selection
        // to get information about failed scripts and the
        // log files
        newest_rev = getNewestRevision(rev_data);

        // enrich the comparison data with the newest revision in this comparison
        data["newest_revision"] = newest_rev
        // update the packages with the information from the newest revision
        // if there are no validation results at all, packages entry might not exist
        if (newest_rev != null) {

            for (var irev in newest_rev["packages"]) {
                for (var ipkg in data["packages"]) {
                    if ( data["packages"][ipkg]["name"] == newest_rev["packages"][irev]["name"] ) {
                        data["packages"][ipkg]["fail_count"] = newest_rev["packages"][irev]["fail_count"]
                        data["packages"][ipkg]["scriptfiles"] = newest_rev["packages"][irev]["scriptfiles"]
                        // also store the label of the newest revision as this is needed
                        // to stich together the loading path of log files
                        data["packages"][ipkg]["newest_revision"] = newest_rev["label"]
                    }
                }
            }
        }

        setupRactive("package", '#packages', data,
        // wire the clicks on package names
        function(ractive) {

            if ( "packages" in data) {
                // todo: load the package which was last time viewn by the users
                first_package_name = getDefaultPackageName(data["packages"])
                if (first_package_name != false)
                    loadValidationPlots(first_package_name, data);
            }
            ractive.on({
                load_validation_plots : function( evt ) {
                    // hide all packages sub-e
                    ractive.set( 'packages.*.display_setting', 'none' );
                    
                    pkgs = ractive.get( 'packages');
                    
                    if (pkgs != null) {
                        for (var ipkg in pkgs) {
                        	if ( pkgs[ipkg].name == evt.context.name ) {
                        		// disaplay this one
                        		ractive.set( 'packages.' + ipkg + '.display_setting', 'block' );
                        		break;
                        	}
                        }
                        
                        // the context will contain the json object which was
                        // used to create this template instance
                        loadValidationPlots( evt.context.name, data );
                    }
                }
               });
            });
    }).fail(function() {

        console.log("Comparison " + rev_string + " does not exist yet, requesting it");

        $.ajax({
			url: createComparisonUrl,
			type: "POST",
			contentType: "application/json",
			dataType: "json",
			data: JSON.stringify({
				"revision_list": rev_list
			})
			}).done( function(data)
            {
                key = data["progress_key"];
                beginCreatePlotWait( rev_string, rev_list, key, rev_data );
        });
    });
}

function loadSelectedRevisions(data) {
    rev_string = get_selected_revs_string();
    rev_list = get_selected_revs_list();
    console.log("Loading rev via string " + rev_string );

    setupRactiveFromRevision(data, rev_string, rev_list);
}

function loadRevisions( rev_string , rev_list) {
    if (typeof rev_string === 'undefined') { rev_string = null; }

    console.log("loading revisions from server");
    rev_load_path = "../revisions"

    $.get(rev_load_path).then(function(data) {
        console.log("loading done !");

        function setupRevisionLoader(ractive) {

            // load the defaults for the first time
            if ( rev_string == null ) {
                loadSelectedRevisions(data);
            } else {
                // otherwise, load a specific selection                
                setupRactiveFromRevision(data, rev_string, rev_list);
            }

            // be ready to load any other revision configuration if user desires
            ractive.on( 'loadSelectedRevisions', function () {

                loadSelectedRevisions(data);
            });
        }

        setupRactive("revision", '#revisions', data, null, setupRevisionLoader);
    });
}
