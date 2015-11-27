#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Normal imports
import argparse
import glob
import datetime
import fnmatch
import math
import os
import pickle
import re
import shutil
import sys
import time
import numbers
# Load ROOT
import ROOT
# The pretty printer. Print prettier :)
import pprint

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


def strip_ext(path):
    """
    Takes a path and returns only the name of the file, without the
    extension on the file name
    """
    return os.path.splitext(os.path.split(path)[1])[0]


def available_revisions():
    """
    Loops over the results folder and looks for revisions. It then returns an
    ordered list, with the most recent revision being the first element in the
    list and the oldest revision being the last element.
    The 'age' of a revision is determined by the 'Last-modified'-timestamp of
    the corresponding folder.
    :return: A list of all revisions available for plotting
    """

    # Get all folders in ./results/ sorted descending by the date they were
    # created (i.e. newest folder first)
    revisions = sorted(os.listdir('./results'),
                       key=lambda _: os.path.getmtime('./results/' + _),
                       reverse=True)
    # Return it
    return revisions


def index_from_revision(revision):
    """
    Takes the name of a revision and returns the corresponding index. Indices
    are used to ensure that the color and style of a revision in a plot are
    always the same, regardless of the displayed revisions.
    Example: release-X is always red, and no other release get drawn in red if
    release-X is not selected for display.
    :param revision: A string containing the name of a revision
    :return: The index of the requested revision, or None, if no index could
        be found for 'revision'
    """

    # If the requested revision exists, return its index
    if revision in available_revisions():
        index = available_revisions().index(revision)
    # Else return a None object
    else:
        index = None

    return index


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

    """
    # The path where the content.html should be
    src = './plots/{0}/content.html'.\
        format('_'.join(sorted(list_of_revisions)))

    # The path where we need it
    dst = './content.html'

    # Make sure the file exists before we copy it
    if os.path.isfile(src):
        shutil.copyfile(src, dst)
    else:
        sys.exit('Wanted to use plots from the archive, but the corresponding '
                 'content.html file could not be found!')
    """


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

    # Loop over all requested revisions
    for revision in list_of_revisions:
        # Loop through the results-folder
        for root, dirs, files in os.walk('./results'):
            # Loop over all files in the subfolders of the results-folder
            for current_file in files:
                # If the files is a ROOT file in a subfolder of a revision,
                # collect it (subfolders are package names, i.e. we expect
                # the plot files for the analysis package in
                # 'revision/analysis/some_plot_file.root'
                if fnmatch.fnmatch(root + '/' + current_file,
                                   '*/' + revision + '/*/*.root'):
                    results.append(os.path.abspath(root + '/' + current_file))

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

    # Now collect both local and central ROOT files:
    for location in ['local', 'central']:

        # Skip folders that do not exist (e.g. the central release dir might
        # not be setup if one is working with a completely local version)
        if basepaths[location] is None:
            continue

        # Get the files from the corresponding directory (either the local or
        # the central release directory)
        for root, dirs, files in os.walk(basepaths[location]):
            # Loop over all these files
            for current_file in files:
                # Construct the full path = base path+filename
                full_path = root + '/' + current_file
                # If the file is a *.root file in a validation subfolder,
                # we shall collect it as a reference file
                # NB: This will only find ROOT files that lie directly in
                # the /[pkg]/validation folder, not in any subdirs of it!
                if (os.path.splitext(full_path)[1] == '.root' and
                        os.path.dirname(full_path).endswith('validation')):
                    results[location].append(os.path.abspath(full_path))

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

            print("el" + str(el))

            plts = el.split(PackageSeperatorToken)[1]
            print("plts" + str(plts))
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


def generate_new_plots(list_of_revisions, process_pipe=None):
    """
    Creates the plots that
    contain the requested revisions. Each plot (or n-tuple, for that matter)
    is stored in an object of class Plot.
    @param process_pipe: communication Pipe object, which is used in
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

        print('Files in pkg:', files_in_pkg)

        # Now we loop over all files that belong to the package to
        # group the plots correctly
        for rootfile in files_in_pkg:

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

            # Now loop over ALL keys (within a file, objects will be
            # sorted by key)
            i_key = 0
            for key in sorted(list_of_keys):
                i_key = i_key + 1

                # report the progress over the Pipe object, if available
                if process_pipe:
                    process_pipe.send({"current_package": i, "total_package": len(list_of_packages),
                                       "package_name": package, "file_name": fileName,
                                       "current_item": i_key, "total_item": len(list_of_keys)})

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

            # Now generate the matrix view with all plots for this ROOT file
            plot_matrix_html = plot_matrix(list_of_plotuples, package, list_of_revisions)
            html_output.write('\n'.join(plot_matrix_html))

            # Close the section for the header
            html_output.write('</section>')

        # Make the command line output more readable
        print(2 * '\n')

        # Close the div for the package
        html_output.write('</div>\n\n')

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
        elif root_object.InheritsFrom('TASImage'):
            root_object_type = 'TASImage'
        else:
            root_object_type = None

        # If we are dealing with a histogram:
        if root_object_type in ['TH1', 'TH2']:

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


def get_style(index, overallItemCount):
    """
    Takes an index and returns the corresponding line attributes,
    i.e. LineColor, LineWidth and LineStyle.
    """

    # Define the colors for the plot
    colors = [ROOT.kRed,
              ROOT.kOrange,
              ROOT.kPink + 9,
              ROOT.kOrange - 8,
              ROOT.kGreen + 1,
              ROOT.kCyan + 2,
              ROOT.kBlue + 1,
              ROOT.kRed + 2,
              ROOT.kOrange + 3,
              ROOT.kYellow + 2,
              ROOT.kYellow + 4]

    # Define the linestyles for the plot
    linestyles = {'dashed': 2,    # Dashed: - - - - -
                  'solid': 1,    # Solid: ----------
                  'dashdot': 10}    # Dash-dot: -?-?-?-
    ls_index = {0: 'dashed', 1: 'solid', 2: 'dashdot'}

    # Define the linewidth for the plots
    linewidth = 2

    # make sure the index is set
    if not index:
        index = 0

    # Get the color for the (index)th revisions
    color = colors[index % len(colors)]

    # Figure out the linestyle
    # If there is only one revision, make it solid!
    # It cannot overlap with any other line
    if overallItemCount == 1:
        linestyle = linestyles['solid']
    # Otherwise make sure the newest revision (which is drawn on top) gets a
    # dashed linestyle
    else:
        linestyle = linestyles[ls_index[index % len(ls_index)]]

    return ROOT.TAttLine(color, linestyle, linewidth)


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

    def dump(self):
        """!
        Allows to print out all information about a RootObject to the command
        line (for debugging purposes).
        @return: None
        """
        pp.pprint(self.data)


class Plotuple:

    """!
    A Plotuple is either a Plot or an N-Tuple

    @var list_of_root_objects: A list of Root-objects which belong
        together (i.e. should be drawn into one histogram or one table)
    @var list_of_revisions: The list of revisions
    @var warnings: A list of warnings that occured while creating the
        plots/tables for this Plotuple object
    @var reference: The reference RootObject for this Plotuple
    @var elements: The elements (RootObject of different revisions) for this
        Plotuple
    @var newest: The newest element in elements
    @var key: The key of the object within the corresponding ROOT file
    @var type: The type of the elements (TH1, TH2, TNtuple)
    @var description: The description of this Plotuple object
    @var check: Hint how the Plotuple object should look like
    @var contact: The contact person for this Plotuple object
    @var package: The package to which this Plotuple object belongs to
    @var rootfile: The rootfile to which the Plotuple object belongs to
    @var chi2test_result: The result of the Chi^2-Test. By default, there is no
        such result. If the Chi^2-Test has been performed, this variable holds
        the information between which objects it has been performed.
    @var pvalue: The p-value that the Chi^2-Test returned
    @var file: The file, in which the histogram or the HMTL-table (for
        n-tuples) are stored (without the file extension!)
    """

    def __init__(self, list_of_root_objects, list_of_revisions):
        """!
        The default constructor for a Plotuple-object.
        @param list_of_root_objects: A list of Root-objects which belong
            together (i.e. should be drawn into one histogram or one table)
        @param list_of_revisions: The list of revisions (Duh!)
        """

        # The list of Root objects in this Plotuple-object
        self.list_of_root_objects = list_of_root_objects

        # The list of revisions
        self.list_of_revisions = list_of_revisions

        # A list of all problems that occured with this Plotuple,
        # e.g. missing reference object, missing meta-information...
        self.warnings = []

        # Find the reference element. If we can't find one, set it to 'None'
        # The reference-object for this Plotuple object
        self.reference = None
        for root_object in self.list_of_root_objects:
            if root_object.is_reference:
                self.reference = root_object
                print(self.reference.rootfile)
                break

        # If we couldn't find a reference element, add that to warnings
        if self.reference is None:
            self.warnings = ['No reference object']

        # All elements of the Plotuple that are not the reference-object
        # Get the elements, i.e. all RootObjects except for the
        # reference object. May be either histograms or n-tuples.
        self.elements = sorted([_ for _ in list_of_root_objects if _ is not
                                self.reference],
                               key=lambda _: _.date,
                               reverse=True)

        # The newest element, i.e. the element belonging the revision
        # whose data were created most recently.
        # Should always be self.element[0], except if there is only a
        # reference object
        if self.elements:
            self.newest = self.elements[0]
        else:
            self.newest = self.reference

        # All available meta-information about the plotuple object:

        # The key (more precisely: the name of the key) that all elements of
        # this Plotuple object share
        self.key = self.newest.key

        # The type of the elements in this Plotuple object
        self.type = self.newest.type

        # The description of the histogram/n-tuple which this Plotuple object
        # will yield
        self.description = self.newest.description

        # The 'Check for ...'-guideline for the histogram/n-tuple which this
        # Plotuple object will yield
        self.check = self.newest.check

        # A contact person for the histogram/n-tuple which this Plotuple object
        # will yield
        self.contact = self.newest.contact

        # The meta-options for this Plotuple object
        self.metaoptions = self.newest.metaoptions

        # The package to which the elements in this Plotuple object belong
        self.package = self.newest.package

        # The root file to which the elements in this Plotuple object belong
        self.rootfile = self.newest.rootfile

        # The result of the Chi^2-Test. By default, there is no such result.
        # If the Chi^2-Test has been performed, this variable holds between
        # which objects it has been performed.
        self.chi2test_result = 'n/a'

        # The p-value that the Chi^2-Test returned.
        self.pvalue = 'n/a'

        # The file, in which the histogram or the HMTL-table (for n-tuples)
        # are stored (without the file extension!)
        self.file = None

        # Deal with incomplete information
        if self.description == '' or self.description is None:
            self.description = 'n/a'
            self.warnings.append('No description')
        if self.check == '' or self.check is None:
            self.check = 'n/a'
            self.warnings.append('No Check')
        if self.contact == '' or self.contact is None:
            self.contact = 'n/a'
            self.warnings.append('No Contact Person')

        # Create the actual plot or n-tuple-table
        self.create_plotuple()

    def create_plotuple(self):
        """!
        Creates the histogram/table/image that belongs to this Plotuble-object.
        """

        if self.type == 'TH1':
            self.create_histogram_plot('1D')
        elif self.type == 'TH2':
            self.create_histogram_plot('2D')
        elif self.type == 'TASImage':
            self.create_image_plot()
        elif self.type == 'TNtuple':
            self.create_ntuple_table()
        else:
            sys.exit('Tried to create histogram/n-tuple, '
                     'but received invalid type')

    def chi2test(self, canvas):
        """!
        Takes two RootObject-objects and a canvas. Performs a Chi^2-Test on the
        two histograms and sets the background of the canvas correspondingly.
        Returns the p-value of the Chi^2-Test.
        @param canvas: Reference to the canvas on which we will draw (chi2test
            may change the background color of the plot depending on the
            p-value)
        @return: None
        """
        pvalue = self.reference.object.Chi2Test(self.newest.object)

        # If pvalue < 0.01: Very strong presumption against neutral hypothesis
        if pvalue < 0.01:
            canvas.SetFillColor(ROOT.kRed)
        # If pvalue < 1: Deviations at least exists
        elif pvalue < 1:
            canvas.SetFillColor(ROOT.kOrange)

        self.chi2test_result = ('Performed Chi^2-Test between '
                                'reference and {0}'
                                .format(self.newest.revision))
        self.pvalue = pvalue

    def draw_ref(self, canvas):
        """!
        Takes a reference RootObject and a (sub)canvas and plots it with the
        correct line-style etc.
        @param canvas: Reference to the canvas on which we will draw the
            reference object.
        @return. None
        """
        self.remove_stats_tf1(self.reference.object)

        # Line is thick and black
        self.reference.object.SetLineColor(ROOT.kBlack)
        self.reference.object.SetLineWidth(2)
        self.reference.object.SetLineStyle(1)

        # Area under the curve is solid gray
        self.reference.object.SetFillColor(ROOT.kGray)
        self.reference.object.SetFillStyle(1001)

        # Draw the reference on the canvas
        self.reference.object.DrawCopy(self.reference.object.GetOption())
        canvas.Update()
        canvas.GetFrame().SetFillColor(ROOT.kWhite)

    def remove_stats_tf1(self, object):
        # removed TF1s which might have been added by validation scripts
        # in tracking/scripts/tracking/validation/plot.py:1597
        tf1 = object.FindObject("FitAndStats")
        if tf1:
            function_list = object.GetListOfFunctions()
            function_list.Remove(tf1)

    def create_image_plot(self):
        """!
        Creates image plot for TASImage-objects.
        @return: None
        """

        # Create a ROOT canvas on which we will draw our histograms
        if len(self.elements) > 4:
            canvas = ROOT.TCanvas('', '', 700, 1050)
        else:
            canvas = ROOT.TCanvas('', '', 700, 525)

        # Split the canvas into enough parts to fit all image_objects
        # Find numbers x and y so that x*y = N (number of histograms to be
        # plotted), and x,y close to sqrt(N)

        if len(self.list_of_root_objects) == 1:
            x = y = 1
        elif len(self.list_of_root_objects) == 2:
            x = 2
            y = 1
        else:
            x = 2
            y = int(math.floor((len(self.list_of_root_objects) + 1) / 2))

        # Actually split the canvas and go to the first pad ('sub-canvas')
        canvas.Divide(x, y)
        pad = canvas.cd(1)
        pad.SetFillColor(ROOT.kWhite)

        # If there is a reference object, plot it first
        if self.reference is not None:
            self.draw_ref(pad)

        # Now draw the normal plots
        itemsToPlotCount = len(self.elements)
        for plot in reversed(self.elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision)
            style = get_style(index, itemsToPlotCount)

            self.remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # Switch to the correct sub-panel of the canvas. If a ref-plot
            # exists, we have to go one panel further compared to the
            # no-ref-case
            if self.reference is not None:
                i = 2
            else:
                i = 1

            pad = canvas.cd(self.elements.index(plot) + i)
            pad.SetFillColor(ROOT.kWhite)

            # Draw the reference on the canvas
            plot.object.DrawCopy(plot.object.GetOption())
            pad.Update()
            pad.GetFrame().SetFillColor(ROOT.kWhite)

            # Write the title in the correct color
            title = pad.GetListOfPrimitives().FindObject('title')
            if title:
                title.SetTextColor(style.GetLineColor())

        # Create the folder in which the plot is then stored
        path = ('./plots/{0}/'.format('_'.join(sorted(self.list_of_revisions))) +
                self.package)
        if not os.path.isdir(path):
            os.makedirs(path)

        # Save the plot as PNG and PDF
        canvas.Print('{0}/{1}_{2}.png'.format(path, strip_ext(self.rootfile),
                                              self.key))
        canvas.Print('{0}/{1}_{2}.pdf'.format(path, strip_ext(self.rootfile),
                                              self.key))

        self.file = './{0}/{1}_{2}'.format('/'.join(path.split('/')[2:]),
                                           strip_ext(self.rootfile), self.key)

    def create_histogram_plot(self, mode):
        """!
        Plots all histogram-objects in this Plotuple together in one histogram,
        which is then given the name of the key.
        @param mode: Determines whether it is a one- or
        two-dimensional histogram.
            Accepted values are '1D' and '2D'
        @return: None
        """

        # If we don't get a valid 'mode', we can stop right here
        if mode not in ['1D', '2D']:
            return

        # Create a ROOT canvas on which we will draw our histograms
        if mode == '2D' and len(self.elements) > 4:
            canvas = ROOT.TCanvas('', '', 700, 1050)
        else:
            canvas = ROOT.TCanvas('', '', 700, 525)

        # If there is a reference object, and the list of plots is not empty,
        # perform a Chi^2-Test on the reference object and the first object in
        # the plot list:
        if self.reference is not None and self.newest:
            self.chi2test(canvas)

        # Allow possibility to turn off the stats box
        if 'nostats' in self.metaoptions:
            ROOT.gStyle.SetOptStat("")
        else:
            ROOT.gStyle.SetOptStat("nemr")

        # Now we distinguish between 1D and 2D histograms
        # If we have a 1D histogram
        if mode == '1D':

            if 'nogrid' not in self.metaoptions:
                canvas.SetGrid()
            if 'logx' in self.metaoptions:
                canvas.SetLogx()
            if 'logy' in self.metaoptions:
                canvas.SetLogy()

            # A variable which holds whether we
            # have drawn on the canvas already or not
            drawn = False

            # If there is a reference object, plot it first
            if self.reference is not None:
                self.draw_ref(canvas)
                drawn = True

        # If we have a 2D histogram
        elif mode == '2D':

            # Split the canvas into enough parts to fit all histogram_objects
            # Find numbers x and y so that x*y = N (number of histograms to be
            # plotted), and x,y close to sqrt(N)

            if len(self.list_of_root_objects) == 1:
                x = y = 1
            elif len(self.list_of_root_objects) == 2:
                x = 2
                y = 1
            else:
                x = 2
                y = int(math.floor((len(self.list_of_root_objects) + 1) / 2))

            # Actually split the canvas and go to the first pad ('sub-canvas')
            canvas.Divide(x, y)
            pad = canvas.cd(1)
            pad.SetFillColor(ROOT.kWhite)

            # If there is a reference object, plot it first
            if self.reference is not None:
                self.draw_ref(pad)

        itemsToPlotCount = len(self.elements)
        # Now draw the normal plots
        for plot in reversed(self.elements):

            # Get the index of the current plot
            index = index_from_revision(plot.revision)
            style = get_style(index, itemsToPlotCount)

            self.remove_stats_tf1(plot.object)

            # Set line properties accordingly
            plot.object.SetLineColor(style.GetLineColor())
            plot.object.SetLineWidth(style.GetLineWidth())
            plot.object.SetLineStyle(style.GetLineStyle())

            # If we have a one-dimensional histogram
            if mode == '1D':
                if not drawn:

                    # Get additional options for 1D histograms
                    additional_options = ''
                    for _ in ['C']:
                        if _ in self.metaoptions:
                            additional_options += ' ' + _

                    # Draw the reference on the canvas
                    plot.object.DrawCopy(plot.object.GetOption() +
                                         additional_options)
                    drawn = True
                else:
                    plot.object.DrawCopy("SAME")

                # redraw grid ontop of histogram, if selected
                if 'nogrid' not in self.metaoptions:
                    canvas.RedrawAxis("g")

            # If we have a two-dimensional histogram
            elif mode == '2D':
                # Switch to the correct sub-panel of the canvas. If a ref-plot
                # exists, we have to go one panel further compared to the
                # no-ref-case
                if self.reference is not None:
                    i = 2
                else:
                    i = 1

                pad = canvas.cd(self.elements.index(plot) + i)
                pad.SetFillColor(ROOT.kWhite)

                # Get additional options for 2D histograms
                additional_options = ''
                for _ in ['col', 'colz', 'cont', 'contz', 'box']:
                    if _ in self.metaoptions:
                        additional_options += ' ' + _

                # Draw the reference on the canvas
                plot.object.DrawCopy(plot.object.GetOption() +
                                     additional_options)
                pad.Update()
                pad.GetFrame().SetFillColor(ROOT.kWhite)

                # Write the title in the correct color
                title = pad.GetListOfPrimitives().FindObject('title')
                if title:
                    title.SetTextColor(style.GetLineColor())

        # Create the folder in which the plot is then stored
        path = ('./plots/{0}/'.format('_'.join(sorted(self.list_of_revisions))) +
                self.package)
        if not os.path.isdir(path):
            os.makedirs(path)

        # Save the plot as PNG and PDF
        canvas.Print('{0}/{1}_{2}.png'.format(path, strip_ext(self.rootfile),
                                              self.key))
        canvas.Print('{0}/{1}_{2}.pdf'.format(path, strip_ext(self.rootfile),
                                              self.key))

        self.file = './{0}/{1}_{2}'.format('/'.join(path.split('/')[2:]),
                                           strip_ext(self.rootfile), self.key)

    def create_ntuple_table(self):
        """!
        If the Plotuple-object contains n-tuples, this will create the
        corresponding HTML-table for it.
        """

        # The string which will contain our lines of code
        html = ['<table>']

        # The column names are stored in a
        # separate variable so we can be sure a
        # column only contains values that
        # belong there (no 'shifting' of rows
        # if a column does not exist in a revision etc.)
        columns = []

        # Actually read in the column names and write them into the table
        # headline. The line-variable is used if a single line of HTML is
        # generated in several steps and serves as a buffer. Once the line
        # is finished, it is appended to 'html', which is a list of lines
        line = '<tr><th></th>'
        for key in list(self.newest.object.keys()):
            columns.append(key)
            line += '<th>' + key + '</th>'
        line += '</tr>'
        html.append(line)

        # Now fill the table with values
        # First check if there is a reference object, and in case there
        # is none,
        # create a row which states that no reference object is available
        if self.reference is None and 'reference' in self.list_of_revisions:
            line = '<tr><td>reference</td>'
            for _ in columns:
                line += '<td>n/a</td>'
            line += '</tr>'
            html.append(line)
        elif self.reference is None and 'reference' not in self.list_of_revisions:
            pass
        # If there is a reference object, print the reference values as the
        # first row of the table
        else:
            line = '<tr><td>reference</td>'
            for key in list(self.reference.object.keys()):
                line += '<td>{0:.4f}</td>'.format(self.reference.object[key])
            line += '</tr>'
            html.append(line)

        # Now print the values for all other revisions
        for ntuple in self.elements:
            # Get the index of the object (to retrieve the color)
            ind = index_from_revision(ntuple.revision)

            # Now get the color of the revision
            color = (ROOT.gROOT.GetColor(get_style(ind, len(self.elements))
                                         .GetLineColor()).AsHexString())

            line = '<tr><td style="color: {0};">'.format(color)
            line += ntuple.revision + '</td>'

            for column in columns:
                line += '<td>{0:.4f}</td>'.format(ntuple.object[column])
            line += '</tr>'

            html.append(line)

        html.append('</table>')

        # Create the folder in which the plot is then stored
        path = ('./plots/{0}/'
                .format('_'
                        .join(sorted(self
                                     .list_of_revisions))) + self.package)
        if not os.path.isdir(path):
            os.makedirs(path)

        with open('{0}/{1}_{2}.html'.format(path, strip_ext(self.rootfile),
                                            self.key), 'w+') as html_file:
            for line in html:
                html_file.write(line + '\n')

        self.file = '{0}/{1}_{2}'.format(path, strip_ext(self.rootfile),
                                         self.key)

    def html(self):
        """!
        @return: The HTML Code for the plot.
        """
        html = []

        fxstle = (self.package + "__" +
                  self.file.split("/")[-1].replace(".", "_")).strip()
        classes = ['object_wrap', "nomatrix", fxstle, 'fltr']
        if isinstance(self.pvalue, numbers.Number) and self.pvalue <= 1:
            classes.append('p_value_leq_1')
        if isinstance(self.pvalue, numbers.Number) and self.pvalue <= 0.01:
            classes.append('p_value_leq_0_01')
        # dont do this, because it will hide both ...
        # if "expert" in self.metaoptions:
        #    classes.append('expert_plot')
        prmn = (self.package + "_" +
                self.file.split("/")[-1].replace(".", "_")).strip()
        html.append('<div class="{0}" id="{1}">\n'.format(' '.join(classes),
                                                          prmn))

        # add a new div which will only be used to hide entries with the expert plot
        # qualification
        if "expert" in self.metaoptions:
            html.append('<div class="expert_plot">')

        html.append('<div class="object">\n')

        if self.type == 'TNtuple':
            html.append('\n')
            with open(self.file + '.html', 'r') as table:
                lines = table.readlines()
                for line in lines:
                    html.append(line + '\n')
            html.append('\n')
        else:
            html.append('<a href="./plots/{0}.pdf">'
                        '<img src="./plots/{0}.png"></a>\n'
                        .format(self.file))
            print('\n' + self.file + '\n')

        html.append('</div>\n')

        html.append('<div class="wrap_boxes descriptions">\n')
        if len(self.key) > 45:
            h2 = "-<br>".join([self.key[i:i + 45] for i
                               in range(0, len(self.key), 45)])
        else:
            h2 = self.key
        html.append('<div class="hidebutton" width="100%" align="right" '
                    'style="font-size: 70%;">'
                    '(<a href="#' + self.package + '" '
                    'style="color:black; '
                    'text-decoration:none;">hide this plot</a>)'
                    '</div>\n<h3>' + h2 + '</h3>\n')

        if self.warnings:
            html.append('<p style="color: red;"><strong>Warnings:</strong> '
                        '{0}</p>'.format(', '.join(self.warnings)))

        # Write out which package and root file the plots belongs to
        # (May be helpful for orientation)
        html.append('<p><strong>Package:</strong> {0}</p>\n'
                    '<p><strong>ROOT file:</strong> {1}</p>'
                    .format(self.package, os.path.basename(self.rootfile)))

        if self.type != 'TNtuple':
            html.append('<p><strong>P-Value:</strong> {0}</p>\n'
                        '<p><strong>Chi^2-Test:</strong> {1}</p>\n'
                        .format(self.pvalue, self.chi2test_result))

        html.append('<p><strong>Contact:</strong> {0}</p>'
                    '<p><strong>Description:</strong> {1}</p>\n'
                    '<p><strong>Check for:</strong> {2}</p>\n'
                    .format(self.contact, self.description, self.check))

        # Give the possibility to share the plot!
        html.append('<p><strong>Share this plot/n-tuple:</strong> '
                    '<input type="text" class="share_plot" '
                    'value="index.html?PackageOverview=false&Revisions={0}'
                    '#{1}" size="50" onClick="this.select();"></p>'
                    .format('_'.join(self.list_of_revisions), prmn))

        # Display the meta-options used for this plot (if there are any)
        if self.metaoptions:
            html.append('<p><strong>Used meta-options:</strong> '
                        '<input type="text" class="meta-options" '
                        'value="{0}" size="50" onClick="this.select();"></p>'
                        .format(self.metaoptions))

        if "expert" in self.metaoptions:
            html.append('</div>\n')

        html.append('</div>\n</div>\n\n')

        return html


##############################################################################
#                      Main function starts here!                            #
##############################################################################


def create_plots(revisions=None, force=False, process_pipe=None):
    """!
    This function generates the plots and html
    page for the requested revisions.
    By default all available revisions are taken. New plots will ony be
    created if they don't exist already for the given set of revisions,
    unless the force option is used.
    @param revisions: The revisions which should be taken into account.
    @param force: If True, plots are created even if there already is a version
        of them (which may me deprecated, though)
    @param process_pipe: communication Pipe object, which is used in
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
        generate_new_plots(list_of_revisions, process_pipe)

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
    if process_pipe:
        process_pipe.send({"status": "complete"})
