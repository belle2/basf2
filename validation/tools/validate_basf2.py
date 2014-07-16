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

# Import the controls for local multiprocessing and the cluster
import clustercontrol
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
    """
    This is the class that provides all global variables, like 'list_of_files'
    etc. There is only one instance of this class with name 'validation. This
    allows to use some kind of namespace, i.e. global variables will always be
    referenced as validation.[name of variable]. This makes it easier to
    distinguish them from local variables that only exist within the scope of a
    or a method.
    """

    def __init__(self):
        """
        The default constructor. Initializes all those variables that will be
        globally accessible later on. Does not return anything.
        """

        # Initialize the log as 'None' and then call the method 'create_log()'
        # to create the actual log.
        self.log = None
        self.create_log()

        # This dictionary holds the paths to the local and central release dir
        # (or 'None' if one of them does not exist)
        self.basepaths = {'local': os.environ.get('BELLE2_LOCAL_DIR', None),
                          'central': os.environ.get('BELLE2_RELEASE_DIR', None)}

        # Initialize the list which will later hold all steering file objects
        # (as instances of class Script)
        self.list_of_scripts = []   # Script objects

        # Initialize the dict which will hold all paths to steering files in
        # the form: {'package' : [list of sf paths for this package]}
        self.dict_of_sf_paths = {}

        # A list of all packages from which we have collected steering files
        self.list_of_packages = []

        # Now we need to distinguish if we want to run the entire validation
        # (all available packages) or only some specific packages.
        self.complete_validation = True  # Default is complete validation!
        self.packages = None             # The specific packages (if applicable)

        # This is where we will store additional arguments for basf2,
        # if we received any from the command line arguments
        self.basf2_options = ''

        # A dictionary which contains all validation folders in the form:
        # {'name of package':'path to validation folder of package'}
        self.validation_folders = None

        # A variable which holds the mode, i.e. 'local' for local
        # multi-processing and 'cluster' for cluster usage
        self.mode = None

        # The maximum number of processes that may run simultaneosly, as well
        # as the number of processes that are currently running
        self.max_number_of_processes = 10
        self.current_number_of_processes = 0

    def build_dependencies(self):
        """
        This method loops over all Script objects in self.list_of_scripts and
        calls their get_dependencies()-method.
        """
        for script_object in self.list_of_scripts:
            script_object.get_dependencies()

        # The following code is only necessary while there are still a lot of
        # steering files without proper headers.
        # It adds all steering files from the validation-folder as a default
        # dependency, because a lot of scripts depend on one data script that
        # is created by a steering file in the validation-folder.
        default_depend = filter_list_of_scripts(validation.list_of_scripts,
                                                PACKAGE='validation')
        for script_object in self.list_of_scripts:
            if script_object.header_incomplete and script_object.package != \
                    'validation':
                script_object.dependencies += default_depend

    def build_headers(self):
        """
        This method loops over all Script objects in self.list_of_scripts and
        calls their get_header()-method.
        """
        for script_object in self.list_of_scripts:
            script_object.get_header()

    def create_log(self):
        """
        Create the logger.
        We use the logging module to create an object which allows us to
        comfortably log everything that happens during the execution of
        this script and even have different levels of importance, such as
        'ERROR' or 'DEBUG'.
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
        log_dir = './html/logs/__general__/'
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
        """
        This function will collect all steering files from the local and
        central release directory and will store the corresponding paths in
        self.list_of_sf_paths.
        """

        # Get all folders that contain steering files
        local_folders = get_validation_folders('local')
        central_folders = get_validation_folders('central')

        # Now we need to get a rid of the duplicates. Everthing that exists
        # locally does not need to be taken from the central dir.

        # Take all local folders:
        self.validation_folders = local_folders

        # Only take those central folders that do not have a local match
        for package in central_folders:
            if not package in self.validation_folders.keys():
                self.validation_folders[package] = central_folders[package]

        # If we are not performing a complete validation, we need to remove
        # all packages that were not given via the '--packages' option
        if not self.complete_validation:
            unwanted_keys = [_ for _ in self.validation_folders if _ not
                             in self.packages]
            for unwanted_key in unwanted_keys:
                del self.validation_folders[unwanted_key]

        # Now write to self.list_of_packages which packages we have collected
        self.list_of_packages = self.validation_folders.keys()

        # Finally, we collect the steering files from each folder we have
        # collected:
        for package in self.validation_folders:

            self.dict_of_sf_paths[package] = []

            # Collect only *.c and *.py files
            pkg_path = self.validation_folders[package]
            c_files = scripts_in_dir(pkg_path, '.c')
            py_files = scripts_in_dir(pkg_path, '.py')

            for steering_file in c_files + py_files:
                self.dict_of_sf_paths[package].append(steering_file)

        # Thats it, now there is a complete dict of all steering files on
        # which we are going to perform the validation in self.dict_of_sf_paths

    def create_script_objects(self):
        """
        Description
        """

        for package in self.dict_of_sf_paths:
            for steering_file in self.dict_of_sf_paths[package]:
                new_script_object = Script(steering_file, package)
                self.list_of_scripts.append(new_script_object)

    def run_validation(self):
        """
        This method runs the actual validation, i.e. it loops over all
        scripts, checks which of them are ready for execution, and runs them.
        """

        # Depending on the selected mode, load either the controls for the
        # cluster or for local multi-processing
        if self.mode == 'cluster':
            executioner = clustercontrol.Cluster()
        else:
            executioner = localcontrol.Local()

        # This variable is needed for the progress bar function
        progress_bar_lines = 0
        print

        #While there are scripts that have not yet been executed...
        while not all(__.finished for __ in self.list_of_scripts):

            # Loop over all steering files / Script objects
            for script_object in self.list_of_scripts:

                # If already executed, skip the script
                if script_object.finished:
                    continue

                # If the script is currently running
                if script_object.running:

                    # Check if the script has finished:
                    result = executioner.is_job_finished(script_object)

                    # If it has finished:
                    if result[0]:

                        # Write to log that the script finished
                        self.log.debug('Finished: ' + script_object.path)

                        # Check for the return code
                        if result[1] != 0:
                            self.log.warning('exit_status was {0} for {1}'
                                             .format(result[1],
                                                     script_object.path))

                        # Set variables correspondingly
                        script_object.finished = True
                        script_object.running = False
                        script_object.returncode = result[1]
                        self.current_number_of_processes -= 1
                        continue

                    # If not, let the execution continue
                    else:
                        continue

                # If the script is neither finished nor currently running,
                # but ready to be executed
                if script_object.dependencies == \
                        script_object.settled_dependencies:

                    # Do not spawn processes if there are already too many!
                    if self.current_number_of_processes <= \
                            self.max_number_of_processes:

                        # Start execution and set attributes for the script
                        self.log.debug('Starting ' + script_object.path)
                        executioner.execute(script_object, self.basf2_options)
                        script_object.running = True
                        self.current_number_of_processes += 1
                    else:
                        # Too many processes
                        pass

            # Loop over all scripts again and update the settled_dependencies
            for script_object in self.list_of_scripts:

                # Clear the settled_dependencies
                script_object.settled_dependencies = []

                # Loop over all dependencies
                for dependency in script_object.dependencies:
                    # If the dependency has been executed already,
                    if dependency.finished:
                        # Append it to the list of settled dependencies
                        script_object.settled_dependencies.append(dependency)

            # Wait for one second before starting again
            time.sleep(1)

            progress_bar_lines = draw_progress_bar(progress_bar_lines)

        print


class Script:
    """
    The object representation of a steering file.
    """

    def __init__(self, path, package):
        """
        The default constructor.
        """

        self._object = self
        self.path = path
        self.name = re.sub(r'[\W_]+', '_', str(os.path.basename(self.path)))
        self.package = package
        self.dir = os.path.dirname(self.path)
        self.header = {}
        self.finished = False
        self.running = False
        self.returncode = None
        self.header_incomplete = False
        self.dependencies = []
        self.settled_dependencies = []

    def dump(self):
        """
        Print out all properties = attributes of a script.
        """
        print
        pp.pprint(vars(self))

    def get_dependencies(self):
        """
        Loops over the dependencies given in the header and tries to find the
        corresponding Script objects, which will then be stored in the
        script.dependencies-list
        """
        # Get a list of all the script in the same directory
        in_same_pkg = filter_list_of_scripts(validation.list_of_scripts,
                                             PACKAGE=self.package)

        # Divide that list into .py and .c files, because .py files are
        # always executed before .c files:
        py_files = [_ for _ in in_same_pkg if _.path.lower().endswith('py')]
        c_files = [_ for _ in in_same_pkg if _.path.lower().endswith('c')]

        # Make sure the lists are ordered by the path of the files
        py_files.sort(key=lambda x: x.path)
        c_files.sort(key=lambda x: x.path)

        # Now put the two lists back together
        in_same_pkg = py_files + c_files

        # If all necessary header information are available:
        if not self.header_incomplete:

            # If the script simply has no dependencies
            if not self.header['dependencies']:
                self.dependencies = []
            else:
                # Loop over all the dependencies given in the header information
                for dependency in self.header['dependencies']:

                    # Find the script which is responsible for the creation of
                    # 'dependency' (in the same package or in validation folder)
                    creator = find_creator(dependency, self.package)

                    # If no creator could be found, raise an error!
                    if creator is None:
                        validation.log.error('Unmatched dependency for {0}:'
                                             '{1} has no creator!'
                                             .format(self.path, dependency))

                    # If creator(s) could be found, add those scripts to the
                    # list of scripts on which self depends
                    else:
                        self.dependencies += creator
        # If the necessary header information are not available:
        else:
            # If there is a script whose name comes before this script, this
            # is presumed as a dependency
            if in_same_pkg.index(self) - 1 >= 0:
                predecessor = in_same_pkg[in_same_pkg.index(self) - 1]
                self.dependencies.append(predecessor)

    def get_header(self):
        """
        This method opens the file given in self.path, tries to extract the
        XML-header of it and then parse it.
        It then fills the self.header variable with a dict containing the
        values that were read from the XML header.
        If there is no header, or the XML is corrupt, it will fill the
        self.header variable with a dict that has all possible keywords,
        but no actual values, i.e. {'keyword':[]}
        """

        # The dict that contains all accepted keywords, with distinction if
        # they are mandatory or optional
        dict_of_keywords = {'mandatory': ['category', 'dependencies', 'output'],
                            'optional': ['author', 'description']}

        # All keywords as a plain list
        list_of_keywords = sum(dict_of_keywords.values(), [])

        # Read the file as a whole
        with open(self.path, "r") as data:
            steering_file_content = data.read()

        # Define the regex to extract everything between the <header>-tags
        pat = re.compile('(<header>.*?</header>)', re.DOTALL | re.M)

        # Apply the regex, i.e. filter out the <header>...</header> part of
        # each steering file. If no such part can be found the result is an
        # empty dict!
        try:
            xml = pat.findall(steering_file_content)[0].strip()
        except IndexError:
            validation.log.error('No file header found: ' + self.path)
            self.header = {keyword: [] for keyword in list_of_keywords}
            self.header_incomplete = True
            return

        # Create an XML tree from the plain XML code. If this fails, the result
        # is again an empty dict!
        try:
            xml_tree = XMLTree.ElementTree(XMLTree.fromstring(xml)).getroot()
        except XMLTree.ParseError:
            validation.log.error('Invalid XML in header: ' + self.path)
            self.header = {keyword: [] for keyword in list_of_keywords}
            self.header_incomplete = True
            return

        # Loop over that tree
        for branch in xml_tree:

            # The keywords that do not need to parsed into a list
            simple_strings = ['description', 'category']

            # Format the values of each branch
            if branch.tag.strip() in simple_strings:
                branch_value = re.sub(' +', ' ', branch.text.replace('\n', ''))
                branch_value = branch_value.strip()
            # For all other keywords, it is useful to store the information
            # in a list, e.g. a list of all output files
            else:
                branch_value = [__.strip() for __ in branch.text.split(',')]
                if branch_value == ['']:
                    branch_value = []

            # Append the branch and its values to the header-dict. This
            # implementation technically allows multiple occurrences of the
            # same <tag></tag>-pair, which will be bundled to the same key in
            # the key in the returned dictionary
            if branch.tag.strip() in self.header:
                self.header[branch.tag.strip()] += branch_value
            else:
                self.header[branch.tag.strip()] = branch_value

        # Make sure that every expected keyword at least exists as an empty
        # key in self.header to avoid IndexErrors (since e.g.
        # get_dependencies always tries to access
        # self.header['dependencies'], which will raise an
        # exception if no dependencies have been stated
        for mandatory_keyword in dict_of_keywords['mandatory']:
            if mandatory_keyword not in self.header:
                self.header[mandatory_keyword] = ''
                self.header_incomplete = True
                validation.log.warning('Incomplete header information:' +
                                       self.path)


###############################################################################
###                         Function definitions                             ###
################################################################################

def create_plots():
    """
    This function starts a new process with the create_plots.py script in it
    to create the plots that include the results from this validation.
    """
    plot_creation = subprocess.Popen(['python', 'create_plots.py', '-f'],
                                     stdout=open(os.devnull, 'w'),
                                     stderr=open(os.devnull, 'w'))
    plot_creation.wait()


def filter_list_of_scripts(list_of_scripts, **kwargs):
    """
    This function receives a list of script objects and a filter (in
    **kwargs). It then searches through the given list and returns the subset
    which matches the filter.
    If given multiple filters, all filters except for the first one will be
    ignored. If you need multiple filters, consider nesting this function!

    Syntax:
    filter_list_of_scripts(desired_property=[list of acceptable values])

    Desired properties are all attributes of a Script object => See the
    __init__ method for Script objects.
    """

    if kwargs is not None:

        # Read in the filter and the values we are filtering for
        sieve, desired_values = kwargs.items()[0]

        # If we don't receive a list of desired values, make it a list!
        if not isinstance(desired_values, list):
            desired_values = [desired_values]

        # Holds the lists of matches we have found
        results = []

        for script_object in list_of_scripts:
            for value in desired_values:

                # If we are filtering for the path, package, or category
                if sieve.lower() in vars(script_object).keys():
                    needle = re.search(value,
                                       vars(script_object)[sieve.lower()])
                    if needle is not None:
                        results.append(script_object)

        return results

    else:
        return []


def find_creator(outputfile, package):
    """
    This function receives the name of a file and tries to find the file
    in the given package which produces this file, i.e. find the file in
    whose header 'outputfile' is listed under <output></output>.
    It then returns a list of all Scripts who claim to be creating 'outputfile'.
    """

    # Get a list of all Script objects for scripts in the given package as well
    # as from the validation-folder
    candidates = filter_list_of_scripts(validation.list_of_scripts,
                                        PACKAGE=package)
    candidates += filter_list_of_scripts(validation.list_of_scripts,
                                         PACKAGE='validation')

    # Reserve some space for the results we will return
    results = []

    # Loop over all candidates and check if they have 'outputfile' listed
    # under their outputs
    for candidate in candidates:
        if outputfile in candidate.header['output']:
            results.append(candidate)

    # Return our results and warn if there is more than one creator
    if len(results) == 0:
        return None
    if len(results) == 1:
        return results
    if len(results) > 1:
        validation.log.warning('Found multiple creators for' + outputfile)
        return results


def get_validation_folders(location):
    """
    Collects the validation folders for all packages from the stated release
    directory (either local or central). Returns a dict with the following form:
    {'name of package':'absolute path to validation folder of package'}
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
    """
    Sets up a parser for command line arguments, parses them and returns the
    arguments.
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
    """
    Returns all the files in the given dir (and its subdirs) that have
    the extension 'ext', if an extension is given (default: all extensions)
    """

    # Write to log what we are collecting
    validation.log.debug('Collecting *{0} files from {1}'.format(ext, dirpath))

    # Some space where we store our results before returning them
    results = []

    # A list of all folder names that will be ignored (e.g. folders that are
    # important for SCons
    blacklist = ['tools', 'scripts', 'examples']

    # Loop over the given directory and its subdirectories and find all files
    for root, dirs, files in os.walk(dirpath):

        # Skip a directory if it is blacklisted
        if os.path.basename(root) in blacklist:
            continue

        # Loop over all files
        for current_file in files:
            # If the file has the requested extension, append its full paths to
            # the results
            if current_file.lower().endswith(ext):
                results.append(root + '/' + current_file)

    # Return our sorted results
    return sorted(results)


def draw_progress_bar(delete_lines, barlength=50):
    """
    This function plots a progress bar of the validation, i.e. it shows which
    percentage of the scripts has been executed yet.
    It furthermore also shows which scripts are currently running, as well as
    the total runtime of the validation.
    """

    # Get statistics: Number of finished scripts + number of scripts in total
    finished_scripts = len([_ for _ in validation.list_of_scripts if
                            _.finished])
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
               if __.running]

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
        validation.complete_validation = False
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

    # Create a script object for every steering file we have collected
    validation.log.note('Creating Script objects...')
    validation.create_script_objects()

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
    create_plots()

    # Log that everything is finished
    validation.log.note('Plots have been created...')
    validation.log.note('Validation finished! Total runtime: {0}s'
                        .format(int(timeit.default_timer() - start)))

except KeyboardInterrupt:
    validation.log.note('Validation terminated by user!')
