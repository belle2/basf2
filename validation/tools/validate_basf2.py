#!/usr/bin/env python
# -*- encoding: utf-8 -*-

# Import timeit module and start a timer. Allows to get the runtime of the
# program at any given point
import timeit
start = timeit.default_timer()

# Normal library imports
import argparse
import glob
import logging
import os
import re
import subprocess
import sys
import time
import shutil

# Import the controls for local multiprocessing
import localcontrol

# Import XML Parser. Use C-Version, if available
try:
    import xml.etree.cElementTree as XMLTree
except ImportError:
    import xml.etree.ElementTree as XMLTree

# A pretty printer. Prints prettier lists, dicts, etc. :)
import pprint
pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)


################################################################################
###                            Class Definition                              ###
################################################################################

class Validation:
    """!
    This is the class that provides all global variables, like 'list_of_files'
    etc. There is only one instance of this class with name 'validation. This
    allows to use some kind of namespace, i.e. global variables will always be
    referenced as validation.[name of variable]. This makes it easier to
    distinguish them from local variables that only exist within the scope of a
    or a method.
    """

    def __init__(self):
        """!
        The default constructor. Initializes all those variables that will be
        globally accessible later on. Does not return anything.
        """

        # Copy the html skeleton if it is not yet available
        if not os.path.isdir('html'):
            belle2_local_dir = os.environ.get('BELLE2_LOCAL_DIR', None)
            if belle2_local_dir is not None and os.path.isdir(belle2_local_dir + '/validation/html'):
                validation_dir = belle2_local_dir
            else:
                validation_dir = os.environ['BELLE2_RELEASE_DIR']
            shutil.copytree(validation_dir + '/validation/html', 'html')

        ## The logging-object for the validation (Instance of the logging-module)
        # Initialize the log as 'None' and then call the method 'create_log()'
        # to create the actual log.
        self.log = None
        self.create_log()

        ## This dictionary holds the paths to the local and central release dir
        # (or 'None' if one of them does not exist)
        self.basepaths = {'local': os.environ.get('BELLE2_LOCAL_DIR', None),
                          'central': os.environ.get('BELLE2_RELEASE_DIR', None)}

        ## The list which holds all steering file objects
        # (as instances of class Script)
        self.list_of_scripts = []   # Script objects

        ## A list of all packages from which we have collected steering files
        self.list_of_packages = []

        ## The list of packages to be included in the validation. If we are
        # running a complete validation, this will be None.
        self.packages = None

        ## Additional arguments for basf2, if we received any from the command
        # line arguments
        self.basf2_options = ''

        ## A variable which holds the mode, i.e. 'local' for local
        # multi-processing and 'cluster' for cluster usage
        self.mode = None

    def build_dependencies(self):
        """!
        This method loops over all Script objects in self.list_of_scripts and
        calls their get_dependencies()-method.
        @return: None
        """
        for script_object in self.list_of_scripts:
            script_object.get_dependencies()

        # The following code is only necessary while there are still a lot of
        # steering files without proper headers.
        # It adds all steering files from the validation-folder as a default
        # dependency, because a lot of scripts depend on one data script that
        # is created by a steering file in the validation-folder.
        default_depend = [script for script in validation.list_of_scripts if script.package == 'validation']
        for script_object in self.list_of_scripts:
            if not script_object.header and script_object.package != \
                    'validation':
                script_object.dependencies += default_depend

    def build_headers(self):
        """!
        This method loops over all Script objects in self.list_of_scripts and
        calls their get_header()-method.
        @return: None
        """
        for script_object in self.list_of_scripts:
            script_object.get_header()

    def create_log(self):
        """!
        Create the logger.
        We use the logging module to create an object which allows us to
        comfortably log everything that happens during the execution of
        this script and even have different levels of importance, such as
        'ERROR' or 'DEBUG'.
        @return: None
        """
        # Create the log and set its default level to DEBUG, which means that
        # it will store _everything_.
        self.log = logging.getLogger('validate_basf2')
        self.log.setLevel(logging.DEBUG)

        # Now we add another custom level 'NOTE'. This is because we don't
        # want to print ERRORs and WARNINGs to the console output, therefore
        # we need a higher level.
        # We define the new level and tell 'self.log' what to do when we use it.
        logging.NOTE = 100
        logging.addLevelName(logging.NOTE, 'NOTE')
        self.log.note = lambda msg, *args: self.log._log(logging.NOTE,
                                                         msg, args)

        # Set up the console handler. The console handler will redirect a
        # certain subset of all log message (i.e. those with level 'NOTE') to
        # the command line (stdout), so we know what's going on when we
        # execute the validation.

        # Define the handler and its level (=NOTE)
        console_handler = logging.StreamHandler()
        console_handler.setLevel(logging.NOTE)

        # Format the handler. We only need the message, no date/time etc.
        console_format = logging.Formatter('%(message)s')
        console_handler.setFormatter(console_format)

        # Add the console handler to self.log
        self.log.addHandler(console_handler)

        # Now set up the file handler. The file handler will redirect
        # _everything_ we log to a logfile so that we have all possible
        # information available for debugging later.

        # Make sure the folder for the log file exists
        log_dir = './results/current/__general__/'
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)

        # Define the handler and its level (=DEBUG to get everything)
        file_handler = logging.FileHandler(log_dir + 'validate_basf2.log', 'w+')
        file_handler.setLevel(logging.DEBUG)

        # Format the handler. We want the datetime, the module that produced
        # the message, the LEVEL of the message and the message itself
        file_format = logging.Formatter('%(asctime)s - %(name)s - '
                                        '%(levelname)s - %(message)s',
                                        datefmt='%Y-%m-%d %H:%M:%S')
        file_handler.setFormatter(file_format)

        # Add the file handler to self.log
        self.log.addHandler(file_handler)

    def collect_steering_files(self):
        """!
        This function will collect all steering files from the local and
        central release directory and will store the corresponding paths in
        self.list_of_sf_paths.
        @return: None
        """

        # Get all folders that contain steering files, first the local ones
        validation_folders = get_validation_folders('local')

        # Then add those central folders that do not have a local match
        for (package, folder) in get_validation_folders('central').items():
            if not package in validation_folders.keys():
                validation_folders[package] = folder

        # If we are not performing a complete validation, we need to remove
        # all packages that were not given via the '--packages' option
        if self.packages is not None:
            unwanted_keys = [_ for _ in validation_folders if _ not
                             in self.packages]
            for unwanted_key in unwanted_keys:
                del validation_folders[unwanted_key]

        # Now write to self.list_of_packages which packages we have collected
        self.list_of_packages = validation_folders.keys()

        # Finally, we collect the steering files from each folder we have
        # collected:
        for (package, folder) in validation_folders.items():

            # Collect only *.C and *.py files
            c_files = scripts_in_dir(folder, '.C')
            py_files = scripts_in_dir(folder, '.py')
            for steering_file in c_files + py_files:
                self.list_of_scripts.append(Script(steering_file, package))

        # Thats it, now there is a complete list of all steering files on
        # which we are going to perform the validation in self.list_of_scripts

    def run_validation(self):
        """!
        This method runs the actual validation, i.e. it loops over all
        scripts, checks which of them are ready for execution, and runs them.
        @return: None
        """

        # Use the local execution for all plotting scripts
        local_control = localcontrol.Local()

        # Depending on the selected mode, load either the controls for the
        # cluster or for local multi-processing
        if self.mode == 'cluster':
            import clustercontrol
            control = clustercontrol.Cluster()
        else:
            control = local_control

        # This variable is needed for the progress bar function
        progress_bar_lines = 0
        print

        # The list of scripts that have to be processed
        remaining_scripts = [script for script in self.list_of_scripts if script.status == 'waiting']

        # While there are scripts that have not yet been executed...
        while remaining_scripts:

            # Loop over all steering files / Script objects
            for script_object in remaining_scripts:

                # If the script is currently running
                if script_object.status == 'running':

                    # Check if the script has finished:
                    result = script_object.control.is_job_finished(script_object)

                    # If it has finished:
                    if result[0]:

                        # Write to log that the script finished
                        self.log.debug('Finished: ' + script_object.path)

                        # Check for the return code and set variables correspondingly
                        script_object.status = 'done'
                        if result[1] != 0:
                            script_object.status = 'failed'
                            self.log.warning('exit_status was {0} for {1}'
                                             .format(result[1],
                                                     script_object.path))
                        script_object.returncode = result[1]

                        # Remove this script from the dependencies of dependent script objects and skip dependent scripts if this one failed
                        for dependent_script in remaining_scripts:
                            if script_object in dependent_script.dependencies:
                                dependent_script.dependencies.remove(script_object)
                                if result[1] != 0:
                                    self.log.warning('Skipping ' + dependent_script.path)
                                    dependent_script.status = 'skipped'

                # Otherwise (the script is waiting) and if it is ready to be executed
                elif not script_object.dependencies:

                    # Determine the way of execution depending on whether data files are created
                    if script_object.header and script_object.header.get('output', []):
                        script_object.control = control
                    else:
                        script_object.control = local_control

                    # Do not spawn processes if there are already too many!
                    if script_object.control.available():

                        # Start execution and set attributes for the script
                        self.log.debug('Starting ' + script_object.path)
                        script_object.control.execute(script_object, self.basf2_options)
                        script_object.status = 'running'

            # Update the list of scripts that have to be processed
            remaining_scripts = [script for script in remaining_scripts if script.status in ['waiting', 'running']]

            # Wait for one second before starting again
            time.sleep(1)

            progress_bar_lines = draw_progress_bar(progress_bar_lines)

        print

    def create_plots(self):
        """!
        This method prepares the html directory for the plots if necessary
        and creates the plots that include the results from this validation.
        @return: None
        """

        # Go to the html directory and create a link to the results folder if it is not yet existing
        save_dir = os.getcwd()
        os.chdir('html')
        if not os.path.exists('results'):
            os.symlink('../results', 'results')

        # import and run plot function
        from validationplots import create_plots
        create_plots(force=True)

        # restore original working directory
        os.chdir(save_dir)


class Script:
    """!
    The object representation of a steering file.
    """

    def __init__(self, path, package):
        """!
        The default constructor.
        """

        ## Pointer to the script object itself
        # Is this necessary?
        self._object = self

        ## The (absolute) path of the steering file
        self.path = path

        ## The name of the steering file. Basically the file name of the
        # steering file, but everything that is not a letter is replaced
        # by an underscore. Useful e.g. for cluster controls.
        self.name = re.sub(r'[\W_]+', '_', str(os.path.basename(self.path)))

        ## The package to which the steering file belongs
        self.package = package

        ## The information from the file header
        self.header = None

        ## A list of script objects, on which this script depends
        self.dependencies = []

        ## Current status of the script.
        # Possible values: 'waiting', 'running', 'finished', 'failed'
        self.status = 'waiting'

        ## Which control is used for executing the script, i.e. cluster or
        # local. Useful when using different script level, e.g. data creation
        # scripts are being run on the cluster, but plotting scripts are
        # executed locally
        self.control = None

        ## The returncode of the script. Should be 0 if all went well.
        self.returncode = None

    def dump(self):
        """!
        Print out all properties = attributes of a script.
        @return: None
        """
        print
        pp.pprint(vars(self))

    def get_dependencies(self):
        """!
        Loops over the input files given in the header and tries to find the
        corresponding Script objects, which will then be stored in the
        script.dependencies-list
        @return: None
        """
        # If all necessary header information are available:
        if self.header is not None:

            # Loop over all the dependencies given in the header information
            for root_file in self.header.get('input', []):

                # Find the script which is responsible for the creation of
                # the input file (in the same package or in validation folder)
                creator = find_creator(root_file, self.package)

                # If no creator could be found, raise an error!
                if creator is None:
                    validation.log.error('Unmatched dependency for {0}:'
                                         '{1} has no creator!'
                                         .format(self.path, root_file))
                    self.status = 'skipped'

                # If creator(s) could be found, add those scripts to the
                # list of scripts on which self depends
                else:
                    self.dependencies += creator

            # remove double entries
            self.dependencies = list(set(self.dependencies))

        # If the necessary header information are not available:
        else:
            # If there is a script whose name comes before this script, this
            # is presumed as a dependency

            # Get a list of all the script in the same directory
            in_same_pkg = [script for script in validation.list_of_scripts if script.package == self.package]

            # Divide that list into .py and .c files, because .py files are
            # always executed before .C files:
            py_files = [_ for _ in in_same_pkg if _.path.endswith('py')]
            c_files = [_ for _ in in_same_pkg if _.path.endswith('C')]

            # Make sure the lists are ordered by the path of the files
            py_files.sort(key=lambda x: x.path)
            c_files.sort(key=lambda x: x.path)

            # Now put the two lists back together
            in_same_pkg = py_files + c_files

            if in_same_pkg.index(self) - 1 >= 0:
                predecessor = in_same_pkg[in_same_pkg.index(self) - 1]
                self.dependencies.append(predecessor)

    def get_header(self):
        """!
        This method opens the file given in self.path, tries to extract the
        XML-header of it and then parse it.
        It then fills the self.header variable with a dict containing the
        values that were read from the XML header.
        @return: None
        """

        # Read the file as a whole
        with open(self.path, "r") as data:
            steering_file_content = data.read()

        # Define the regex to extract everything between the <header>-tags
        pat = re.compile('(<header>.*?</header>)', re.DOTALL | re.M)

        # Apply the regex, i.e. filter out the <header>...</header> part of
        # each steering file.
        try:
            xml = pat.findall(steering_file_content)[0].strip()
        except IndexError:
            validation.log.error('No file header found: ' + self.path)
            return

        # Create an XML tree from the plain XML code.
        try:
            xml_tree = XMLTree.ElementTree(XMLTree.fromstring(xml)).getroot()
        except XMLTree.ParseError:
            validation.log.error('Invalid XML in header: ' + self.path)
            return

        # we have a header
        self.header = {}

        # Loop over that tree
        for branch in xml_tree:

            # The keywords that should be parsed into a list
            list_tags = ['input', 'output', 'contact']

            # Format the values of each branch
            if branch.tag.strip() in list_tags:
                branch_value = [__.strip() for __ in branch.text.split(',')]
                if branch_value == ['']:
                    branch_value = []
            else:
                branch_value = re.sub(' +', ' ', branch.text.replace('\n', ''))
                branch_value = branch_value.strip()

            # Append the branch and its values to the header-dict. This
            # implementation technically allows multiple occurrences of the
            # same <tag></tag>-pair, which will be bundled to the same key in
            # the key in the returned dictionary
            if branch.tag.strip() in self.header:
                self.header[branch.tag.strip()] += branch_value
            else:
                self.header[branch.tag.strip()] = branch_value


###############################################################################
###                         Function definitions                             ###
################################################################################

def find_creator(outputfile, package):
    """!
    This function receives the name of a file and tries to find the file
    in the given package which produces this file, i.e. find the file in
    whose header 'outputfile' is listed under <output></output>.
    It then returns a list of all Scripts who claim to be creating 'outputfile'.

    @param outputfile: The file of which we want to know by which script is
        created
    @param package: The package in which we want to search for the creator
    """

    # Get a list of all Script objects for scripts in the given package as well
    # as from the validation-folder
    candidates = [script for script in validation.list_of_scripts if script.package in [package, 'valiation']]

    # Reserve some space for the results we will return
    results = []

    # Loop over all candidates and check if they have 'outputfile' listed
    # under their outputs
    for candidate in candidates:
        if candidate.header and outputfile in candidate.header.get('output', []):
            results.append(candidate)

    # Return our results and warn if there is more than one creator
    if len(results) == 0:
        return None
    if len(results) > 1:
        validation.log.warning('Found multiple creators for' + outputfile)
    return results


def get_validation_folders(location):
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
    if validation.basepaths[location] is None:
        return {}

    # Write to log what we are collecting
    validation.log.debug('Collecting {0} folders'.format(location))

    # Reserve some memory for our results
    results = {}

    # Now start collecting the folders.
    # First, collect the general validation folders, because it needs special
    # treatment (does not belong to any other package but may include
    # steering files):
    if os.path.isdir(validation.basepaths[location] + '/validation'):
        results['validation'] = validation.basepaths[location] + '/validation'

    # Now get a list of all folders with name 'validation' which are
    # subfolders of a folder (=package) in the release directory
    package_dirs = glob.glob(os.path.join(validation.basepaths[location], '*',
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
    parser.add_argument("-o", "--options", help="A string which will be given"
                                                "to basf2 as arguments. "
                                                "Example: '-n 100'. "
                                                "Quotes are necessary!",
                        type=str, nargs='*')
    parser.add_argument("-pkg", "--packages", help="The name(s) of one or "
                                                   "multiple packages. "
                                                   "Validation will be run "
                                                   "only on these packages! "
                                                   "E.g. -pkgte analysis arich",
                        type=str, nargs='*')
    parser.add_argument("-m", "--mode", help="The mode which will be used for "
                                             "running the validation. Two "
                                             "possible values: 'local' or "
                                             "'cluster'. Default is 'local'",
                        type=str, nargs='?', default='local')

    # Return the parsed arguments!
    return parser.parse_args()


def scripts_in_dir(dirpath, ext='*'):
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
    validation.log.debug('Collecting *{0} files from {1}'.format(ext, dirpath))

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


def draw_progress_bar(delete_lines, barlength=50):
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
    finished_scripts = len([_ for _ in validation.list_of_scripts if
                            _.status in ['done', 'failed', 'skipped']])
    all_scripts = len(validation.list_of_scripts)
    percent = 100.0 * finished_scripts / all_scripts

    # Get the runtime of the script
    runtime = int(timeit.default_timer() - start)

    # Move the cursor up and clear lines
    for i in range(delete_lines):
        print "\x1b[2K \x1b[1A",

    # Print the progress bar:
    progressbar = ""
    for i in range(barlength):
        if i < int(barlength * percent / 100.0):
            progressbar += '='
        else:
            progressbar += ' '
    print '\x1b[0G[{0}] {1:6.1f}% ({2}/{3})'.format(progressbar, percent,
                                                    finished_scripts,
                                                    all_scripts)

    # Print the total runtime:
    print 'Runtime: {0}s'.format(runtime)

    # Print the list of currently running scripts:
    running = [os.path.basename(__.path) for __ in validation.list_of_scripts
               if __.status == 'running']

    # If nothing is currently running
    if not running:
        running = ['-']

    print 'Running: {0}'.format(running[0])
    for __ in running[1:]:
        print '{0} {1}'.format(len('Running:') * " ", __)

    return len(running) + 2

################################################################################
###                     Actual program starts here!                          ###
################################################################################

# If there is no release of basf2 set up, we can stop the execution right here!
if os.environ.get('BELLE2_RELEASE', None) is None:
    sys.exit('Error: No basf2 release set up!')

# Otherwise we can start the execution.
# The main part is wrapped in a try/except-contruct to fetch keyboard interrupts
try:

    # Create the validation object. 'validation' holds all global variables
    # and provides the logger!
    validation = Validation()

    # Write to log that we have started the validation process
    validation.log.note('Starting validation...')

    # Now we process the command line arguments.
    # First of all, we read them in:
    cmd_arguments = parse_cmd_line_arguments()

    # Now we check whether we are running a complete validation or only
    # validating a certain set of packages:
    if cmd_arguments.packages:
        validation.packages = cmd_arguments.packages
        validation.log.note('Only validating package(s): {0}'
                            .format(', '.join(validation.packages)))
    else:
        validation.log.note('Performing complete validation...')

    # Check if we received additional arguments for basf2
    if cmd_arguments.options:
        validation.basf2_options = ' '.join(cmd_arguments.options)
        validation.log.note('Received arguments for basf2: {0}'
                            .format(validation.basf2_options))

    # Check if we are using the cluster or local multiprocessing:
    if cmd_arguments.mode and cmd_arguments.mode in ['local', 'cluster']:
        validation.mode = cmd_arguments.mode
    else:
        validation.mode = 'local'
    if validation.mode == 'local':
        validation.log.note('Validation will use local multi-processing.')
    elif validation.mode == 'cluster':
        validation.log.note('Validation will use the cluster.')

    # Now collect the steering files which will be used in this validation.
    # This will fill validation.list_of_sf_paths with values.
    validation.log.note('Collecting steering files...')
    validation.collect_steering_files()

    # Build headers for every script object we have created
    validation.log.note('Building headers for Script objects...')
    validation.build_headers()

    # Build dependencies for every script object we have created
    validation.log.note('Building dependencies for Script objects...')
    validation.build_dependencies()

    # Start the actual validation
    validation.log.note('Starting the validation...')
    validation.run_validation()

    # Log that the validation has finished and that we are creating plots
    validation.log.note('Validation finished...')
    validation.log.note('Start creating plots...')
    validation.create_plots()

    # Log that everything is finished
    validation.log.note('Plots have been created...')
    validation.log.note('Validation finished! Total runtime: {0}s'
                        .format(int(timeit.default_timer() - start)))

except KeyboardInterrupt:
    validation.log.note('Validation terminated by user!')
