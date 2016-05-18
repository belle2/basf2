#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Normal imports
import argparse
import glob
import datetime
import glob
import math
import os
import pickle
import re
import shutil
import sys
import time
import numbers
import queue
# Load ROOT
import ROOT
# The pretty printer. Print prettier :)
import pprint
import time
import json
import json_objects

import validationcomparison
from validationplotuple import Plotuple
import metaoptions
from validationfunctions import strip_ext, index_from_revision, get_style, available_revisions

try:
    import simplejson as json
except ImportError:
    import json


# Only execute the program if a basf2 release is set up!
if os.environ.get('BELLE2_RELEASE', None) is None:
    sys.exit('Error: No basf2 release set up!')

pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)

# Token used to separate the package name from the plot name in strings
PackageSeperatorToken = "__:__"

##############################################################################
#                          Function definitions                              #
##############################################################################


def largest_name(name):
    nameArr = []
    for plts in os.listdir(name):
        plts = plts.strip()
        if plts.endswith(".png"):
            nameArr.append(len(plts[:-4]))
    return max(nameArr)


def nPvLT(pValDict, number):
    for pval in pValDict.values():
        if isinstance(pval, numbers.Number) and pval <= number:
            return 0
    return 1


def date_from_revision(revision):
    """
    Takes the name of a revision and returns the 'last modified'-timestamp of
    the corresponding directory, which holds the revision.
    :param revision: A string containing the name of a revision
    :return: The 'last modified'-timestamp of the folder which holds the
        revision
    """

    # Reference files do not need a date since there is always just one
    # version of it, which is presumed to be the latest
    if revision == 'reference':
        return 0
    # Regular releases and builds however do have a reasonably well defined
    # 'last modified'-date!
    else:
        # If the revision exists:
        if revision in os.listdir('./results'):
            return os.path.getmtime('./results/' + revision)
        # Otherwise return a None object
        else:
            return None


def find_root_object(list_of_root_objects, **kwargs):
    """
    Receives a list of RootObject objects and a filter KEYWORD=['accepted,
    values'] and return the sublist that matches this filter
    """
    if kwargs is not None:
        # Read in the filter and the values we are filtering for
        sieve, desired_values = list(kwargs.items())[0]

        # If we don't receive a list of desired values, make it a list!
        if not isinstance(desired_values, list):
            desired_values = [desired_values]

        # Holds the lists of matches we have found
        results = []

        for rootobject in list_of_root_objects:
            for value in desired_values:
                try:
                    __ = re.search("^" + value + "$", rootobject.data[sieve])
                    if __ is not None:
                        results.append(rootobject)
                except IndexError:
                    continue
        return results
    # If no filer is given there will be no output
    else:
        return []


def serve_existing_plots(list_of_revisions):
    """
    Goes to the folder where
    the plots for the given selection are stored, and replaces the current
    './content.html' with the one from the folder with the plots.
    :return: No return value
    """

    print("File exists already and will be served from archive!")


def files_in_pkg(pkg, list_of_revisions):
    """
    For a given package, and a list of revisions, returns a list of all plot
    and reference files in this package. Gives only the file names!
    """

    # Remove all files that do not belong to the given package
    # Also, only remember the name of the file, not the full path
    in_pkg = []
    for rootfile in get_plot_files(list_of_revisions):
        head, tail = os.path.split(rootfile)
        if head.endswith(pkg):
            in_pkg.append(tail)

    # Treat reference files specially, because they are located as
    # <pkgname>/validation/<filename>.root
    for rootfile in get_reference_files():
        head, tail = os.path.split(rootfile)
        if head.endswith(pkg + "/validation"):
            in_pkg.append(tail)

    return sorted(in_pkg)


def get_plot_files(list_of_revisions):
    """
    Returns a list of all plot files as absolute paths. For this purpose,
    it loops over all revisions in 'list_of_revisions', finds the
    corresponding results folder and collects the plot ROOT files.
    :rtype: list
    :return: A list of all plot files, i.e. plot ROOT files from the
             requested revisions
    """
    # This is where we store the paths of plot ROOT files we've found
    results = []

    results_foldername = "./results"

    # Loop over all requested revisions and look for root files
    # in their package folders
    for revision in list_of_revisions:

        rev_result_folder = os.path.join(results_foldername, revision)
        if not os.path.isdir(rev_result_folder):
            continue

        packages = os.listdir(rev_result_folder)

        for p in packages:
            package_folder = os.path.join(rev_result_folder, p)
            # find all root files within this package
            root_files = glob.glob(package_folder + "/*.root")
            # append with absolute path
            results = results + [os.path.abspath(rf) for rf in root_files]

    return results


def get_reference_files():
    """
    This function loops over the local and central release dir and collects
    the .root-files from the validation-subfolders of the packages. These are
    the files which we will use as references.
    From the central release directory, we collect the files from the release
    which is set up on the machine running this script.
    :rtype : list
    :return: A list of all reference files, i.e. ROOT files that are located
             in the same folder as the steering files of the package
    """

    # The base paths to the local and central release directories
    basepaths = {'local': os.environ.get('BELLE2_LOCAL_DIR', None),
                 'central': os.environ.get('BELLE2_RELEASE_DIR', None)}

    # This is where we store the paths of reference ROOT files we've found
    results = {'local': [], 'central': []}

    # validation folder name used by the packages to keep the validation reference
    # plots
    validation_folder_name = 'validation'
    validation_test_folder_name = 'validation-test'

    # Now collect both local and central ROOT files:
    for location in ['local', 'central']:

        # Skip folders that do not exist (e.g. the central release dir might
        # not be setup if one is working with a completely local version)
        if basepaths[location] is None:
            continue

        # list all available packages
        root = basepaths[location]
        # searches for a validation folder in any top-most folder (package folders) and
        # lists all root-files within
        glob_search = os.path.join(root, "*", validation_folder_name, "*.root")
        revision_root_files = [os.path.abspath(f) for f in glob.glob(glob_search) if os.path.isfile(f)]
        # also look in the folder containing the validation tests
        glob_search = os.path.join(root, "*", validation_test_folder_name, "*.root")
        revision_root_files += [os.path.abspath(f) for f in glob.glob(glob_search) if os.path.isfile(f)]

        # this looks very much like a root file, store
        results[location] += revision_root_files

    # Now we need to get a rid of all the duplicates: Since local > central,
    # we will delete all central reference files that have a local counterpart.
    # First, loop over all local reference files
    for local_file in results['local']:
        # Remove the location, i.e. reduce the path to /[package]/[filename]
        local_path = local_file.replace(basepaths['local'], '')
        # Now loop over all central reference files
        for central_file in results['central']:
            # Remove the location, i.e.
            # reduce the path to /[package]/[filename]
            central_path = central_file.replace(basepaths['central'], '')
            # If package and filename are the same, we remove the central
            # file from our results list
            if local_path == central_path:
                results['central'].remove(central_file)

    # Return both local and central reference files. The return value does
    # not maintain the distinction between local and central files, because
    # we stored the absolute path to the reference files, and local and
    # central reference files are treated the same anyway.
    return results['local'] + results['central']


def plot_matrix(list_of_plotuples, package, list_of_revisions, *args):
    """!
    This function generates the plot matrix by searching png pictures in
    package folder and appending a image code into a html file.
    """

    # define where we have the plots
    pre = '_'.join(sorted(list_of_revisions)) + "/" + package
    folder = './plots/{0}/'.format(pre)
    # we can pass a subdirectory to this function as an additional argument
    # if we do pass an additional argument, then expand our folder path
    pth = ""
    if args:
        for arg in args:
            if isinstance(arg, str):
                if arg.endswith("/"):
                    arg = arg[:-1]
                if arg.startswith("/"):
                    arg = arg[1:]
                folder += "/" + arg
                pth += "/" + arg
            else:
                sys.exit("Error: function plot_matrix() takes subdirectory"
                         "names only as strings")
        pth += "/"
    # let the user know that where we are
    print("Plot matrix creation from images from " + folder)

    # Get the file the Plotuples are from. Beware, this is a filthy hack and
    # the entire function should probably be redone!
    if list_of_plotuples:
        rootfile = strip_ext(list_of_plotuples[0].rootfile)
    else:
        return ['<strong>List of Plottuples was empty!</strong><br><br>']

    # get description and p_value for every plot, we actually don't need
    # the list, we are working with descriptions, pvalues and keys only
    # it is better to search the existing images than loop over plots
    desc = {}
    p_value = {}
    is_expert = {}
    for item in list_of_plotuples:
        mod_key = '{0}_{1}'.format(strip_ext(item.rootfile), item.key)
        desc[mod_key] = item.description
        p_value[mod_key] = item.pvalue
        is_expert[mod_key] = "expert" in item.metaoptions

    ident = []
    html = []
    i = 1
    # if there is a folder and this folder contains plots
    if os.path.isdir(folder) and len(glob.glob(folder + "/*.png")) >= 1:

        if (nPvLT(p_value, 0.01) and nPvLT(p_value, 1)):
            html.append("<div class=\"fltr p_value_leq_0_01 "
                        "hlaska\" style=\"display:none\">"
                        "<strong>No plots with p-value "
                        "less than 0.01</strong></div>\n")
            html.append("<div "
                        "class=\"fltr p_value_leq_1 hlaska\" "
                        "style=\"display:none\">"
                        "<strong>No plots with p-value less "
                        "than 1</strong></div>\n")
        elif nPvLT(p_value, 0.01):
            html.append("<div class=\"fltr p_value_leq_0_01"
                        " hlaska\" "
                        "style=\"display:none\">"
                        "<strong>No plots with p-value less "
                        "than 0.01</strong></div>\n")
        elif nPvLT(p_value, 1):
            html.append("<div class=\"fltr p_value_leq_1 "
                        "hlaska\" style=\"display:none\">"
                        "<strong>No plots with p-value "
                        "less than 1</strong></div>\n")

        html.append('<div class="plot_matrix">\n\n<div class="object">\n')
        # the slider
        html.append('\n<table width="100%" '
                    'style="border:none">\n<tr>\n'
                    '<td width="50%" '
                    'align="left">\nChange image size: \n</td>\n'
                    '<td width="50%" align="right"'
                    '>\n<input width=\"25%\" '
                    'type=\"range\" id=\"{0}\" '
                    'class=\"plot-size\" min=\"10\" '
                    'max=\"50\" value=\"25\">&nbsp;'
                    '<span id="slidernumber_{0}">25</span>'
                    '%&nbsp;&nbsp;&nbsp;\n</td>\n</tr>\n'
                    '</table>\n<p></p>\n'.format(rootfile))

        # creation of the actual plot matrix
        for plts in sorted(os.listdir(folder)):
            plts = plts.strip()
            if plts.endswith(".png") and plts[:-4] in desc:
                # first, print that we are attaching an image to the grid
                print("Adding {0} to the plot matrix".format(plts[:-4]))
                # set our image path
                img = './plots/{0}/{1}/{2}'.format('_'.join(sorted(
                    list_of_revisions)), package, plts)
                # get the plot key from the file path
                plts = plts[:-4].strip()
                # if there is no description, say it
                if not desc[plts]:
                    desc[plts] = "n/a"
                # setting up our filters
                classes = ["imidzes_" + rootfile, "fltr"]
                if isinstance(p_value[plts], numbers.Number):
                    if p_value[plts] <= 1:
                        classes.append('p_value_leq_1')
                    if p_value[plts] <= 0.01:
                        classes.append('p_value_leq_0_01')
                if is_expert[plts]:
                    classes.append('expert_plot')
                # the code for the actual plots
                pre = (package + "_" + plts.replace(".", "_")).strip()
                pre1 = (package + "__" + plts.replace(".", "_")).strip()
                html.append('<span id="{6}" class="imidz-{5} '
                            'imidz"><a href="#{0}"> '
                            '<img width="25%" src="{1}" '
                            'class="matrix-img {4}" '
                            'align="left" alt="{2}" '
                            'title="{2}, {3}"> '
                            '</a></span>'
                            '\n'.format(pre, img, plts, desc[plts],
                                        " ".join(classes),
                                        rootfile,
                                        pre1))
                # save which plots we are using, this string will
                # be then pasted into id="" html tag
                ident.append(package + PackageSeperatorToken + plts)

        # lots of spaghetti-looking code that creates the hide/show button,
        # the information at the beginning and the select all/none button.
        pre = ('_'.join(sorted(list_of_revisions)) + "/" + package)
        html.append("</div>\n\n<div class='wrap_boxes'>\n\n"
                    "<div style=\"cursor:pointer;"
                    "font-size: 70%; text-align:right; width:100%\" "
                    "class=\"hide hidetext\" id=\"h{0}\">(show)</div>"
                    "<span class='wrap_h{0} hideme'><h2>Plots in this "
                    "package</h2> \nUncheck the checkbox next to "
                    "the plot name to hide the plot from the plot "
                    "matrix and click on the plot name or on the "
                    "plot miniature on the plot matrix to show a "
                    "bigger plot with full description.<br>\n<br>\n "
                    "<form method=\"post\" name=\"{1}\" "
                    "action=\"./matrix_save\"><span style=\"color:"
                    " #348017; text-decoration:underline;\"><input "
                    "type=\"checkbox\" class=\"selectall\"> Check all"
                    "</span><br><input type=\"hidden\" name=\"package\""
                    " value=\"{1}\"><table style='width:100%; "
                    "border:none'>\n<input type=\"hidden\" name=\"size\" "
                    "class=\"slidernumber_{0}\" value=\"25\">"
                    "\n<tr>".format(rootfile, pre))
        # this is here for aesthetics. If we have the largest
        # name of a plot with a length up to
        # 5 letters, we can fit 5 of these names into one line of text
        # so we create 5xN table
        if largest_name(folder) <= 5:
            k = 5
        # if the largest name is up to 15 letters long, we can
        # fit only 3 of these names into one line of text
        elif largest_name(folder) <= 15:
            k = 3
        # if 25, we can fit only two of these names
        elif largest_name(folder) <= 25:
            k = 2
        # and if it is longer, we can fit only one.
        # I don't suppose somebody will
        # be in need of so long name that we would have to wrap it
        else:
            k = 1
        # and now we create the table
        for el in sorted(ident):
            # split the package from the plot key
            plts = el.split(PackageSeperatorToken)[1]
            # prepare the ends of lines
            if (i % k) == 0:
                end = "\n</tr>\n<tr>"
            else:
                end = ""
            i += 1
            # also we can't forget our classes,
            # we don't want a name without a plot to be shown
            classes = ["fltr", "matrix-img"]
            if isinstance(p_value[plts], numbers.Number):
                if p_value[plts] <= 1:
                    classes.append('p_value_leq_1')
                if p_value[plts] <= 0.01:
                    classes.append('p_value_leq_0_01')
            # the code for the actual name table
            pre = "_".join(el.split(PackageSeperatorToken)).replace(".", "_")
            html.append("<td style='width:{5}%' class='{4}'>"
                        "<input type='checkbox' name='matrix' "
                        "class='matrix-toggle matrix-{6}' "
                        "value='{0}' checked><a href='#{1}' "
                        "class=\"imidz\" name=\"{0}\">{2}</a>"
                        "\n</td>{3}\n".format(el.strip(),
                                              pre, plts, end,
                                              " ".join(classes),
                                              100.0 / k, rootfile))
        html.append("</tr>\n</table>\n")
        # the save button
        html.append("\n<div width=\"100%\" align=\"right\" "
                    "title='Option PNG: "
                    "Saves the matrix exactly as you see in bigger "
                    "resolution as .png image.\nOption PDF: Creates an "
                    ".pdf file out of selected plots shown in the "
                    "matrix. Will not be formatted, each plot is a new "
                    "page of the .pdf file, plots will be placed in "
                    "order one after another.'>\n<select name='type'>"
                    "<option value='png'>Save the matrix as PNG image"
                    "</option><option value='pdf'>Merge plots "
                    "into a single PDF file</option></select>&nbsp;"
                    "<input type='submit' value='Save'>\n</div>\n</form>\n"
                    "</span>\n</div>\n\n</div>\n\n")
    # if we don't have the folder for this package,
    # say we don't have anything to display
    elif not os.path.isdir(folder):
        html.append("\n<strong>Nothing to display</strong>"
                    "\n<br>\n")
    # and if we don't have any plots, then
    # in overview mode(hence the <span>),
    # where we show only plots, write that we
    # don't have any plots in this package
    elif os.path.isdir(folder) and not len(glob.glob(folder + "/*.png")):
        html.append("\n<span class='noplots'><strong"
                    ">This section does not contain any plots."
                    "</strong></span>")
    print("Plot matrix successfully created.")
    return html


def generate_new_plots(list_of_revisions, process_queue=None):
    """
    Creates the plots that
    contain the requested revisions. Each plot (or n-tuple, for that matter)
    is stored in an object of class Plot.
    @param process_queue: communication queue object, which is used in
           multi-processing mode to report the progress of the plot creating.
    :return: No return value
    """

    # Since we are going to plot, we need to initialize ROOT
    ROOT.gROOT.SetBatch()
    ROOT.gStyle.SetOptStat(1110)
    ROOT.gStyle.SetOptFit(101)

    # Before we can start plotting, we of course need to collect all
    # ROOT-files that contain data for the plot that we want, e.g. we need to
    # collect all plot ROOT files from the revisions in 'list_of_revisions'.
    # The 'reference'-plots, if requested, require special treatment, as they
    # are stored on a different location than the regular plot ROOT files.

    # Collect all plot files, i.e. plot ROOT files from the requested revisions
    list_of_plot_files = get_plot_files(list_of_revisions)

    # If we also want a reference plot, collect the reference ROOT files
    if 'reference' in list_of_revisions:
        list_of_reference_files = get_reference_files()
    else:
        list_of_reference_files = []

    # Now create the ROOT objects for the plot and the reference objects,
    # and get the lists of keys and packages
    plot_objects, plot_keys, plot_packages = create_RootObjects_from_list(list_of_plot_files, False)
    reference_objects, reference_keys, reference_packages = create_RootObjects_from_list(list_of_reference_files, True)

    # Get the joint lists (and remove duplicates if applicable)
    list_of_root_objects = plot_objects + reference_objects
    list_of_keys = sorted(list(set(plot_keys)))
    list_of_packages = sorted(list(set(plot_packages)))

    # Now create the HTML files that contain the lists of all available
    # revisions and the available packages
    create_revision_list_html()

    create_packages_list_html(list_of_packages, list_of_revisions)

    # Open the output file
    # First: Create destination directory if it does not yet exist
    content_dir = './plots/{0}'.format('_'.join(sorted(
        list_of_revisions)))
    if not os.path.exists(content_dir):
        os.makedirs(content_dir)

    # Now open the file to which we will write all the content HTML
    html_output = open('{0}/content.html'.format(content_dir), 'w+')

    # Write meta-data, i.e. creation date and revisions contained in the file
    created = str(datetime.datetime.utcfromtimestamp(int(time.time())))
    json_revs = ','.join(['"' + _ + '"' for _ in list_of_revisions])
    html_output.write('<!-- {{ "lastModified": '
                      '"Plots created:<br>{0} (UTC)", '
                      '"revisions":[{1}] }} -->'
                      '\n\n'.format(created, json_revs))

    comparison_packages = []

    # for every package
    i = 0
    for package in sorted(list_of_packages):
        i = i + 1

        # Some information to be printed out while the plots are created
        print('Now creating plots for package: {0}'.format(package))

        html_output.write('<div id="' + package + '">\n')
        html_output.write('<h1>' + package + '</h1>\n')

        # A list of all objects (including reference objects) that
        # belong to the current package
        objects_in_pkg = find_root_object(list_of_root_objects,
                                          package=package)

        # Find all ROOT files that were created in the scope of this
        # package (also including reference files)
        files_in_pkg = []
        for plot_object in objects_in_pkg:
            # For every object in the package, get the corresponding
            # file name and append it to the list if its not in there
            rootfile_name = os.path.basename(plot_object.rootfile)
            if rootfile_name not in files_in_pkg:
                files_in_pkg.append(rootfile_name)
        files_in_pkg.sort()

        compare_files = []

        # Now we loop over all files that belong to the package to
        # group the plots correctly
        for rootfile in files_in_pkg:
            # todo: remove, only for debugging
            # time.sleep(2.5)
            fileName, fileExt = os.path.splitext(rootfile)

            # Some more information to be printed out while plots are
            # being created
            print('\nNow creating plots for file: {0}'.format(rootfile))

            # Start the section for the fixed/floating headers
            html_output.write('<section class="persistent-area">')

            # Write to the HTML the name of the current file
            html_output.write('<h2 id="{0}_{1}" class="persistent-header">{1}</h2><br>\n'
                              .format(package, fileName))

            # Get the list of all objects that belong to the current
            # package and the current file. First the regular objects:
            objects_in_pkg_and_file = find_root_object(objects_in_pkg,
                                                       rootfile=".*/" + rootfile + ".*")

            # And then the reference objects
            objects_in_pkg_and_file += find_root_object(reference_objects,
                                                        rootfile=".*/" + rootfile + ".*")

            # A list in which we keep all the plotuples for this file
            list_of_plotuples = []

            # report the progress over the queue object, if available
            if process_queue:
                try:
                    process_queue.put_nowait({"current_package": i, "total_package": len(list_of_packages),
                                              "package_name": package, "file_name": fileName})
                except queue.Full:
                    # message could not be placed, but no problem next message will maybe work
                    pass

            # Now loop over ALL keys (within a file, objects will be
            # sorted by key)
            i_key = 0
            compare_plots = []
            compare_ntuples = []
            for key in sorted(list_of_keys):
                i_key = i_key + 1

                # Find all objects for the Plotuple that is defined by the
                # package, the file and the key
                root_objects = find_root_object(objects_in_pkg_and_file,
                                                key=key)

                # If this list is empty, we can continue right away
                if not root_objects:
                    continue

                # Otherwise we can generate Plotuple object
                plotuple = Plotuple(root_objects, list_of_revisions)
                list_of_plotuples.append(plotuple)

                # Write the HTML for the Plotuple to the file stream
                html_output.write('\n\n')
                html_output.write('\n'.join(plotuple.html()))

                if (plotuple.type == 'TNtuple'):
                    compare_ntuples.append(json_objects.ComparisonNTuple(title=plotuple.get_plot_title(),
                                                                         description=plotuple.description,
                                                                         contact=plotuple.contact,
                                                                         check=plotuple.check,
                                                                         is_expert=plotuple.is_expert(),
                                                                         json_file_path=plotuple.file_json))
                else:
                    compare_plots.append(json_objects.ComparisonPlot(title=plotuple.get_plot_title(),
                                                                     comparison_result=plotuple.comparison_result,
                                                                     comparison_text=plotuple.chi2test_result,
                                                                     comparison_pvalue=plotuple.pvalue,
                                                                     comparison_pvalue_warn=plotuple.pvalue_warn,
                                                                     comparison_pvalue_error=plotuple.pvalue_error,
                                                                     description=plotuple.description,
                                                                     contact=plotuple.contact,
                                                                     check=plotuple.check,
                                                                     height=plotuple.height,
                                                                     width=plotuple.width,
                                                                     is_expert=plotuple.is_expert(),
                                                                     plot_path=plotuple.get_plot_path(),
                                                                     png_filename=plotuple.get_png_filename(),
                                                                     pdf_filename=plotuple.get_pdf_filename()))

            compare_file = json_objects.ComparisonPlotFile(title=fileName,
                                                           package=package,
                                                           rootfile=fileName,
                                                           compared_revisions=list_of_revisions,
                                                           plots=compare_plots,
                                                           ntuples=compare_ntuples)
            compare_files.append(compare_file)

            # Now generate the matrix view with all plots for this ROOT file
            plot_matrix_html = plot_matrix(list_of_plotuples, package, list_of_revisions)
            html_output.write('\n'.join(plot_matrix_html))

            # Close the section for the header
            html_output.write('</section>')

        comparison_packages.append(json_objects.ComparisonPackage(name=package, plotfiles=compare_files))
        # Make the command line output more readable
        print(2 * '\n')

        # Close the div for the package
        html_output.write('</div>\n\n')

    print("storing to " + '{0}/comparison.json'.format(content_dir))

    # create objects for all revisions
    comparison_revs = []

    for r in list_of_revisions:
        index = index_from_revision(r)

        # revision has black by default
        line_color = "#000000"
        if index is not None:
            style = get_style(index)
            line_color = ROOT.gROOT.GetColor(style.GetLineColor()).AsHexString()
        print("For {} index {} color {}".format(r, index, line_color))
        comparison_revs.append(json_objects.ComparisonRevision(label=r, creation_date=datetime.datetime.now(), color=line_color))

    # todo: refactor this information extracion -> json inside a specific class / method after the
    # plots have been created
    json_objects.dump(os.path.join(content_dir, "comparison.json"), json_objects.Comparison(comparison_revs, comparison_packages))

    # Add JavaScript for the fixed headers
    # Admittedly, this is an ugly workaround...
    html_output.write('<script type="text/javascript">\n'
                      'var clonedHeaderRow;\n'
                      '$(".persistent-area").each(function() {\n'
                      'clonedHeaderRow = $(".persistent-header", this);\n'
                      'clonedHeaderRow\n'
                      '.before(clonedHeaderRow.clone())\n'
                      '.css("width", clonedHeaderRow.width())\n'
                      '.addClass("floatingHeader");\n'
                      '});\n'
                      '</script>\n')

    # Close the file stream for the HTML file
    html_output.close()


def create_RootObjects_from_list(list_of_root_files, is_reference):
    """
    Takes a list of root files, loops over them and creates the RootObjects
    for it. It then returns the list of RootObjects, a list of all keys,
    and a list of all packages for those objects.
    :param list_of_root_files: The list of all *.root files which shall be
        read in and for which the corresponding RootObjects shall be created
    :param is_reference: Boolean value indicating if the objects are
        reference objects or not.
    :return: List RootObjects, List of Keys in said RootObjects, List of
        Packages in said RootObjects
    """

    # Reserve some space for the results that will be returned by this
    # function
    list_objects = []
    list_keys = []
    list_packages = []

    # Now loop over all given
    for root_file in list_of_root_files:

        # Create the RootObjects from this file and store them, as well as the
        file_objects, \
            file_keys, \
            file_package = create_RootObjects_from_file(root_file,
                                                        is_reference)

        # Append results to the global results
        list_objects += file_objects
        list_keys += file_keys
        list_packages.append(file_package)

    # Remove possible duplicates from the lists
    list_keys = sorted(list(set(list_keys)))
    list_packages = sorted(list(set(list_packages)))

    return list_objects, list_keys, list_packages


def create_RootObjects_from_file(root_file, is_reference):
    """
    Takes a root file, loops over its contents and creates the RootObjects
    for it. It then returns the list of RootObjects, a list of all keys,
    and a list of all packages for those objects.
    :param root_file: The *.root files which shall be read in and for which the
        corresponding RootObjects shall be created
    :param is_reference: Boolean value indicating if the object is a
        reference object or not.
    :return: List RootObjects, List of Keys in said RootObjects, and the
        Packages of said RootObjects
    """

    # Reserve some space for the results that will be returned by this
    # function
    file_objects = []
    file_keys = []

    # Retrieve the Revision and the Package from the path. The Package can
    # directly be returned (c.f. return at the bottom of this function)
    if is_reference:
        revision = 'reference'
        package = root_file.split('/')[-3]
    else:
        revision = root_file.split('/')[-3]
        package = root_file.split('/')[-2]

    # Get the 'last modified' timestamp of the revision that contains our
    # current root_file
    dir_date = date_from_revision(revision)

    # Open the file with ROOT
    ROOT_Tfile = ROOT.TFile(root_file)

    # Loop over all Keys in that ROOT-File
    for key in ROOT_Tfile.GetListOfKeys():

        # Get the name of the Key and save it in file_keys
        name = key.GetName()
        file_keys.append(name)

        # temporary workaround for dbstore files located (wrongly)
        # in the validation results folder
        if re.search(".*dbstore.*root", root_file):
            continue

        # Get the ROOT object that belongs to that Key. If there is no
        # object, continue
        root_object = ROOT_Tfile.Get(name)
        if (not root_object) or (root_object is None):
            continue

        # Determine which type of object it is, i.e. TH1, TH2 or TNtuple
        if root_object.InheritsFrom('TNtuple'):
            root_object_type = 'TNtuple'
        # this will also match TProfile, as this root class derives from
        # TH1D
        elif root_object.InheritsFrom('TH1'):
            if root_object.InheritsFrom('TH2'):
                root_object_type = 'TH2'
            else:
                root_object_type = 'TH1'
        # TEfficiency barks and quarks like a TProfile, but is unfortunately not
        elif root_object.InheritsFrom('TEfficiency'):
            root_object_type = 'TEfficiency'
        elif root_object.InheritsFrom('TASImage'):
            root_object_type = 'TASImage'
        else:
            root_object_type = None

        # If we are dealing with a histogram:
        if root_object_type in ['TH1', 'TH2', 'TEfficiency']:

            # Ensure that the data read from the ROOT files lives on even
            # after the ROOT file is closed
            root_object.SetDirectory(0)

            # Read out meta information:
            # DescriptionDescription, Check and Contact
            # Initialize as None objects
            description = None
            check = None
            contact = None

            # Now check if the data exists in the ROOT file and if so, read it
            if root_object.FindObject('Description'):
                description = root_object.FindObject('Description').GetTitle()
            if root_object.FindObject('Check'):
                check = root_object.FindObject('Check').GetTitle()
            if root_object.FindObject('Contact'):
                contact = root_object.FindObject('Contact').GetTitle()

            # Empty fields are filled with 'n/a'
            for metadatum in [description, check, contact]:
                # .GetTitle() returns 'None', if there is no title
                if metadatum is None:
                    metadatum = 'n/a'

            # Now check for meta-options (colz, log-scale, etc.)
            metaoptions = []
            if root_object.FindObject('MetaOptions'):
                # Get the title. If there is no title, set metaoptions to an
                # empty list again. Otherwise parse the string of options into
                # a list of options (split on comma, remove whitespaces).
                metaoptions = root_object.FindObject('MetaOptions').GetTitle()
                if metaoptions is None:
                    metaoptions = []
                else:
                    metaoptions = [_.strip() for _ in metaoptions.split(',')]

        # If we are dealing with an n-tuple
        elif root_object_type == 'TNtuple':

            # Go to first entry in the n-tuple
            root_object.GetEntry(0)

            # Storage for the values of the n-tuple. We use a dictionary,
            # because we can't access the n-tuple's values anymore after
            # closing the ROOT file (<=> histograms)
            ntuple_values = {}
            for leaf in root_object.GetListOfLeaves():
                ntuple_values[leaf.GetName()] = leaf.GetValue()

            # Get description, check and contact
            description = root_object.GetAlias('Description')
            check = root_object.GetAlias('Check')
            contact = root_object.GetAlias('Contact')

            # Empty fields are filled with 'n/a'
            for metadatum in [description, check, contact]:
                # .GetAlias() returns '' (empty string), if there is no alias
                if metadatum == '':
                    metadatum = 'n/a'

            # Now check for meta-options (colz, log-scale, etc.)
            metaoptions = root_object.GetAlias('MetaOptions')
            if metaoptions:
                # If there are meta-options, split the string on commas and
                # remove unnecessary whitespaces
                metaoptions = [_.strip() for _ in metaoptions.split(',')]

            # Overwrite 'root_object' with the dictionary that contains the
            # values, because the values are what we want to save, and we
            # want to use the same RootObject()-call for both histograms and
            # n-tuples :-)
            root_object = ntuple_values

        elif root_object_type == 'TASImage':
            # TODO Set this to correct values
            description = None
            check = None
            contact = None
        # If it is neither an histogram nor an n-tuple, we skip it!
        else:
            continue

        # Create the RootObject and append it to the results
        file_objects.append(RootObject(revision, package, root_file,
                                       name, root_object,
                                       root_object_type, dir_date,
                                       description, check, contact,
                                       metaoptions, is_reference))

    # Close the ROOT file before we open the next one!
    ROOT_Tfile.Close()

    return file_objects, file_keys, package


def create_revision_list_html():
    """
    Creates a HTML-list of all available revisions with checkboxes and the
    font color set accordingly to the line style of the revision in the plots.
    """

    revisions = open('revisions.html', 'w+')
    revisions.write('<label><input type="checkbox" name="revisions" '
                    ' value="reference" checked>&nbsp;<span '
                    'style="color: black;">reference</span></label><br>\n')

    for item in available_revisions():
        ind = index_from_revision(item)
        color = ROOT.gROOT.GetColor(get_style(ind, len(available_revisions()))
                                    .GetLineColor()).AsHexString()
        date_of_revision = datetime.datetime.utcfromtimestamp(int(
            date_from_revision(item)))
        revisions.write('<label title="Data created: {0} (UTC)">'
                        '<input type="checkbox" name="revisions" value="{1}" '
                        'checked>&nbsp;'
                        '<span style="color: {2};">{1}</span></label><br>\n'
                        .format(date_of_revision, item, color))
    revisions.write('<div id="regenerate">Load selected</div>')
    revisions.close()


def create_packages_list_html(list_of_packages, list_of_revisions):

    # Create a file with all available packages
    # First: Create destination directory if it does not yet exist
    content_dir = './plots/{0}'.format('_'.join(sorted(
        list_of_revisions)))
    if not os.path.exists(content_dir):
        os.makedirs(content_dir)

    # Now open the file to which we will write list of packages
    with open('{0}/packages.html'.format(content_dir), 'w+') as pkgs:
        for pkg in sorted(list_of_packages):

            # Write the checkbox to the HTML
            pkgs.write('<label>'
                       '<input type="checkbox" name="packages" '
                       'value="{0}" checked>&nbsp;'
                       '<a href="#{0}" class="pkg_container_head" '
                       'name="{0}">{0} &raquo;</a><br>'
                       '</label>\n'
                       .format(pkg))

            # Get a list of all ROOT result files in this package
            in_pkg = sorted(list(set(files_in_pkg(pkg, list_of_revisions))))

            # Print them into a container
            pkgs.write('<div class="pkg_container pkg_{0}">'.format(pkg))

            for rootfile in in_pkg:

                # Get the file name and shorten it if necessary
                filename, extension = os.path.splitext(rootfile)
                if len(filename) > 20:
                    shortname = filename[:17] + '...'
                else:
                    shortname = filename

                # Write to HTML
                pkgs.write('<a href="#{0}_{1}" title="{2}">{3}</a><br>\n'
                           .format(pkg, filename, rootfile, shortname))

            pkgs.write('</div>')


##############################################################################
#                             Class Definition                               #
##############################################################################

class RootObject:

    """!
    Wraps a ROOT object (either a histogram or an n-tuple) together with the
    available meta-information about it.
    Storing the information in a dictionary is necessary to make the objects
    searchable, i.e. implement a function that can return for example all
    objects from a certain revision.

    @var data: A dict with all information about the Root-object
    @var revision: The revision to which the object belongs to
    @var package: The package to which the object belongs to
    @var rootfile: The root file to which the object belongs to
    @var key: The key (more precisely: the name of the key) which the object
        has within the root file
    @var object: The root object itself
    @var type: The type, i.e. whether its a histogram or an n-tuple
    @var description: The description, what the histogram/n-tuple contains
    @var check: A brief description how the histogram or the values should
        look like
    @var contact: A contact person for this histogram/n-tuple
    @var date: The date of the object (identical with the date of its rootfile)
    @var is_reference: Boolean value if it is an object from a reference file
        or not
    """

    def __init__(self, revision, package, rootfile, key, root_object,
                 root_object_type, date, description, check, contact,
                 metaoptions, is_reference):
        """!
        The constructor. Sets the element up and store the information in a
        dict, but also sets up object variables for simplified access.

        @param revision: The revision of the object, e.g. 'release-00-04-01'
        @param package: The package of the object, e.g. 'analysis'
        @param rootfile: The absolute path to the ROOT file that contains
                this object
        @param key: The key of the object, which is basically its name.
                Example: 'P_Eff_k_e'. For each revision, there should be one
                object with the same key.
        @param root_object: The ROOT object itself. Storing works only for
                histograms.
        @param root_object_type: The type of the object. Possible values are
                'TH1' (1D histogram), 'TH2' (2D histogram), and 'TNtuple'
        @param date: The date when the containing revision folder was last
                modified. Important to find the most recent object.
        @param description: A short description of what is displayed in the
                plot. May also contain LaTeX-Code (enclosed in $...$),
                which will later be parsed by MathJax
        @param check: A short description of how the data in the plot should
                look like, i.e. for example the target location of a peak etc.
        @param contact: A name or preferably an e-mail address of the person
                who is responsible for this plot and may be contacted in case
                of problems
        @param metaoptions: Meta-options for the plot, e.g. 'colz' for histo-
                grams, or log-scale for the axes, etc.
        @param is_reference: A boolean value telling if an object is a
                reference object or a normal plot/n-tuple object from a
                revision. Possible Values: True for reference objects,
                False for revision objects.
        """

        # TO DO
        # All of the following could be simplified, if one modified the
        # find_root_object() method to search through vars(Root-Object)

        # A dict with all information about the Root-object
        # Have all information as a dictionary so that we can search and
        # filter the objects by properties
        self.data = {'revision': revision,
                     'package': package,
                     'rootfile': rootfile,
                     'key': key,
                     'object': root_object,
                     'type': root_object_type,
                     'check': check,
                     'description': description,
                     'contact': contact,
                     'date': date,
                     'metaoptions': metaoptions,
                     'is_reference': is_reference}

        # For convenient access, define the following variables, which are
        # only references to the values from the dict

        # The revision to which the object belongs to
        self.revision = self.data['revision']

        # The package to which the object belongs to
        self.package = self.data['package']

        # The root file to which the object belongs to
        self.rootfile = self.data['rootfile']

        # The key (more precisely: the name of they) which the object has
        # within the root file
        self.key = self.data['key']

        # The root object itself
        self.object = self.data['object']

        # The type, i.e. whether its a histogram or an n-tuple
        self.type = self.data['type']

        # The description, what the histogram/n-tuple contains
        self.description = self.data['description']

        # A brief description how the histogram or the values should look
        # like (e.g. characteristic peaks etc.)
        self.check = self.data['check']

        # A contact person for this histogram/n-tuple
        self.contact = self.data['contact']

        # The date of the object (identical with the date of its rootfile)
        self.date = self.data['date']

        # Meta-options for the object, e.g. colz or log-scale for the axes
        self.metaoptions = self.data['metaoptions']

        # Boolean value if it is an object from a reference file or not
        self.is_reference = self.data['is_reference']

    def __str__(self):
        return str(self.data)

    def dump(self):
        """!
        Allows to print out all information about a RootObject to the command
        line (for debugging purposes).
        @return: None
        """
        pp.pprint(self.data)


##############################################################################
#                      Main function starts here!                            #
##############################################################################


def create_plots(revisions=None, force=False, process_queue=None):
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
    """

    # Initialize the list of revisions which we will plot
    list_of_revisions = []

    # Retrieve the desired revisions from the command line arguments and store
    # them in 'list_of_revisions'
    if revisions:
        # Loop over all revisions given on the command line
        for revision in revisions:
            # If it is a valid (i.e. available) revision, append it to the list
            # of revisions that we will include in our plots
            # 'reference' needs to be treated
            # separately, because it is always a viable option, but will never
            # be listed in 'available_revisions()'
            if revision in available_revisions() or revision == 'reference':
                list_of_revisions.append(revision)

    # In case no valid revisions were given, fall back to default and use all
    # available revisions and reference. The order should now be [reference,
    # newest_revision, ..., oldest_revision].
    if not list_of_revisions:
        list_of_revisions = ['reference'] + available_revisions()

    # Now we check whether the plots for the selected revisions have been
    # generated before or not. In the path we use the alphabetical order of the
    # revisions, not the chronological one
    # (easier to work with on the web server side)
    expected_path = './plots/{0}/content.html'.format(
        '_'.join(sorted(list_of_revisions)))

    # If the path exists and we don't want to force the regeneration of plots,
    # serve what's in the archive
    if os.path.exists(expected_path) and not force:
        serve_existing_plots(list_of_revisions)
        print('Served existing plots.')
    # Otherwise: Create the requested plots
    else:
        generate_new_plots(list_of_revisions, process_queue)

        # Check if we generated the default view (i.e. all available revisions
        # and the references). If this is the case, copy these files to the
        # base directory
        if (sorted(list_of_revisions) ==
                sorted(['reference'] + available_revisions())):

            # The path where the content.html and packages.html should be
            src = './plots/{0}/'.format('_'.join(sorted(list_of_revisions)))

            # The path where we need them
            dst = './'

            # Actually copy the files
            shutil.copyfile(src + 'content.html', dst + 'content.html')
            shutil.copyfile(src + 'packages.html', dst + 'packages.html')

            print('Copied generated content.html and packages.html to the '
                  'base directory! \n')

    # signal the main process that the plot creation is complete
    if process_queue:
        process_queue.put({"status": "complete"})
        process_queue.close()
