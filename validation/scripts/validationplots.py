#!/usr/bin/env python3

# Normal imports
import glob
import os
import re
import sys
import queue
from typing import Dict, Any, List, Union, Optional
import collections
from multiprocessing import Queue

# Load ROOT
import ROOT

# In case some ROOT files loaded by the validation scripts contain some
# RooFit objects, ROOT will auto-load RooFit. Due to some (yet not
# understood) tear down problem, this results in this error:
# Fatal in <TClass::SetUnloaded>: The TClass for map<TString,double> is being
# unloaded when in state 3 To prevent this, we are loading RooFit here
# before ROOT has a chance to do this
from ROOT import RooFit  # noqa

# The pretty printer. Print prettier :)
import pprint
import json_objects

from basf2 import B2ERROR
import validationpath
from validationplotuple import Plotuple
from validationfunctions import (
    index_from_revision,
    get_style,
    available_revisions,
    terminal_title_line,
)
import validationfunctions

from validationrootobject import RootObject


# Only execute the program if a basf2 release is set up!
if (
    os.environ.get("BELLE2_RELEASE_DIR", None) is None
    and os.environ.get("BELLE2_LOCAL_DIR", None) is None
):
    sys.exit("Error: No basf2 release set up!")

pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)


##############################################################################
#                          Function definitions                              #
##############################################################################


def date_from_revision(
    revision: str, work_folder: str
) -> Optional[Union[int, float]]:
    """
    Takes the name of a revision and returns the 'last modified'-timestamp of
    the corresponding directory, which holds the revision.
    :param revision: A string containing the name of a revision
    :return: The 'last modified'-timestamp of the folder which holds the
        revision
    """

    # Reference files do not need a date since there is always just one
    # version of it, which is presumed to be the latest
    if revision == "reference":
        return 0
    # Regular releases and builds however do have a reasonably well defined
    # 'last modified'-date!
    else:
        revisions = os.listdir(validationpath.get_results_folder(work_folder))
        if revision in revisions:
            return os.path.getmtime(
                validationpath.get_results_tag_folder(work_folder, revision)
            )
        # Otherwise return a None object
        else:
            return None


def merge_nested_list_dicts(a, b):
    """ Given two nested dictionary with same depth that contain lists, return
    'merged' dictionary that contains the joined lists.
    :param a: Dict[Dict[...[Dict[List]]..]]
    :param b: Dict[Dict[...[Dict[List]]..]] (same depth as a)
    :return:
    """

    def _merge_nested_list_dicts(_a, _b):
        """ Merge _b into _a, return _a. """
        for key in _b:
            if key in _a:
                if isinstance(_a[key], dict) and isinstance(_b[key], dict):
                    _merge_nested_list_dicts(_a[key], _b[key])
                else:
                    assert isinstance(_a[key], list)
                    assert isinstance(_b[key], list)
                    _a[key].extend(_b[key])
            else:
                _a[key] = _b[key]
        return _a

    return _merge_nested_list_dicts(a.copy(), b.copy())


def get_plot_files(
    revisions: List[str], work_folder: str
) -> Dict[str, Dict[str, List[str]]]:
    """
    Returns a list of all plot files as absolute paths. For this purpose,
    it loops over all revisions in 'revisions', finds the
    corresponding results folder and collects the plot ROOT files.
    :param revisions: Name of the revisions.
    :param work_folder: Folder that contains the results/ directory
    :return: plot files, i.e. plot ROOT files from the
             requested revisions as dictionary
             {revision: {package: [root files]}}
    """
    # This is where we store the paths of plot ROOT files we've found
    results = collections.defaultdict(lambda: collections.defaultdict(list))

    results_foldername = validationpath.get_results_folder(work_folder)

    # Loop over all requested revisions and look for root files
    # in their package folders
    for revision in revisions:

        if revision == "reference":
            results["reference"] = collections.defaultdict(
                list, get_tracked_reference_files()
            )
            continue

        rev_result_folder = os.path.join(results_foldername, revision)
        if not os.path.isdir(rev_result_folder):
            continue

        packages = os.listdir(rev_result_folder)

        for package in packages:
            package_folder = os.path.join(rev_result_folder, package)
            # find all root files within this package
            root_files = glob.glob(package_folder + "/*.root")
            # append with absolute path
            results[revision][package].extend(
                [os.path.abspath(rf) for rf in root_files]
            )

    return results


def get_tracked_reference_files() -> Dict[str, List[str]]:
    """
    This function loops over the local and central release dir and collects
    the .root-files from the validation-subfolders of the packages. These are
    the files which we will use as references.
    From the central release directory, we collect the files from the release
    which is set up on the machine running this script.
    :return: ROOT files that are located
             in the same folder as the steering files of the package as
             {package: [list of root files]}
    """

    # The base paths to the local and central release directories
    basepaths = {
        "local": os.environ.get("BELLE2_LOCAL_DIR", None),
        "central": os.environ.get("BELLE2_RELEASE_DIR", None),
    }

    # This is where we store the paths of reference ROOT files we've found
    results = {
        "local": collections.defaultdict(list),
        "central": collections.defaultdict(list),
    }

    # validation folder name used by the packages to keep the validation
    # reference plots
    validation_folder_name = "validation"
    validation_test_folder_name = "validation-test"

    # Now collect both local and central ROOT files:
    for location in ["local", "central"]:

        # Skip folders that do not exist (e.g. the central release dir might
        # not be setup if one is working with a completely local version)
        if basepaths[location] is None:
            continue

        # list all available packages
        root = basepaths[location]

        packages = os.listdir(root)

        for package in packages:
            # searches for a validation folder in any top-most folder (package
            # folders) and lists all root-files within
            glob_search = os.path.join(
                root, package, validation_folder_name, "*.root"
            )
            results[location][package].extend(
                [
                    os.path.abspath(f)
                    for f in glob.glob(glob_search)
                    if os.path.isfile(f)
                ]
            )
            # Special case: The validation-test folder in the validation package
            # which is used as a quick test of this framework.
            if package == "validation":
                glob_search = os.path.join(
                    root, package, validation_test_folder_name, "*.root"
                )
                results[location][validation_test_folder_name].extend(
                    [
                        os.path.abspath(f)
                        for f in glob.glob(glob_search)
                        if os.path.isfile(f)
                    ]
                )

    # Now we need to get a rid of all the duplicates: Since local > central,
    # we will delete all central reference files that have a local counterpart.
    # First, loop over all local reference files
    for package, local_files in results["local"].items():
        for local_file in local_files:
            # Remove the location, i.e. reduce the path to /[package]/[filename]
            local_path = local_file.replace(basepaths["local"], "")
            # Now loop over all central reference files
            for central_file in results["central"][package]:
                # Remove the location, i.e.
                # reduce the path to /[package]/[filename]
                central_path = central_file.replace(basepaths["central"], "")
                # If package and filename are the same, we remove the central
                # file from our results list
                if local_path == central_path:
                    results["central"][package].remove(central_file)

    # Return both local and central reference files. The return value does
    # not maintain the distinction between local and central files, because
    # we stored the absolute path to the reference files, and local and
    # central reference files are treated the same anyway.

    ret = {
        package: results["central"][package] + results["local"][package]
        for package in list(results["central"].keys())
        + list(results["central"].keys())
    }

    return ret


def generate_new_plots(
    revisions: List[str],
    work_folder: str,
    process_queue: Optional[Queue] = None,
    root_error_ignore_level=ROOT.kWarning,
) -> None:
    """
    Creates the plots that contain the requested revisions. Each plot (or
    n-tuple, for that matter) is stored in an object of class Plot.
    @param revisions
    @param work_folder: Folder containing results
    @param process_queue: communication queue object, which is used in
           multi-processing mode to report the progress of the plot creating.
    @param root_error_ignore_level: Value for gErrorIgnoreLevel. Default:
        ROOT.kWarning. If set to None, global level will be left unchanged.
    @return: No return value
    """

    print(
        validationfunctions.terminal_title_line(
            "Creating plots for the revision(s) " + ", ".join(revisions) + "."
        )
    )

    # Since we are going to plot, we need to initialize ROOT
    ROOT.gROOT.SetBatch()
    ROOT.gStyle.SetOptStat(1110)
    ROOT.gStyle.SetOptFit(101)

    # Prevent cluttering with ROOT info messages
    if root_error_ignore_level is not None:
        ROOT.gErrorIgnoreLevel = root_error_ignore_level

    # Before we can start plotting, we of course need to collect all
    # ROOT-files that contain data for the plot that we want, e.g. we need to
    # collect all plot ROOT files from the revisions in 'revisions'.
    # The 'reference'-plots, if requested, require special treatment, as they
    # are stored on a different location than the regular plot ROOT files.

    # Collect all plot files, i.e. plot ROOT files from the requested revisions
    if len(revisions) == 0:
        print(
            "No revisions selected for plotting. Returning without "
            "doing anything.",
            file=sys.stderr,
        )
        return

    plot_files = get_plot_files(revisions[1:], work_folder)
    reference_files = get_plot_files(revisions[:1], work_folder)

    # We don't want to have plots that only show the tracked references.
    # Instead we collect all packages that have at least one plot of a new
    # revision in them.
    # Only exception: If 'reference' is the only revision we have, we show it
    # because this is clearly what the user wants
    plot_packages = set()
    only_tracked_reference = set(plot_files.keys()) | set(
        reference_files.keys()
    ) == {"reference"}
    for results in [plot_files, reference_files]:
        for rev in results:
            if rev == "reference" and not only_tracked_reference:
                continue
            for package in results[rev]:
                if results[rev][package]:
                    plot_packages.add(package)

    # The dictionaries {package: {file: {key: [list of root objects]}}}
    plot_p2f2k2o = rootobjects_from_files(
        plot_files, is_reference=False, work_folder=work_folder
    )
    reference_p2f2k2o = rootobjects_from_files(
        reference_files, is_reference=True, work_folder=work_folder
    )

    # Delete all that doesn't belong to a package that we want to plot:
    for package in set(plot_p2f2k2o.keys()) - plot_packages:
        del plot_p2f2k2o[package]
    for package in set(reference_p2f2k2o.keys()) - plot_packages:
        del reference_p2f2k2o[package]

    all_p2f2k2o = merge_nested_list_dicts(plot_p2f2k2o, reference_p2f2k2o)

    # Open the output file
    # First: Create destination directory if it does not yet exist
    content_dir = validationpath.get_html_plots_tag_comparison_folder(
        work_folder, revisions
    )
    comparison_json_file = validationpath.get_html_plots_tag_comparison_json(
        work_folder, revisions
    )

    if not os.path.exists(content_dir):
        os.makedirs(content_dir)

    comparison_packages = []

    # Collect all plotuples for all the files
    all_plotuples = []

    # for every package
    for i, package in enumerate(sorted(list(plot_packages))):

        # Some information to be printed out while the plots are created
        print(
            terminal_title_line(
                f"Creating plots for package: {package}", level=1
            )
        )

        compare_files = []

        # Now we loop over all files that belong to the package to
        # group the plots correctly
        for rootfile in sorted(all_p2f2k2o[package].keys()):
            file_name, file_ext = os.path.splitext(rootfile)

            # Some more information to be printed out while plots are
            # being created
            print(f"Creating plots for file: {rootfile}")

            # A list in which we keep all the plotuples for this file
            plotuples = []

            # report the progress over the queue object, if available
            if process_queue:
                try:
                    process_queue.put_nowait(
                        {
                            "current_package": i,
                            "total_package": len(plot_packages),
                            "status": "running",
                            "package_name": package,
                            "file_name": file_name,
                        }
                    )
                except queue.Full:
                    # message could not be placed, but no problem next message
                    # will maybe work
                    pass

            # Now loop over ALL keys (within a file, objects will be
            # sorted by key)
            compare_plots = []
            compare_ntuples = []
            compare_html_content = []
            has_reference = False

            root_file_meta_data = collections.defaultdict(lambda: None)

            for key in all_p2f2k2o[package][rootfile].keys():
                plotuple = Plotuple(
                    all_p2f2k2o[package][rootfile][key], revisions, work_folder
                )
                plotuple.create_plotuple()
                plotuples.append(plotuple)
                has_reference = plotuple.has_reference()

                if plotuple.type == "TNtuple":
                    compare_ntuples.append(plotuple.create_json_object())
                elif plotuple.type == "TNamed":
                    compare_html_content.append(plotuple.create_json_object())
                elif plotuple.type == "meta":
                    meta_key, meta_value = plotuple.get_meta_information()
                    root_file_meta_data[meta_key] = meta_value
                else:
                    compare_plots.append(plotuple.create_json_object())

            compare_file = json_objects.ComparisonPlotFile(
                title=file_name,
                package=package,
                rootfile=file_name,
                compared_revisions=revisions,
                plots=compare_plots,
                has_reference=has_reference,
                ntuples=compare_ntuples,
                html_content=compare_html_content,
                description=root_file_meta_data["description"],
            )
            compare_files.append(compare_file)

            all_plotuples.extend(plotuples)

        comparison_packages.append(
            json_objects.ComparisonPackage(
                name=package, plotfiles=compare_files
            )
        )
        # Make the command line output more readable
        print()

    print(f"Storing to {comparison_json_file}")

    # create objects for all revisions
    comparison_revs = []

    for i_revision, revision in enumerate(revisions):
        line_color = None
        index = index_from_revision(revision, work_folder)
        if index is not None:
            style = get_style(index)
            line_color = ROOT.gROOT.GetColor(style.GetLineColor()).AsHexString()
        if i_revision == 0:
            line_color = "#000000"
        if line_color is None:
            print(
                f"ERROR: line_color for revision f{revision} could not be set!"
                f" Choosing default color f{line_color}.",
                file=sys.stderr,
            )
        # print("For {} index {} color {}".format(revision, index, line_color))

        # todo the creation date and git_hash of the original revision should
        #  be transferred here
        comparison_revs.append(
            json_objects.ComparisonRevision(label=revision, color=line_color)
        )

    # todo: refactor this information extraction -> json inside a specific
    #  class / method after the plots have been created
    json_objects.dump(
        comparison_json_file,
        json_objects.Comparison(comparison_revs, comparison_packages),
    )

    print_plotting_summary(all_plotuples)


def print_plotting_summary(
    plotuples: List[Plotuple], warning_verbosity=1, chi2_verbosity=1
) -> None:
    """
    Print summary of all plotuples plotted, especially printing information
    about failed comparisons.
    :param plotuples: List of Plotuple objects
    :param warning_verbosity: 0: no information about warnings, 1: write out
        number of warnings per category, 2: report offending scripts
    :param chi2_verbosity: As warning_verbosity but with the results of the
        chi2 comparisons
    :return: None
    """
    print()
    print(terminal_title_line("Summary of plotting", level=0))

    print("Total number of plotuples considered: {}".format(len(plotuples)))

    def pt_key(plotuple):
        """ How we report on this plotuple """
        key = plotuple.key
        if len(key) > 30:
            key = key[:30] + "..."
        rf = os.path.basename(plotuple.rootfile)
        if len(rf) > 30:
            rf = rf[:30] + "..."
        return f"{plotuple.package}/{key}/{rf}"

    n_warnings = 0
    plotuple_no_warning = []
    plotuple_by_warning = collections.defaultdict(list)
    plotuples_by_comparison_result = collections.defaultdict(list)
    for plotuple in plotuples:
        for warning in plotuple.warnings:
            n_warnings += 1
            plotuple_by_warning[warning].append(pt_key(plotuple))
        if not plotuple.warnings:
            plotuple_no_warning.append(pt_key(plotuple))
        plotuples_by_comparison_result[plotuple.comparison_result].append(
            pt_key(plotuple)
        )

    if warning_verbosity:
        print()
        if n_warnings:
            print(f"A total of {n_warnings} warnings were issued.")
            for warning, perpetrators in plotuple_by_warning.items():
                print(
                    f"* '{warning}' was issued by {len(perpetrators)} "
                    f"plotuples"
                )
                if warning_verbosity >= 2:
                    for perpetrator in perpetrators:
                        print(f"  - {perpetrator}")
        else:
            print("No warnings were issued. ")
        print(
            validationfunctions.congratulator(
                total=len(plotuples), success=len(plotuple_no_warning)
            )
        )
        print()

    if chi2_verbosity:
        if not warning_verbosity:
            print()
        print("Chi2 comparisons")
        for result, perpetrators in plotuples_by_comparison_result.items():
            print(
                f"* '{result}' was the result of {len(perpetrators)} "
                f"comparisons"
            )
            if chi2_verbosity >= 2:
                for perpetrator in perpetrators:
                    print(f"  - {perpetrator}")
        score = (
            len(plotuples_by_comparison_result["equal"])
            + 0.75 * len(plotuples_by_comparison_result["not_compared"])
            + 0.5 * len(plotuples_by_comparison_result["warning"])
        )
        print(
            validationfunctions.congratulator(
                rate_name="Weighted score: ",
                total=len(plotuples),
                success=score,
            )
        )
        print()


def rootobjects_from_files(
    root_files_dict: Dict[str, Dict[str, List[str]]],
    is_reference: bool,
    work_folder: str,
) -> Dict[str, Dict[str, Dict[str, List[RootObject]]]]:
    """
    Takes a nested dictionary of root file paths for different revisions
    and returns a (differently!) nested dictionary of root file objects.

    :param root_files_dict: The dict of all *.root files which shall be
        read in and for which the corresponding RootObjects shall be created:
        {revision: {package: [root file]}}
    :param is_reference: Boolean value indicating if the objects are
        reference objects or not.
    :param work_folder:
    :return: {package: {file: {key: [list of root objects]}}}
    """

    # Return value: {package: {key: objects}}
    return_dict = collections.defaultdict(
        lambda: collections.defaultdict(lambda: collections.defaultdict(list))
    )

    # Now loop over all given
    for revision, package2root_files in root_files_dict.items():
        for package, root_files in package2root_files.items():
            for root_file in root_files:
                key2objects = rootobjects_from_file(
                    root_file, package, revision, is_reference, work_folder
                )
                for key, objects in key2objects.items():
                    return_dict[package][os.path.basename(root_file)][
                        key
                    ].extend(objects)

    return return_dict


def get_root_object_type(root_object: ROOT.TObject) -> str:
    """
    Get the type of the ROOT object as a string in a way that makes sense to us.
    In particular, "" is returned if we have a ROOT object that is of no
    use to us.
    :param root_object: ROOT TObject
    :return: type as string if the ROOT object
    """
    if root_object.InheritsFrom("TNtuple"):
        return "TNtuple"
    # this will also match TProfile, as this root class derives from
    # TH1D
    elif root_object.InheritsFrom("TH1"):
        if root_object.InheritsFrom("TH2"):
            return "TH2"
        else:
            return "TH1"
    # TEfficiency barks and quarks like a TProfile, but is unfortunately not
    elif root_object.InheritsFrom("TEfficiency"):
        return "TEfficiency"
    elif root_object.InheritsFrom("TGraph"):
        return "TGraph"
    elif root_object.ClassName() == "TNamed":
        return "TNamed"
    elif root_object.InheritsFrom("TASImage"):
        return "TASImage"
    else:
        return ""


def get_metadata(root_object: ROOT.TObject) -> Dict[str, Any]:
    """ Extract metadata (description, checks etc.) from a ROOT object
    :param root_object ROOT TObject
    """
    root_object_type = get_root_object_type(root_object)

    metadata = {
        "description": "n/a",
        "check": "n/a",
        "contact": "n/a",
        "metaoptions": [],
    }

    # todo [ref, medium]: we should incorporate this in the MetaOptionParser and
    #   never pass them around as a list in the first place
    def metaoption_str_to_list(metaoption_str):
        return [opt.strip() for opt in metaoption_str.split(",") if opt.strip()]

    if root_object_type in ["TH1", "TH2", "TEfficiency", "TGraph"]:
        _metadata = {
            e.GetName(): e.GetTitle() for e in root_object.GetListOfFunctions()
        }

        metadata["description"] = _metadata.get("Description", "n/a")
        metadata["check"] = _metadata.get("Check", "n/a")
        metadata["contact"] = _metadata.get("Contact", "n/a")

        metadata["metaoptions"] = metaoption_str_to_list(
            _metadata.get("MetaOptions", "")
        )

    elif root_object_type == "TNtuple":
        _description = root_object.GetAlias("Description")
        _check = root_object.GetAlias("Check")
        _contact = root_object.GetAlias("Contact")

        if _description:
            metadata["description"] = _description
        if _check:
            metadata["check"] = _check
        if _contact:
            metadata["contact"] = _contact

        _metaoptions_str = root_object.GetAlias("MetaOptions")
        if _metaoptions_str:
            metadata["metaoptions"] = metaoption_str_to_list(_metaoptions_str)

    # TODO: Can we somehow incorporate TNameds and TASImages?

    return metadata


def rootobjects_from_file(
    root_file: str,
    package: str,
    revision: str,
    is_reference: bool,
    work_folder: str,
) -> Dict[str, List[RootObject]]:
    """
    Takes a root file, loops over its contents and creates the RootObjects
    for it.

    :param root_file: The *.root file which shall be read in and for which the
        corresponding RootObjects shall be created
    :param package:
    :param revision:
    :param work_folder:
    :param is_reference: Boolean value indicating if the object is a
        reference object or not.
    :return: package, {key: [list of root objects]}. Note: The list will
        contain only one root object right now, because package + root file
        basename key uniquely determine it, but later we will merge this list
        with files from other revisions. In case of errors, it returns an
        empty dictionary.
    """

    # Return value: {key: root object}
    key2object = collections.defaultdict(list)

    # Open the file with ROOT
    # In case of errors, simply return an empty key2object dictionary
    tfile = None
    try:
        tfile = ROOT.TFile(root_file)
        if not tfile or not tfile.IsOpen():
            B2ERROR(f"The file {root_file} can not be opened. Skipping it.")
            return key2object
    except OSError as e:
        B2ERROR(f"{e}. Skipping it.")
        return key2object

    # Get the 'last modified' timestamp of the revision that contains our
    # current root_file
    dir_date = date_from_revision(revision, work_folder)

    # Loop over all Keys in that ROOT-File
    for key in tfile.GetListOfKeys():
        name = key.GetName()

        # temporary workaround for dbstore files located (wrongly)
        # in the validation results folder
        if re.search(".*dbstore.*root", root_file):
            continue

        # Get the ROOT object that belongs to that Key. If there is no
        # object, continue
        root_object = tfile.Get(name)
        if not root_object:
            continue

        root_object_type = get_root_object_type(root_object)
        if not root_object_type:
            # get_root_object_type returns "" for any type that we're not
            # interested in
            continue

        # Ensure that the data read from the ROOT files lives on even
        # after the ROOT file is closed
        if root_object.InheritsFrom("TH1"):
            root_object.SetDirectory(0)

        metadata = get_metadata(root_object)

        if root_object_type == "TNtuple":
            # Go to first entry in the n-tuple
            root_object.GetEntry(0)

            # Storage for the values of the n-tuple. We use a dictionary,
            # because we can't access the n-tuple's values anymore after
            # closing the ROOT file (<=> histograms)
            ntuple_values = {}
            for leaf in root_object.GetListOfLeaves():
                ntuple_values[leaf.GetName()] = leaf.GetValue()

            # Overwrite 'root_object' with the dictionary that contains the
            # values, because the values are what we want to save, and we
            # want to use the same RootObject()-call for both histograms and
            # n-tuples :-)
            root_object = ntuple_values

        key2object[name].append(
            RootObject(
                revision,
                package,
                root_file,
                name,
                root_object,
                root_object_type,
                dir_date,
                metadata["description"],
                metadata["check"],
                metadata["contact"],
                metadata["metaoptions"],
                is_reference,
            )
        )

    # Close the ROOT file before we open the next one!
    tfile.Close()

    return key2object


##############################################################################
#                      Main function starts here!                            #
##############################################################################


def create_plots(
    revisions=None,
    force=False,
    process_queue: Optional[Queue] = None,
    work_folder=".",
):
    """!
    This function generates the plots and html
    page for the requested revisions.
    By default all available revisions are taken. New plots will ony be
    created if they don't exist already for the given set of revisions,
    unless the force option is used.
    @param revisions: The revisions which should be taken into account.
    @param force: If True, plots are created even if there already is a version
        of them (which may me deprecated, though)
    @param process_queue: communication Queue object, which is used in
           multi-processing mode to report the progress of the plot creating.
    @param work_folder: The work folder
    """

    # Initialize the list of revisions which we will plot
    if not revisions:
        revisions = []

    # Loop over all revisions given on the command line
    for revision in revisions:
        # If it is a valid (i.e. available) revision, append it to the list
        # of revisions that we will include in our plots
        # 'reference' needs to be treated
        # separately, because it is always a viable option, but will never
        # be listed in 'available_revisions()'
        if (
            revision not in available_revisions(work_folder)
            and not revision == "reference"
        ):
            print(f"Warning: Removing invalid revision '{revision}'.")
            revisions.pop(revision)

    # In case no valid revisions were given, fall back to default and use all
    # available revisions and reference. The order should now be [reference,
    # newest_revision, ..., oldest_revision].
    if not revisions:
        revisions = ["reference"] + available_revisions(work_folder)

    # Now we check whether the plots for the selected revisions have been
    # generated before or not. In the path we use the alphabetical order of the
    # revisions, not the chronological one
    # (easier to work with on the web server side)
    expected_path = validationpath.get_html_plots_tag_comparison_json(
        work_folder, revisions
    )

    # If the path exists and we don't want to force the regeneration of plots,
    # serve what's in the archive
    if os.path.exists(expected_path) and not force:
        print(
            "Plots for the revision(s) {} have already been created before "
            "and will be served from the archive.".format(", ".join(revisions))
        )
    # Otherwise: Create the requested plots
    else:
        generate_new_plots(revisions, work_folder, process_queue)

    # signal the main process that the plot creation is complete
    if process_queue:
        process_queue.put({"status": "complete"})
        process_queue.close()
