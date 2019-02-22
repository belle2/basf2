/** This file contains all the main functions */

"use strict";

// todo: generally: Maybe it would be better to make data a global variable instead of
// passing it from function to function, potentially having to reload it from the
// server if that chain breaks

// ============================================================================
// Global variables
// ============================================================================

/**
 * Is the JS that allows us to render LaTeX already loaded?
 * @type {boolean}
 */
var latexRenderingLoaded = false;

/**
 * Is the function renderLatex already recursively running?
 * @type {boolean}
 */
var latexRenderingInProgress = false;

/**
 * The number of LaTeX formulas that are on the page.
 * @type {number}
 */
var latexEqnCount = 0;


// ============================================================================
// "Top level" functions, i.e. functions that are called directly from the
// HTML or triggered from there.
// ============================================================================

/**
 * This function gets called from the main page validation.html at the
 * beginning and sets up the revisions sub-menu and then loads the
 * corresponding plots to the default selection of the revisions
 */
function loadRevisions() {
    console.log("Loading revisions from server");
    let revLoadPath = "../revisions";

    $.get(revLoadPath).then(function (data) {
        console.log("Loading done!");

        function setupRevisionLoader(ractive) {

            setDefaultPrebuildOption();

            loadPrebuildRevisions();
            let revs = getDefaultRevisions();
            setRevisions(revs);

            loadSelectedRevisions(data);

            // be ready to load any other revision configuration if user desires
            ractive.on('loadSelectedRevisions', function () {
                loadSelectedRevisions(data);
            });

            ractive.on('loadPrebuildRevisions', function () {
                loadPrebuildRevisions(data);
            });
        }

        setupRactive("revision", '#revisions', data, null, setupRevisionLoader);
    });
}

/** Gets called from the main html page and sets up a small system information
 * submenu.
 */
function setSystemInfo() {
    console.log("Setting system info.");
    $.get("../system_info").done(
        function(data) {
            setupRactive("system_info", '#systeminfo', data);
        }

    ).fail(
        function (){
            console.warn("Could not get system info.")
        }
    );
}

/**
 * Gets called from plot_container, if plot is clicked.
 * the popup must be installed when the user clicks the item. If the popup
 * is installed once Ractive is done creating the template the popups do not work
 * any more if the user clicked on the "Overview" checkbox because new DOM items
 * get created
 * @param itemId
 */
function triggerPopup(itemId) {

    $(`#${itemId}`).magnificPopup({
        type: 'inline',
        // Allow opening popup on middle mouse click. Always set it to true if
        // you don't provide alternative source in href.
        midClick: true
    });
}


/**
 * This function call is triggered by the button under the revisions list
 * "Load selected" and sets up the page with the new set of revisions.
 * @param data revision data
 */
function loadSelectedRevisions(data) {

    let revList = getSelectedRevsList();
    let revString = selectedRevsListToString(revList);

    if (revString === "") {
        alert("Please select at least one tag!");
    }

    console.log(`Loading rev via string '${revString}'.`);

    setupRactiveFromRevision(data, revList);
}

/**
 * Set the dropdown menu that specifies the favorite prebuilt combination of
 * revisions to the value which was last selected by the user or to 'rbn' if
 * no such setting exists.
 * @returns the mode which was set
 */
function setDefaultPrebuildOption(){
    let mode = localStorage.getItem(getStorageId("prebuildRevisionDefault"));
    console.debug(`RECOVERED ${mode}`);
    if (mode == null){
        mode = "rbn";
    }
    // todo: check if this is an allowed mode, else discard!
    $("#prebuilt-select").val(mode);
    return mode;
}

/**
 * This will get the selected prebuilt combination from the drop down menu,
 * get the corresponding revisions and show the corresponding plots.
 * It will also save the value of the drop down menu in localStorage.
 * This function should be bound to the user changing the value in the drop
 * down menu.
 * @param data
 */
function loadPrebuildRevisions(data){
    let selector = $("#prebuilt-select")[0];
    let mode = selector.options[selector.selectedIndex].value;
    localStorage.setItem(getStorageId("prebuildRevisionDefault"), mode);
    console.debug(`Loading prebuild reivision with mode '${mode}'`);
    let revisions = getDefaultRevisions(mode);
    console.debug(`Revisions to load are ${revisions.toString()}`);
    setRevisions(revisions);
    // todo: loadPrebuildRevisions can be either called from loadRevisions where we do not want to load anything yet, or from the trigger of the selection menu. Maybe make this into 2 functions?
    if (typeof data !== 'undefined'){
        loadSelectedRevisions(data);
    }
}

/**
 * Sets the state of the revision checkboxes
 * @parm mode: "all" (all revisions), "r" (last revision only), "n" (last
 *  nightly only), "b" (last build only), "nnn" (all nightlies), "rbn"
 *  (default, last build, nightly and revision).
 */
function getDefaultRevisions(mode="rbn") {
    let allRevisions = getAllRevsList().sort().reverse();

    let referenceRevision = "reference";
    let releaseRevisions = [];
    let buildRevisions = [];
    let nightlyRevisions = [];

    for (let i in allRevisions){
        let rev = allRevisions[i];
        // fixme: This will have problems with sorting. Probably we rather want to have prerelease as a new category!
        if (rev.startsWith("release") || rev.startsWith("prerelease")) {
            releaseRevisions.push(rev);
        }
        if (rev.startsWith("build")) {
            buildRevisions.push(rev);
        }
        if (rev.startsWith("nightly")) {
            nightlyRevisions.push(rev);
        }
    }

    if (mode === "all"){
        return allRevisions;
    }
    else if (mode === "r" && releaseRevisions.length >= 1){
        return [referenceRevision, releaseRevisions[0]];
    }
    else if (mode === "b" && buildRevisions.length >= 1){
        return [referenceRevision, buildRevisions[0]];
    }
    else if (mode === "n" && nightlyRevisions.length >= 1){
        return [referenceRevision, nightlyRevisions[0]];
    }
    else if (mode === "nnn" && nightlyRevisions.length >= 1){
        return [referenceRevision].concat(nightlyRevisions);
    }
    else if (mode === "rbn"){
        // default anyway
    }
    else {
        console.error(`Unknown getDefaultRevisions mode '${mode}'!`);
    }

    let rbnRevisions = [referenceRevision];
    if (releaseRevisions.length >= 1){
        rbnRevisions.push(releaseRevisions[0])
    }
    if (buildRevisions.length >= 1){
        rbnRevisions.push(buildRevisions[0])
    }
    if (nightlyRevisions.length >= 1){
        rbnRevisions.push(nightlyRevisions[0])
    }

    return rbnRevisions
}

/**
 * Set the state of the revision checkboxes in the revision submenu.
 * @param revisionList any revision in this list will be checked, all others
 *  will be unchecked. Any revision in this list which does not have a
 *  corresponding checkbox will be ignored.
 */
function setRevisions(revisionList) {
    $('.reference-checkbox').each(function (i, obj) {
        obj.checked = revisionList.includes(obj.value);
    });
}

// ============================================================================
// Loading
// ============================================================================

/**
 * Gets information about the comparisons and plots (generated when
 * we generate the plots), merges it with the information about the revisions
 * and uses that to set up the package template.
 * If we cannot get the comparison/plot information, then the plots for the
 * current selection of revisions haven't yet been generated and we
 * request them.
 * @param revData
 * @param revList
 */
function setupRactiveFromRevision(revData, revList) {

    // don't event attempt to show comparisons for empty revisions
    if (!Array.isArray(revList) || !revList.length)
        return;

    let revString = selectedRevsListToString(revList);

    // make dynamic
    let comparisonLoadPath = `../comparisons/${revString}`;
    let createComparisonUrl = "../create_comparison";

    console.log(`Loading Comparison 'comparisonLoadPath'`);

    // todo: This SCREAMS to be refactored in some way....
    $.get(comparisonLoadPath).done(function (data) {

        // Get the newest revision within the selection
        // to get information about failed scripts and the
        // log files
        let newestRev = getNewestRevision(revData);

        console.debug(`Newest revision is '${newestRev["label"]}'`);

        // enrich the comparison data with the newest revision in this comparison
        data["newest_revision"] = newestRev;

        // We have two sources of information for scripts and plots:
        // * The comparison object from comparisonLoadPath
        // * The revision object
        // We update the data from the comparison object with additional data
        // from the revision object.
        if (newestRev != null) {
            console.debug("Updating package information.");

            // We now go through all of the packages in the revision object
            // and add the corresponding information to the comparison object.
            // object. For this we create a lookup table
            //    'package name' -> 'index in list'
            // for the comparison object.
            let comparisonDataPkg2Index = {};
            for (let index in data["packages"]) {
                let name = data["packages"][index]["name"];
                comparisonDataPkg2Index[name] = index;
            }

            for (let irev in newestRev["packages"]) {

                // Information to be copied from the revision object:
                let name = newestRev["packages"][irev]["name"];
                let failCount = newestRev["packages"][irev]["fail_count"];
                let scriptfiles = newestRev["packages"][irev]["scriptfiles"];
                let label = newestRev["label"];

                if (name in comparisonDataPkg2Index) {
                    // Found the package in the comparison object
                    // ==> Just add the information
                    let ipkg = comparisonDataPkg2Index [name];

                    data["packages"][ipkg]["fail_count"] = failCount;
                    data["packages"][ipkg]["scriptfiles"] = scriptfiles;
                    // Also store the label of the newest revision as this
                    // is needed to stich together the loading path of
                    // log files
                    data["packages"][ipkg]["newest_revision"] = label;
                } else {
                    // Did not find the package in the comparison object
                    // ==> If there's a reason to display it on the homepage
                    //     (e.g. failed scripts whose logs we want to make
                    //     available, then we need to add a new item to the
                    //     package list of the comparison object).
                    console.debug(
                        `Package '${newestRev["packages"][irev]["name"]}` +
                        "' was found in the revision file, but not in the" +
                        "comparison file. Probably this package did not " +
                        "create a single output file."
                    );
                    if (newestRev["packages"][irev]["scriptfiles"].length > 0) {
                        console.debug(
                            "However it did have failing scripts, so we " +
                            "will make it visible on the validation page. "
                        );
                        // Create a new empty entry with the same information
                        // as above and add it to the data
                        let pkgDict = {};
                        pkgDict["name"] = name;
                        pkgDict["fail_count"] = failCount;
                        pkgDict["scriptfiles"] = scriptfiles;
                        pkgDict["newest_revision"] = label;
                        // Also add keys that usually come from the
                        // comparison file and are nescessary for things to work
                        pkgDict["visible"] = true;
                        pkgDict["comparison_error"] = 0; // else problems in package template
                        data["packages"].push(pkgDict);

                    }
                }
            }
        } else {
            console.debug("Newest rev is null.")
        }

        setupRactive("package", '#packages', data,
            // Wire the clicks on package names
            function (ractive) {

                if ("packages" in data) {
                    // todo: load the package which was last time viewn by the users
                    let firstPackageName = getDefaultPackageName(data["packages"]);
                    if (firstPackageName !== false) {
                        loadValidationPlots(firstPackageName, data);
                    } else {
                        console.warn("No package could be loaded.");
                        $("content").text("No package could be loaded");
                    }
                }
                ractive.on({
                    // todo: why does pycharm complain about this being unused? It's used in package.html
                    load_validation_plots: function (evt) {
                        // This gets called if the user clicks on a package in the
                        // package-selection side menu.

                        // Hide all sub-packages
                        ractive.set('packages.*.display_setting', 'none');

                        let pkgs = ractive.get('packages');

                        let package_name = evt.context.name;

                        // Display sub-packages for this one.
                        if (pkgs != null) {
                            for (let ipkg in pkgs) {
                                if (pkgs[ipkg].name === package_name) {
                                    // disaplay this one
                                    ractive.set(`packages.${ipkg}.display_setting`, 'block');
                                    break;
                                }
                            }

                            // the context will contain the json object which was
                            // used to create this template instance
                            loadValidationPlots(evt.context.name, data);
                        }

                        // Remember that this package was open last
                        localStorage.setItem(getStorageId("packageList"), package_name);
                    }
                });
            });
    }).fail(function () {

        console.log(`Comparison '${revString}' does not exist yet, requesting it`);

        $.ajax({
            url: createComparisonUrl,
            type: "POST",
            contentType: "application/json",
            dataType: "json",
            data: JSON.stringify({
                "revision_list": revList
            })
        }).done(function (data) {
            let key = data["progress_key"];
            beginCreatePlotWait(revList, revList, key, revData);
        });
    });
}


/**
 * Sets up the plot containers with the correct plots corresponding to the
 * selection of the revisions.
 * @param packageLoadName
 * @param data
 */
function loadValidationPlots(packageLoadName, data) {
    console.log(`loadValidationPlots: Loading plots for package '${packageLoadName}'`);

    let loadedPackage = null;

    console.log(`loadValidationPlots: Comparison data for package '${packageLoadName}' loaded`);

    let selected_list = getSelectedRevsList();
    // update the already displayed revision labels with the correct colors
    $(".revision-label").each(function () {

        let label = $(this).text();
        // find the revision with the same label
        for (let i in data["revisions"]) {
            if (data["revisions"][i].label === label) {
                $(this).css("color", data["revisions"][i].color);
            }
        }

        if (selected_list.indexOf(label) < 0) {
            // the one which are not selected will be grayed out
            $(this).css("color", "grey");
        }
    });

    if (packageLoadName === "") {
        packageLoadName = getDefaultPackageName(data["packages"]);
    }

    // Find data of the package by package name
    for (let i in data["packages"]) {
        if (data["packages"][i].name === packageLoadName) {
            loadedPackage = data["packages"][i];
            break;
        }
    }

    // fixme: Shouldn't something happen here??
    if (loadedPackage == null) {
    }

    // create unique ids for each plot, which can be used to create
    // links to individual plot images
    let uniq_plot_id = 1;
    for (let i in loadedPackage["plotfiles"]) {
        for (let ploti in loadedPackage["plotfiles"][i]["plots"]) {
            loadedPackage["plotfiles"][i]["plots"][ploti]["unique_id"] = uniq_plot_id++;
        }
    }

    let wrappedPackage = {packages: [loadedPackage]};

    setupRactive("plot_container", '#content', wrappedPackage, null,
        // on complete
        function (ractive) {
            ractiveValueRecoverSession(ractive, "show_overview");
            ractiveValueRecoverSession(ractive, "show_expert_plots");

            // setup the jquery ui toggle buttons
            // this can only be done here, otherwise the initial values of the toggle buttons
            // will not be correct
            /*
            do not enable jquery ui buttons atm, because the toggle option
            cannot be properly initialized with color
            $("#check_show_overview").button();
            $("#check_show_expert_plots").button();*/

            // make sure changes to the viewing settings are stored right away
            ractive.observe('show_overview', function () {
                ractiveValuePreserveSession(ractive, "show_overview");
            });
            ractive.observe('show_expert_plots', function () {
                ractiveValuePreserveSession(ractive, "show_expert_plots");
            });

            // check if an "empty" entry needs to be added to the script accordion
            if ( $('.failed_script').length > 0) {
                $("#no_failed_scripts").hide();
            }

            if ( $('.finished_script').length > 0) {
                $("#no_finished_scripts").hide();
            }

            if ( $('.skipped_script').length > 0) {
                $("#no_skipped_scripts").hide();
            }

        },
        // on teardown
        function (ractive) {
        },
        // on render
        function () {
            $("#accordion_script_files").accordion({
                heightStyle: "content"
            });
        },
        // on change
        function (ra) {
        }
    );
}

// ============================================================================
// Typesetting
// ============================================================================

/**
 * Load the Ntuple json file from a server and transfer
 * it into a HTML table
 * @param domId
 * @param jsonLoadingPath
 */
function fillNtupleTable(domId, jsonLoadingPath) {
    // move out of the static folder
    $.getJSON(`../${jsonLoadingPath}`, function (data) {
        let items = [];

        // add header
        items.push("<tr>");
        items.push("<th>tag</th>");

        // get the name of each value which is plotted
        for (let rev in data) {
            for (let fig in data[rev]) {
                let val_pair = data[rev][fig];
                items.push(`<th>${val_pair[0]}</th>`);
            }
            break;
        }

        items.push("</tr>");

        // reference first, if available
        $.each(data, function (key) {

            if (key === "reference") {
                items.push("<tr>");
                items.push(`<td>${key}</td>`);
                for (let fig in data[key]) {
                    let val_pair = data[key][fig];
                    items.push(`<td>${val_pair[1]}</td>`);
                }
                items.push("</tr>");
            }
        });

        // now the rest
        $.each(data, function (key) {
            if (key !== "reference") {
                items.push("<tr>");
                items.push(`<td>${key}</td>`);
                for (let fig in data[key]) {
                    let val_pair = data[key][fig];
                    items.push(`<td>${val_pair[1]}</td>`);
                }
                items.push("</tr>");
            }
        });

        $(`#${domId}`).after(items);
    });
}


// ============================================================================
// Small helper functions
// ============================================================================

/**
 * The package that is opened, when the validation page is opened.
 *  Currently that's just picking the page first in alphabetic order
 *  (i.e. analysis) or false if no packages are available.
 * @param packageList A list of package objects (data["packages"])
 * @return {*}
 */
function getDefaultPackageName(packageList) {

    if (packageList.length === 0) {
        console.debug("getDefaultPackageName: No packages available.");
        return false;
    }

    let lastPackage = localStorage.getItem(getStorageId("packageList"));
    if (lastPackage !== null){
        // check if lastPackage is still available
        let found = false;
        for(let i in packageList){
            if (packageList[i].name === lastPackage) {
                found = true;
            }
        }
        // If it is still available, return the name, otherwise proceed
        if (found){
            console.debug(`Opening package '${lastPackage}' because it was opened last`);
            return lastPackage
        }
        else{
            console.debug(`Last package '${lastPackage}' is not available anymore.`);
        }

    }

    let firstPackageName = packageList[0].name;
    if (firstPackageName !== 'undefined') {
        return firstPackageName;
    } else {
        console.debug("getDefaultPackageName: Name of first package undefined.");
        return false;
    }
}


/**
 * Returns array with the names of the selected revisions.
 * @return {Array}
 */
function getSelectedRevsList() {
    let selectedRev = [];
    $('.reference-checkbox').each(function (i, obj) {
        if (obj.checked === true) {
            selectedRev.push(obj.value)
        }
    });
    selectedRev.sort();
    return selectedRev;
}

function getAllRevsList() {
    let revs = [];
    $('.reference-checkbox').each(function (i, obj) {
        revs.push(obj.value)
    });
    revs.sort();
    return revs;
}

/**
 * Returns a string representation of the array of selected revisions.
 * We need that to create folder names & queries
 */
function selectedRevsListToString(selectedRevs) {
    let revString = "";
    for (let i in selectedRevs) {
        if (i > 0)
            revString += "_";
        revString += selectedRevs[i];
    }
    return revString;
}

/**
 * Return the newest revision that is included in the dataset.
 * @param revData
 * @return {*}
 */
function getNewestRevision(revData) {
    let newest = null;
    // deliberately super early date
    let newestData = "2000-00-00 00:00:00";
    let revList = revData["revisions"];

    for (let i in revList) {
        if (revList[i]["label"] !== "reference") {
            if (revList[i]["creation_date"] > newestData) {
                newestData = revList[i]["creation_date"];
                newest = revList[i]
            }
        }
    }

    return newest
}

/**
 * Render any latex formula on the current page and keep on recursively calling
 * this function until it looks like no new elements appear.
 * Note: We wait until we have latex support (via the latexRenderingLoaded
 * global variable) and (via latexRenderingInProgress) also make sure that
 * only one kind of this function is active (including its recursive calls)
 * That means that calling this function is super cheap, so please call it
 * whenever your actions might make any DOM that contains LaTeX appear on the
 * page!
 * @param force do not check latexRenderingInProgress
 * @param irepeat how often did we recursively call this function to check
 *  for left over latex elements (will be reset whenever new LaTeX is rendered)
 * @returns {*}
 */
function renderLatex(force=false, irepeat=0) {
    if (irepeat === 0 && !force && latexRenderingInProgress){
        console.debug("Superfluous level 0 call to renderLatex()");
        return false;
    }

    // Make sure only one instance of this function is running.
    latexRenderingInProgress = true;

    if (!latexRenderingLoaded){
        // Latex rendering is not yet loaded, so let's just call this very
        // function again in 300 ms
        console.debug("Latex rendering requested, but not yet available. Waiting.");
        return setTimeout(() => renderLatex(force=true), 300);
    }

    if ( irepeat === 0 ){
        console.debug("Rendering LaTeX.");
    }

    // In order to see whether there is still new LaTeX code appearing on the
    // page, we count the number of equations. If new LaTeX code appears, we
    // reload it 3 more times (waiting 1s in between). We also reload it at
    // least 3 times in general.
    MathJax.Hub.Queue(
        function () {
            latexEqnCount = MathJax.Hub.getAllJax().length;
        },
        ["Typeset", MathJax.Hub],
        function () {
            let neqn = MathJax.Hub.getAllJax().length;
            let msg = `LaTeX re-rendering: neqn=${neqn}, irepeat=${irepeat}. `;
            if (latexEqnCount !== neqn) {
                // New LaTeX appeared, restart counting.
                irepeat = 0;
            }
            if (irepeat >= 3) {
                console.debug("Stopping " + msg);
            }
            else {
                console.debug("Scheduling " + msg);
                return setTimeout(() => renderLatex(force=true, irepeat=irepeat+1), 1000);
            }
        }
    );
}
