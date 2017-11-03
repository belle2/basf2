#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import timeit module and start a timer. Allows to get the runtime of the
# program at any given point
import timeit
g_start_time = timeit.default_timer()

import os
import time
import glob
import argparse
import ROOT
import validationpath
import subprocess

###############################################################################
#                           Function definitions                              #
###############################################################################


def get_timezone():
    """
    Returns the correct timezone as short string
    """
    tzTuple = time.tzname

    # in some timezones, there is a daylight saving times entry in the second item
    # of the tuple
    if time.daylight != 0:
        return tzTuple[1]
    else:
        return tzTuple[0]


def get_compact_git_hash(repo_folder):
    """
    Returns the compact git hash from a folder (or any of this folders parents)
    or None if none of theses folders is a git repository
    """
    cwd = os.getcwd()
    os.chdir(repo_folder)
    # todo: we want the short version here
    try:
        current_git_commit = subprocess.check_output(["git", "show", "--oneline", "-s"]).decode().rstrip()
        # the first word in this string will be the hash
        current_git_commit = current_git_commit.split(" ")
        if len(current_git_commit) > 1:
            current_git_commit = current_git_commit[0]
        else:
            # something went wrong, return None
            current_git_commit = None
    except subprocess.CalledProcessError:
        current_git_commit = None
    finally:
        os.chdir(cwd)

    return current_git_commit


def basf2_command_builder(steering_file, parameters, use_multi_processing=False):
    """
    This utility function takes the steering file name and other basf2 parameters
    and returns a list which can be executed via the OS shell for example to
    subprocess.Popen(params ...)
    If use_multi_processing is True, the script will be executed in multi-processing
    mode with only 1 parallel process in order to test if the code also performs
    as expected in multi-processing mode
    """
    cmd_params = ['basf2']
    if use_multi_processing:
        cmd_params += ['-p1']
    cmd_params += [steering_file]
    cmd_params += parameters

    return cmd_params


def available_revisions(work_folder):
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
    revisions = sorted(os.listdir(validationpath.get_results_folder(work_folder)),
                       key=lambda _: os.path.getmtime(os.path.join(validationpath.get_results_folder(work_folder), _)),
                       reverse=True)
    # Return it
    return revisions


def get_start_time():
    """!
    The function returns the value g_start_time which contain the start time
    of the validation and is set just a few lines above.

    @param return: Time since the validation has been started
    """
    return g_start_time


def find_creator(outputfile, package, list_of_scripts, log):
    """!
    This function receives the name of a file and tries to find the file
    in the given package which produces this file, i.e. find the file in
    whose header 'outputfile' is listed under <output></output>.
    It then returns a list of all Scripts who claim to be creating 'outputfile'

    @param outputfile: The file of which we want to know by which script is
        created
    @param package: The package in which we want to search for the creator
    """

    # Get a list of all Script objects for scripts in the given package as well
    # as from the validation-folder
    candidates = [script for script in list_of_scripts
                  if script.package in [package, 'validation']]

    # Reserve some space for the results we will return
    results = []

    # Loop over all candidates and check if they have 'outputfile' listed
    # under their outputs
    for candidate in candidates:
        if candidate.header and \
           outputfile in candidate.header.get('output', []):
            results.append(candidate)

    # Return our results and warn if there is more than one creator
    if len(results) == 0:
        return None
    if len(results) > 1:
        log.warning('Found multiple creators for' + outputfile)
    return results


def get_validation_folders(location, basepaths, log):
    """!
    Collects the validation folders for all packages from the stated release
    directory (either local or central). Returns a dict with the following
    form:
    {'name of package':'absolute path to validation folder of package'}

    @param location: The location where we want to search for validation
        folders (either 'local' or 'central')
    """

    # Make sure we only look in existing locations:
    if location not in ['local', 'central']:
        return {}
    if basepaths[location] is None:
        return {}

    # Write to log what we are collecting
    log.debug('Collecting {0} folders'.format(location))

    # Reserve some memory for our results
    results = {}

    # Now start collecting the folders.
    # First, collect the general validation folders, because it needs special
    # treatment (does not belong to any other package but may include
    # steering files):
    if os.path.isdir(basepaths[location] + '/validation'):
        results['validation'] = basepaths[location] + '/validation'

    # get the special folder containing the validation tests
    if os.path.isdir(basepaths[location] + '/validation/validation-test'):
        results['validation-test'] = basepaths[location] + '/validation/validation-test'

    # Now get a list of all folders with name 'validation' which are
    # subfolders of a folder (=package) in the release directory
    package_dirs = glob.glob(os.path.join(basepaths[location], '*',
                                          'validation'))

    # Now loop over all these folders, find the name of the package they belong
    # to and append them to our results dictionary
    for package_dir in package_dirs:
        package_name = os.path.basename(os.path.dirname(package_dir))
        results[package_name] = package_dir

    # Return our results
    return results


def get_argument_parser(modes=["local"]):
    # Set up the command line parser
    parser = argparse.ArgumentParser()

    # Define the accepted command line flags and read them in
    parser.add_argument("-d", "--dry", help="Perform a dry run, i.e. run the"
                        "validation module without actually executing the"
                        "steering files (for debugging purposes).",
                        action='store_true')
    parser.add_argument("-m", "--mode", help="The mode which will be used for "
                        "running the validation. "
                        "Possible values: " + str(modes) +
                        " Default is 'local'",
                        type=str, nargs='?', default='local')
    parser.add_argument("-i", "--intervals", help="Comma seperated list of intervals "
                        "for which to execute the validation scripts. Default is 'nightly'",
                        type=str, nargs='?', default='nightly')
    parser.add_argument("-o", "--options", help="A string which will be given"
                        "to basf2 as arguments. Example: '-n 100'. "
                        "Quotes are necessary!",
                        type=str, nargs='*')
    parser.add_argument("-p", "--parallel", help="The maximum number of "
                        "parallel processes to run the validation. Only used "
                        "for local execution. Default is number of CPU cores.",
                        type=int, nargs='?', default=None)
    parser.add_argument("-pkg", "--packages", help="The name(s) of one or "
                        "multiple packages. Validation will be run "
                        "only on these packages! E.g. -pkg analysis arich",
                        type=str, nargs='*')
    parser.add_argument("-s", "--select", help="The file name of one or more "
                        "comma separated validation scripts that should be "
                        "executed exclusively. All dependent scripts will also "
                        "be executed. E.g. -s ECL2D.C",
                        type=str, nargs='*')
    parser.add_argument("-si", "--select-ignore-dependencies", help="The file "
                        "name of one or more comma separated validation scripts "
                        "that should be executed exclusively. This will ignore "
                        "all depencies. This is useful if you modified a script "
                        "that produces plots based on the output of its "
                        "dependencies.",
                        type=str, nargs='*')
    parser.add_argument("-q", "--quiet", help="Suppress the progress bar",
                        action='store_true')
    parser.add_argument("-t", "--tag", help="The name that will be used for "
                        "the current revision in the results folder. Possibly "
                        "useful for local basf2 instances where there is no"
                        "BuildBot'. Default is 'current'",
                        type=str, nargs='?', default='current')
    parser.add_argument("--test", help="Execute validation in testing mode"
                        "where only the validation scripts contained in the"
                        "validation package are executed. During regular"
                        "validation, these scripts are ignored.",
                        action='store_true')
    parser.add_argument("--use-cache", help="If validation scripts are marked as "
                        "cacheable and their output files already exist, don't execute "
                        "these scripts again",
                        action='store_true')
    parser.add_argument("--view", help="Once the validation is finished, start"
                                       "the local web server and display the validation"
                                       "results in the system's default browser.",
                        action='store_true')
    return parser


def parse_cmd_line_arguments(isTest=None, tag=None, modes=["local"]):
    """!
    Sets up a parser for command line arguments, parses them and returns the
    arguments.
    @return: An object containing the parsed command line arguments.
        Arguments are accessed like they are attributes of the object,
        i.e. [name_of_object].[desired_argument]
    """

    # Return the parsed arguments!
    return get_argument_parser(modes).parse_args()


def scripts_in_dir(dirpath, log, ext='*'):
    """!
    Returns all the files in the given dir (and its subdirs) that have
    the extension 'ext', if an extension is given (default: all extensions)

    @param dirpath: The directory in which we are looking for files
    @param ext: The extension of the files, which we are looking for.
        '*' is the wildcard-operator (=all extensions are accepted)
    @return: A sorted list of all files with the specified extension in the
        given directory.
    """

    # Write to log what we are collecting
    log.debug('Collecting *{0} files from {1}'.format(ext, dirpath))

    # Some space where we store our results before returning them
    results = []

    # A list of all folder names that will be ignored (e.g. folders that are
    # important for SCons
    blacklist = ['tools', 'scripts', 'examples', validationpath.folder_name_html_static]

    # Loop over the given directory and its subdirectories and find all files
    for root, dirs, files in os.walk(dirpath):

        # Skip a directory if it is blacklisted
        if os.path.basename(root) in blacklist:
            continue

        # Loop over all files
        for current_file in files:
            # If the file has the requested extension, append its full paths to
            # the results
            if current_file.endswith(ext):
                results.append(os.path.join(root, current_file))

    # Return our sorted results
    return sorted(results)


def strip_ext(path):
    """
    Takes a path and returns only the name of the file, without the
    extension on the file name
    """
    return os.path.splitext(os.path.split(path)[1])[0]


def get_style(index, overallItemCount=1):
    """
    Takes an index and returns the corresponding line attributes,
    i.e. LineColor, LineWidth and LineStyle.
    """

    # Define the colors for the plot
    colors = [ROOT.kRed,
              ROOT.kOrange,
              ROOT.kPink + 9,
              ROOT.kOrange - 8,
              ROOT.kGreen + 2,
              ROOT.kCyan + 2,
              ROOT.kBlue + 1,
              ROOT.kRed + 2,
              ROOT.kOrange + 3,
              ROOT.kYellow + 2,
              ROOT.kSpring]

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


def index_from_revision(revision, work_folder):
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
    if revision in available_revisions(work_folder):
        index = available_revisions(work_folder).index(revision)
    # Else return a None object
    else:
        index = None

    return index
