/** This file contains all the main functions */

"use strict";

// ============================================================================
// Global variables
// ============================================================================

/**
 * Is the JS that allows us to render LaTeX already loaded?
 * @type {boolean}
 */
let latexRenderingLoaded = false;

/**
 * Is the function renderLatex already recursively running?
 * @type {boolean}
 */
let latexRenderingInProgress = false;

/**
 * The number of LaTeX formulas that are on the page.
 * @type {number}
 */
let latexEqnCount = 0;

/**
 * This object holds information about the script execution of each revision,
 * and is loaded from results/revisions.json, i.e. corresponds to the
 * json_objects.Revisions object (basically a list of json_objects.Revision
 * objects).
 * This only needs to be initialized ones (namely by loadRevisions() function)
 * @type {null}
 */
let revisionsData = null;

/**
 * This object holds information about the plots that correspond to
 * one set of selected revisions and is loaded from
 * html/plots/<string of revisions>/comparison.json.
 * However, we also copy some information from revisions_data to it.
 * This variable is set (and updated) by setupRactiveFromRevision.
 * We need to update this, whenever the revision selection changes!
 * @type {null}
 */
let comparisonData = null;

// ============================================================================
// "Top level" functions, i.e. functions that are called directly from the
// HTML or triggered from there.
// ============================================================================

/**
 * This function gets called from the main page validation.html at the
 * beginning, loads the revisions data and triggers the set up of the revisions
 * sub-menu and the loading of the default selection of revisions
 */
function loadRevisions() {
    console.log("Loading revisions from server");
    let revLoadPath = "../revisions";

    $.get(revLoadPath).then(function (_data) {
        revisionsData = _data;

        console.log("Loading done!");

        setupRactive(
            "revision",
            '#revisions',
            revisionsData,
            null,
            function (ractive) {
                setDefaultPrebuildOption();
                loadPrebuildRevisions();
            }
        );
    });
}

/** Gets called from the main html page and sets up a small system information
 * submenu.
 */
function setSystemInfo() {
    console.log("Setting system info.");
    $.get("../system_info").done(
        function(system_info) {
            setupRactive("system_info", '#systeminfo', system_info);
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
 */
function loadSelectedRevisions() {

    let revList = getSelectedRevsList();

    console.log(`revList: ${revList}`);

    // Now we need to put the revision that serves as comparison reference first!
    let reference = getReferenceSelection();

    console.log(`Bringing reference '${reference}' to front`);

    if (reference !== undefined){
        // remove reference from the list of revisions
        let referenceIndex = revList.indexOf(reference);
        if (referenceIndex > -1){
            revList.splice(referenceIndex, 1);
        }
        else {
            console.warn("Selected reference not in revisions.")
        }
        // add reference to fron of revisions
        revList.unshift(reference);
    }
    else {
        console.warn("No reference selected.")
    }

    console.log(`revList, after sorting: ${revList}`);

    let revString = selectedRevsListToString(revList);

    if (revString === "") {
        alert("Please select at least one tag!");
    }

    console.log(`Loading revisions '${revList}' via string '${revString}'.`);

    setupRactiveFromRevision(revList);
    loadCustomSelectionRequired(false);
}

/**
 * Set the dropdown menu that specifies the favorite prebuilt combination of
 * revisions to the value which was last selected by the user or to 'rbn' if
 * no such setting exists.
 * @returns the mode which was set
 */
function setDefaultPrebuildOption(){
    setReferenceSelectionOptions();

    let mode = localStorage.getItem(getStorageId("prebuildRevisionDefault"));
    console.debug(`RECOVERED ${mode}`);
    if (mode == null){
        mode = "rbn";
    }
    // todo: check if this is an allowed mode (since it might since have vanished), else discard!
    $("#prebuilt-select").val(mode);
    return mode;
}

/**
 * This will get the selected prebuilt combination from the drop down menu,
 * get the corresponding revisions and show the corresponding plots.
 * It will also save the value of the drop down menu in localStorage.
 * This function should be bound to the user changing the value in the drop
 * down menu.
 */
function loadPrebuildRevisions(){
    let selector = $("#prebuilt-select")[0];
    let mode = selector.options[selector.selectedIndex].value;
    localStorage.setItem(getStorageId("prebuildRevisionDefault"), mode);
    console.debug(`Loading prebuild revision with mode '${mode}'`);
    let revisions = getDefaultRevisions(mode);
    console.debug(`Revisions to load are ${revisions.toString()}`);
    setRevisionSelection(revisions);
    loadSelectedRevisions();
}

/**
 * Return the revision selected as reference
 * @returns
 */
function getReferenceSelection(){
    let selector = $("#reference-select")[0];
    let option = selector.options[selector.selectedIndex].value;
    if ( option === "auto"){
        console.log("Automatically assigning reference.");
        let selected_revs = getSelectedRevsList();
        selected_revs.sort();
        if (selected_revs.includes("reference")){
            return "reference"
        }
        if (selected_revs.length <= 1){
            // impossible to do a comparison anyhow
            return selected_revs[0];
        }
        return getNewestRevision(1);
    }
    else {
        return option;
    }
}

/**
 * Set the dropdown menu 'custom reference selection'
 */
function setReferenceSelectionOptions(){
    let selector = $("#reference-select")[0];
    // Remove all options except for 'automatic':
    selector.options.length = 1;
    let selected_revs = getSelectedRevsList();
    let newest_rev = getNewestRevision();
    console.log("Newest_rev:", newest_rev);
    // Do not allow to select the newest revision as reference (because it
    // is the second comparison element anyhow)
    selected_revs.splice(selected_revs.indexOf(newest_rev), 1);
    console.log("Remaining revs:", selected_revs);
    // Add all remaining revisions as options for the reference:
    for (let rev of selected_revs){
        selector.options[selector.options.length] = new Option(
            rev, rev
        );
    }
    onReferenceSelectionChanged();
}

function onRevisionSelectionChanged(){
    // Because we now have different revisions to pick from, we update the
    // reference dropdown menu.
    setReferenceSelectionOptions();
    // Since this also reset to 'Automatic', this probably also changed our
    // reference selection, so we call
    onReferenceSelectionChanged();
    loadCustomSelectionRequired(true);
}

/**
 * This function gets called whenever the selection of the reference changes.
 * 1. If anything else than the standard revision is selected,
 * the standard revision is disabled (untick and disable checkbox).
 * 2. The reference revision will also be shown in the plots (tick checkbox)
 * 3. The reference revision is shown in bold.
 */
function onReferenceSelectionChanged(){
    let selectedReference = getReferenceSelection();
    let newestRevision = getNewestRevision();

    console.log(`referenceSelectionChanged: Will compare '${newestRevision}' (newest selected revision) with '${selectedReference}' (reference).`);

    // Upddate boldness

    $(`.revision-label`).each(
        (i, obj) => {
            obj.style.fontWeight = "normal";
        }
    );

    $(`#revision-label-${selectedReference}`).each(
        (i, obj) => {
            obj.style.fontWeight = "bold";
        }
    );


    if ( newestRevision !== null ){
        $(`#revision-label-${newestRevision}`).each(
            (i, obj) => {
                obj.style.fontWeight = "bold";
            }
        );
    }
    loadCustomSelectionRequired(true);
}

/**
 * Sets the state of the revision checkboxes
 * @parm mode: "all" (all revisions), "r" (last revision only), "n" (last
 *  nightly only), "b" (last build only), "nnn" (all nightlies), "rbn"
 *  (default, last build, nightly and revision).
 */
function getDefaultRevisions(mode="rbn") {

    let allRevisions = getRevs().sort().reverse();

    let referenceRevision = "reference";
    let releaseRevisions = [];
    let buildRevisions = [];
    let nightlyRevisions = [];

    for (let rev of allRevisions){
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

    // NOTE THE DESIGN PATTERN BELOW:
    // Don't use [referenceRevision, otherList[0]] or similar, because this
    // gives problems if otherList is of length 0!

    // First, we cache the case of running locally: There all of the above
    // revision lists are empty and our only guess is to return allRevisions, which
    // in particular will include 'reference' and 'current' etc.
    if (!nightlyRevisions.length && !buildRevisions.length && !releaseRevisions.length){
        return allRevisions;
    }

    if (mode === "all"){
        return allRevisions;
    }
    else if (mode === "r"){
        return [referenceRevision].concat(releaseRevisions.slice(0, 1));
    }
    else if (mode === "b"){
        return [referenceRevision].concat(buildRevisions.slice(0,1));
    }
    else if (mode === "n"){
        return [referenceRevision].concat(nightlyRevisions.slice(0, 1));
    }
    else if (mode === "nnn"){
        return [referenceRevision].concat(nightlyRevisions);
    }
    else if (mode === "rbn"){
        // default anyway
    }
    else {
        console.error(`getDefaultRevisions: Unknown mode ${mode} ==> falling back to rbn!`);
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
function setRevisionSelection(revisionList) {
    $('.reference-checkbox').each(function (i, obj) {
        obj.checked = revisionList.includes(obj.value);
    });
    onRevisionSelectionChanged();
}

// ============================================================================
// MISC styling
// ============================================================================

function loadCustomSelectionRequired(required){
    let button = $("#button-revision-load")[0];
    if (required === true){
        button.style.color = "#FF0000"
    }
    else{
        button.style.color = "#000000"
    }
}

// ============================================================================
// Loading
// ============================================================================

/**
 * Updates the comparisonData global variable (takes care of enriching
 * comparisonData with some items of revisionsData)
 * @param _comparisonData
 */
function updateComparisonData(_comparisonData) {
    comparisonData = _comparisonData;

    // Get the newest revision within the selection
    // to get information about failed scripts and the
    // log files
    let newestRev = getObjectWithKey(revisionsData["revisions"], "label", getNewestRevision());

    console.debug(`Newest revision is '${newestRev["label"]}'`);

    // enrich the comparison data with the newest revision in this comparison
    comparisonData["newest_revision"] = newestRev;

    // We update comparisonData with some items from revisionsData.
    if (newestRev != null) {
        console.debug("Updating package information.");

        // We now go through all of the packages in the revision object
        // and add the corresponding information to the comparison object.
        // object. For this we create a lookup table
        //    'package name' -> 'index in list'
        // for the comparison object.
        let comparisonDataPkg2Index = {};
        for (let index in comparisonData["packages"]) {
            let name = comparisonData["packages"][index]["name"];
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

                comparisonData["packages"][ipkg]["fail_count"] = failCount;
                comparisonData["packages"][ipkg]["scriptfiles"] = scriptfiles;
                // Also store the label of the newest revision as this
                // is needed to stich together the loading path of
                // log files
                comparisonData["packages"][ipkg]["newest_revision"] = label;
            } else {
                // Did not find the package in the comparison object
                // ==> If there's a reason to display it on the homepage
                //     (e.g. failed scripts whose logs we want to make
                //     available, then we need to add a new item to the
                //     package list of the comparison object).
                console.debug(
                    `Package '${newestRev["packages"][irev]["name"]}` +
                    "' was found in the revision file, but not in the " +
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
                    comparisonData["packages"].push(pkgDict);

                }
            }
        }

    } else {
        console.debug("Newest rev is null.")
    }
}

function packagePaneExpertToggle() {
    template2ractive["package"].set(
        "show_expert_plots",
        $("#check_show_expert_plots")[0].checked
    )
}

/**
 * Gets information about the comparisons and plots (generated when
 * we generate the plots), merges it with the information about the revisions
 * and uses that to set up the package template.
 * If we cannot get the comparison/plot information, then the plots for the
 * current selection of revisions haven't yet been generated and we
 * request them.
 * @param revList
 */
function setupRactiveFromRevision(revList) {

    // don't event attempt to show comparisons for empty revisions
    if (!Array.isArray(revList) || !revList.length)
        return;

    let revString = selectedRevsListToString(revList);

    // make dynamic
    let comparisonLoadPath = `../comparisons/${revString}`;
    let createComparisonUrl = "../create_comparison";

    console.log(`Loading Comparison from '${comparisonLoadPath}'`);

    $.get(comparisonLoadPath).done(function (_comparisonData) {

        updateComparisonData(_comparisonData);

        setupRactive("package", '#packages', comparisonData,
            // Wire the clicks on package names
            function (ractive) {

                if ("packages" in comparisonData) {
                    let firstPackageName = getDefaultPackageName();
                    if (firstPackageName !== false) {
                        loadValidationPlots(firstPackageName);
                    } else {
                        console.warn("No package could be loaded.");
                        $("content").text("No package could be loaded");
                    }
                }

                ractive.on({
                    // todo: why does pycharm complain about this being unused? It's used in package.html
                    load_validation_plots: function (evt) {
                        packagePaneExpertToggle();

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
                            loadValidationPlots(evt.context.name);
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
        }).done(function (progress_data) {
            let key = progress_data["progress_key"];
            beginCreatePlotWait(revList, revList, key, revisionsData);
        });
    });
}


/**
 * Sets up the plot containers with the correct plots corresponding to the
 * selection of the revisions.
 * @param packageLoadName name of the package to be loaded (if unsupplied:
 *  default package name)
 */
function loadValidationPlots(packageLoadName="") {
    console.log(`loadValidationPlots: Loading plots for package '${packageLoadName}'`);

    // update the already displayed revision labels with the correct colors
    $(".revision-label").each( function () {
        $(this).css("color", "grey");
    });

    for (let revision of comparisonData["revisions"]){
        let label = revision["label"];
        let color = revision["color"];

        $(`#revision-label-${label}`).each( function () {
            $(this).css("color", color)
        })
    }


    if (packageLoadName === "") {
        packageLoadName = getDefaultPackageName();
    }
    let loadedPackageData = getObjectWithKey(comparisonData["packages"], "name", packageLoadName);
    if (loadedPackageData == null) {
        // we requested a package which doesn't exist in our data
        return;
    }

    // create unique ids for each plot, which can be used to create
    // links to individual plot images
    let uniq_plot_id = 1;
    for (let i in loadedPackageData["plotfiles"]) {
        for (let ploti in loadedPackageData["plotfiles"][i]["plots"]) {
            loadedPackageData["plotfiles"][i]["plots"][ploti]["unique_id"] = uniq_plot_id++;
        }
    }

    let wrappedPackage = {packages: [loadedPackageData]};

    setupRactive("plot_container", '#content', wrappedPackage, null,
        // on complete
        function (ractive) {
            ractiveValueRecoverSession(ractive, "show_overview");
            ractiveValueRecoverSession(ractive, "show_expert_plots");

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
    $.getJSON(`../${jsonLoadingPath}`, function (ntuple_data) {
        let table_str = "";

        // add header
        table_str += "<tr>";
        table_str += "<th>Tag</th>";

        // get the name of each value which is plotted
        for (let rev in ntuple_data) {
            for (let val_pair of ntuple_data[rev]) {
                table_str += `<th>${val_pair[0]}</th>`;
            }
            break;
        }

        table_str += "</tr>";

        // reference first, if available
        $.each(ntuple_data, function (key) {
            if (key === "reference") {
                table_str += "<tr>";
                table_str += `<td>${key}</td>`;
                for (let val_pair of ntuple_data[key]) {
                    table_str += `<td>${val_pair[1]}</td>`;
                }
                table_str += "</tr>";
            }
        });

        // now the rest
        $.each(ntuple_data, function (key) {
            if (key !== "reference") {
                table_str += "<tr>";
                table_str += `<td>${key}</td>`;
                for (let val_pair of ntuple_data[key]) {
                    table_str += `<td>${val_pair[1]}</td>`;
                }
                table_str += "</tr>";
            }
        });

        $(`#${domId}`).append(table_str);
    });
}


// ============================================================================
// Various getters
// ============================================================================

/**
 * The package that is opened, when the validation page is opened.
 *  Currently that's just picking the page first in alphabetic order
 *  (i.e. analysis) or false if no packages are available.
 * @return {*}
 */
function getDefaultPackageName() {

    let packageList = comparisonData["packages"];

    if (packageList.length === 0) {
        console.debug("getDefaultPackageName: No packages available.");
        return false;
    }

    let lastPackage = localStorage.getItem(getStorageId("packageList"));
    if (lastPackage !== null){
        // check if lastPackage is still available
        let found = getObjectWithKey(packageList, "name", lastPackage) !== null;
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

/**
 * Returns an array with all revision labels.
 * @returns {Array}
 */
function getRevs() {
    let revs = [];
    for (let revision of revisionsData["revisions"]) {
        revs.push(revision["label"]);
    }
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
        if (i > 0) revString += ",";
        revString += selectedRevs[i];
    }
    return revString;
}

// todo: once we properly have dictionaries instead of lists in revisionsData, this should only return the name of the newest revision...
/**
 * Return the newest revision that is selected.
 * @param index: Get the i-th newest revision
 * @return {*}
 */
function getNewestRevision(index=0) {

    let selectedRevs = getSelectedRevsList();
    if (selectedRevs == null || selectedRevs.length === 0){
        console.warn("no selected revisions found!");
    }
    console.log(selectedRevs);
    if ( selectedRevs.length === 1 &&  selectedRevs[0] === "reference"){
        // Need a special treatment in this case, because reference doesn't
        // have a date set!
        return "reference";
    }

    let date2rev = {};
    for (let revision of revisionsData["revisions"]){
        let label = revision["label"];
        if ( ! selectedRevs.includes(label)) continue;
        if ( label === "reference" ) continue;
        let date = revision["creation_date"];
        date2rev[date] = label;
    }

    let dates = Object.keys(date2rev);
    dates.sort().reverse();

    let chosen_date;
    if (index >= dates.length){
        console.log(`getNewestRevision(${index}): Too few revisions (${dates.length}); returning last one instead`);
        chosen_date = dates[-1];
    }
    else {
        chosen_date = dates[index];
    }
    let chosenRev = date2rev[chosen_date];
    console.log(`Revision by newest at index ${index}: ${chosenRev}`);
    return chosenRev
}

/** Find an object in a list of objects by comparing a key with a certain value. */
function getObjectWithKey(objects, key, value) {
    for (let o of objects){
        if (o[key] === value) {
            return o;
        }
    }
    console.warn(`Failed to find object with ${key}==${value}.`);
    console.log("Objects to search in were:");
    console.log(objects);
    return null;
}


// ============================================================================
// LaTeX
// ============================================================================

/**
 * Render any latex formula on the current page and keep on recursively calling
 * this function until it looks like no new elements appear.
 * Note: We wait until we have latex support (via the latexRenderingLoaded
 * global variable) and (via latexRenderingInProgress) also make sure that
 * only one kind of this function is active (including its recursive calls).
 * It will not re-render anything that is already typeset.
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
            let msg = `LaTeX reloading: neqn=${neqn}, irepeat=${irepeat}. `;
            if (latexEqnCount !== neqn) {
                // New LaTeX appeared, restart counting.
                irepeat = 0;
            }
            if (irepeat >= 3) {
                console.debug("Stopping " + msg);
                latexRenderingInProgress = false;
            }
            else {
                console.debug("Scheduling " + msg);
                return setTimeout(() => renderLatex(force=true, irepeat=irepeat+1), 1000);
            }
        }
    );
}
