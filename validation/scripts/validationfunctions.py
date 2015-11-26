#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Import timeit module and start a timer. Allows to get the runtime of the
# program at any given point
import timeit
g_start_time = timeit.default_timer()

import os
import glob
import argparse

import validationscript

###############################################################################
#                           Function definitions                              #
###############################################################################


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


def parse_cmd_line_arguments():
    """!
    Sets up a parser for command line arguments, parses them and returns the
    arguments.
    @return: An object containing the parsed command line arguments.
        Arguments are accessed like they are attributes of the object,
        i.e. [name_of_object].[desired_argument]
    """

    # Set up the command line parser
    parser = argparse.ArgumentParser()

    # Define the accepted command line flags and read them in
    parser.add_argument("-d", "--dry", help="Perform a dry run, i.e. run the"
                        "validation module without actually executing the"
                        "steering files (for debugging purposes).",
                        action='store_true')
    parser.add_argument("-m", "--mode", help="The mode which will be used for "
                        "running the validation. Two possible values: 'local' "
                        "or 'cluster'. Default is 'local'",
                        type=str, nargs='?', default='local')
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
                        "executed exclusively. All dependant scripts will also"
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
    parser.add_argument("--view", help="Once the validation is finished, start"
                                       "the local web server and display the validation"
                                       "results in the system's default browser.",
                        action='store_true')

    # Return the parsed arguments!
    return parser.parse_args()


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
    blacklist = ['tools', 'scripts', 'examples', 'html']

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
                results.append(root + '/' + current_file)

    # Return our sorted results
    return sorted(results)


def draw_progress_bar(delete_lines, list_of_scripts, barlength=50):
    """
    This function plots a progress bar of the validation, i.e. it shows which
    percentage of the scripts has been executed yet.
    It furthermore also shows which scripts are currently running, as well as
    the total runtime of the validation.

    @param delete_lines: The amount of lines which need to be deleted before
        we can redraw the progress bar
    @param barlength: The length of the progess bar (in characters)
    @return: The number of lines that were printed by this function call.
        Usefule if this function is called repeatedly.
    """

    # Get statistics: Number of finished scripts + number of scripts in total
    finished_scripts = len([_ for _ in list_of_scripts if
                            _.status in [validationscript.ScriptStatus.finished,
                                         validationscript.ScriptStatus.failed,
                                         validationscript.ScriptStatus.skipped]])
    all_scripts = len(list_of_scripts)
    percent = 100.0 * finished_scripts / all_scripts

    # Get the runtime of the script
    runtime = int(timeit.default_timer() - get_start_time())

    # Move the cursor up and clear lines
    for i in range(delete_lines):
        print("\x1b[2K \x1b[1A", end=' ')

    # Print the progress bar:
    progressbar = ""
    for i in range(barlength):
        if i < int(barlength * percent / 100.0):
            progressbar += '='
        else:
            progressbar += ' '
    print('\x1b[0G[{0}] {1:6.1f}% ({2}/{3})'.format(progressbar, percent,
                                                    finished_scripts,
                                                    all_scripts))

    # Print the total runtime:
    print('Runtime: {0}s'.format(runtime))

    # Print the list of currently running scripts:
    running = [os.path.basename(__.path) for __ in list_of_scripts
               if __.status == validationscript.ScriptStatus.running]

    # If nothing is repeatedly running
    if not running:
        running = ['-']

    print('Running: {0}'.format(running[0]))
    for __ in running[1:]:
        print('{0} {1}'.format(len('Running:') * " ", __))

    return len(running) + 2
