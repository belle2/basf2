#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# basf2 specific imports
from basf2 import statistics
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
import ROOT

# Normal library imports
import math
import logging
import os
import timeit
import sys
import time
import shutil
import datetime

# Import the controls for local multiprocessing
import localcontrol

import json_objects

# A pretty printer. Prints prettier lists, dicts, etc. :)
import pprint
pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)

from validationscript import Script, ScriptStatus
from validationfunctions import get_start_time, get_validation_folders, scripts_in_dir, \
    parse_cmd_line_arguments
import validationfunctions

import validationserver
import validationplots
import validationscript
import validationpath

# local and cluster control backends
import localcontrol
import clustercontrol
import clustercontrolsge
import clustercontroldrmaa


def statistics_plots(
    fileName='',
    timingMethods=[statistics.INIT, statistics.EVENT],
    memoryMethods=[statistics.INIT, statistics.EVENT],
    contact='',
    jobDesc='',
    prefix='',
):
    """
    Add memory usage and execution time validation plots to the given root file.
    The current root file will be used if the fileName is empty (default).
    """

    # Open plot file
    save_dir = ROOT.gDirectory
    plotFile = None
    if fileName:
        plotFile = ROOT.TFile.Open(fileName, 'UPDATE')

    if not jobDesc:
        jobDesc = sys.argv[1]

    # Global timing
    methodName = {}
    hGlobalTiming = ROOT.TH1D(prefix + 'GlobalTiming', 'Global Timing', 5, 0, 5)
    hGlobalTiming.SetStats(0)
    hGlobalTiming.GetXaxis().SetTitle('method')
    hGlobalTiming.GetYaxis().SetTitle('time/call [ms]')
    hGlobalTiming.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Description',
            """The (average) time of the different basf2 execution phases for %s.
            The error bars show the rms of the time distributions.""" %
            jobDesc))
    hGlobalTiming.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            """There should be no significant and persistent increases in the the
            run time of the methods. Only cases where the increase compared to the
            reference or previous versions persists for at least two consecutive
            revisions should be reported since the measurements can be influenced
            by load from other processes on the execution host."""))
    if contact:
        hGlobalTiming.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
    for (index, method) in statistics.EStatisticCounters.values.items():
        methodName[method] = str(method)[0] \
            + str(method).lower()[1:].replace('_r', 'R')
        if index == 5:
            break
        hGlobalTiming.SetBinContent(index + 1, statistics.get_global().time_mean(method) * 1e-6)
        hGlobalTiming.SetBinError(index + 1, statistics.get_global().time_stddev(method) * 1e-6)
        hGlobalTiming.GetXaxis().SetBinLabel(index + 1, methodName[method])
    hGlobalTiming.Write()

    # Timing per module for the different methods
    modules = statistics.modules
    hModuleTiming = ROOT.TH1D(prefix + 'ModuleTiming', 'Module Timing',
                              len(modules), 0, len(modules))
    hModuleTiming.SetStats(0)
    hModuleTiming.GetXaxis().SetTitle('module')
    hModuleTiming.GetYaxis().SetTitle('time/call [ms]')
    hModuleTiming.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            """There should be no significant and persistent increases in the run
            time of a module. Only cases where the increase compared to the reference
            or previous versions persists for at least two consecutive revisions should
            be reported since the measurements can be influenced by load from other
            processes on the execution host."""))
    if contact:
        hModuleTiming.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
    for method in timingMethods:
        hModuleTiming.SetTitle('Module %s Timing' % methodName[method])
        hModuleTiming.GetListOfFunctions().Add(
            ROOT.TNamed(
                'Description',
                """The (average) execution time of the %s method of modules for %s.
                The error bars show the rms of the time distributions.""" %
                (methodName[method],
                 jobDesc)))
        index = 1
        for modstat in modules:
            hModuleTiming.SetBinContent(index, modstat.time_mean(method) * 1e-6)
            hModuleTiming.SetBinError(index, modstat.time_stddev(method) * 1e-6)
            hModuleTiming.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleTiming.Write('%s%sTiming' % (prefix, methodName[method]))
        hModuleTiming.GetListOfFunctions().RemoveLast()

    # Memory usage profile
    memoryProfile = ROOT.Belle2.PyStoreObj('VirtualMemoryProfile', 1)
    if memoryProfile:
        memoryProfile.obj().GetListOfFunctions().Add(ROOT.TNamed('Description',
                                                                 'The virtual memory usage vs. the event number for %s.' % jobDesc))
        memoryProfile.obj().GetListOfFunctions().Add(
            ROOT.TNamed(
                'Check',
                """The virtual memory usage should be flat for high event numbers. If it keeps rising this is an
                indication of a memory leak.<br>There should also be no significant increases with respect
                to the reference (or previous revisions if no reference exists)."""))
        if contact:
            memoryProfile.obj().GetListOfFunctions().Add(ROOT.TNamed('Contact',
                                                                     contact))
        memoryProfile.obj().Write(prefix + 'VirtualMemoryProfile')

    # Rss Memory usage profile
    memoryProfile = ROOT.Belle2.PyStoreObj('RssMemoryProfile', 1)
    if memoryProfile:
        memoryProfile.obj().GetListOfFunctions().Add(ROOT.TNamed('Description',
                                                                 'The rss memory usage vs. the event number for %s.' % jobDesc))
        memoryProfile.obj().GetListOfFunctions().Add(
            ROOT.TNamed(
                'Check',
                """The rss memory usage should be flat for high event numbers. If it keeps rising this is an
                indication of a memory leak.<br>There should also be no significant increases with respect
                to the reference (or previous revisions if no reference exists). In the (rare) case that
                memory is swapped by the OS, the rss memory usage can decrease."""))
        if contact:
            memoryProfile.obj().GetListOfFunctions().Add(ROOT.TNamed('Contact',
                                                                     contact))
        memoryProfile.obj().Write(prefix + 'RssMemoryProfile')

    # Memory usage per module for the different methods
    sqrtN = 1 / math.sqrt(statistics.get_global().calls() - 1)
    hModuleMemory = ROOT.TH1D(prefix + 'ModuleMemory', 'Virtual Module Memory',
                              len(modules), 0, len(modules))
    hModuleMemory.SetStats(0)
    hModuleMemory.GetXaxis().SetTitle('module')
    hModuleMemory.GetYaxis().SetTitle('memory increase/call [kB]')
    hModuleMemory.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Description', 'The (average) increase in virtual memory usage per call of the %s method of modules for %s.' %
            (methodName[method], jobDesc)))
    hModuleMemory.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            'The increase in virtual memory usage per call for each module should be consistent with zero or the reference.'))
    if contact:
        hModuleMemory.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
    for method in memoryMethods:
        hModuleMemory.SetTitle('Module %s Memory' % methodName[method])
        index = 1
        for modstat in modules:
            hModuleMemory.SetBinContent(index, modstat.memory_mean(method))
            hModuleMemory.SetBinError(index, modstat.memory_stddev(method) * sqrtN)
            hModuleMemory.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        hModuleMemory.Write('%s%sMemory' % (prefix, methodName[method]))
        hModuleMemory.GetListOfFunctions().RemoveLast()

    if plotFile:
        plotFile.Close()
    save_dir.cd()


def event_timing_plot(
    dataFile,
    fileName='',
    maxTime=20.0,
    burnIn=1,
    contact='',
    jobDesc='',
    prefix='',
):
    """
    Add a validation histogram of event execution time to the given root file.
    The current root file will be used if the fileName is empty (default).
    The data file has to contain the profile information created by the Profile
    module.
    """

    if not jobDesc:
        jobDesc = os.path.basename(sys.argv[0])

    # Get histogram with time vs event number
    save_dir = ROOT.gDirectory
    data = ROOT.TFile.Open(dataFile)
    tree = data.Get("tree")
    entries = tree.GetEntries()
    tree.Draw('Entry$>>hEventTime(%d,-0.5,%d.5)' % (entries, entries - 1),
              'ProfileInfo.m_timeInSec', 'goff')
    # load the histogram created by the above Draw command
    hEventTime = data.Get("hEventTime")
    hEventTime.SetDirectory(0)
    data.Close()
    save_dir.cd()

    # Open plot file
    plotFile = None
    if fileName:
        plotFile = ROOT.TFile.Open(fileName, 'UPDATE')

    # Create and fill histogram with event execution time distribution
    stat = ROOT.gStyle.GetOptStat()
    ROOT.gStyle.SetOptStat(101110)
    hTiming = ROOT.TH1D(prefix + 'Timing', 'Event Timing', 100, 0, maxTime)
    hTiming.UseCurrentStyle()
    hTiming.GetXaxis().SetTitle('time [s]')
    hTiming.GetYaxis().SetTitle('events')
    hTiming.GetListOfFunctions().Add(ROOT.TNamed('Description',
                                                 'The distribution of event execution times for %s.'
                                                 % jobDesc))
    hTiming.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            'The distribution should be consistent with the reference (or previous revisions if no reference exists).'))
    if contact:
        hTiming.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
    for event in range(1 + burnIn, entries + 1):
        hTiming.Fill(hEventTime.GetBinContent(event) - hEventTime.GetBinContent(event - 1))
    hTiming.Write()
    ROOT.gStyle.SetOptStat(stat)

    if plotFile:
        plotFile.Close()
    save_dir.cd()


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


class IntervalSelector:
    """
    This can be used to parse the execution intervals of validation scripts
    and can check whether a script object is in the list of intervals
    configured in this class.
    """

    def __init__(self, intervals):
        """
        Initialzes the IntervalSelector class with a list of intervals which
        should be selected
        """

        #: stores the intervals which have been selected
        self.intervals = [x.strip() for x in intervals]

    def in_interval(self, script_object):
        """
        checks whether the interval listed in a script object's header is
        within the selected
        """

        # for scripts, which have no interval set, the default is nightly
        script_interval = "nightly"

        if script_object.header is not None:
            if "interval" in script_object.header:
                script_interval = script_object.header["interval"]

        return script_interval in self.intervals


###############################################################################
#                              Class Definition                               #
###############################################################################


class Validation:

    """!
    This is the class that provides all global variables, like 'list_of_files'
    etc. There is only one instance of this class with name 'validation. This
    allows to use some kind of namespace, i.e. global variables will always be
    referenced as validation.[name of variable]. This makes it easier to
    distinguish them from local variables that only exist within the scope of a
    or a method.

    @var tag: The name of the folder within the results directory
    @var log: Reference to the logging object for this validation instance
    @var basepaths: The paths to the local and central release directory
    @var list_of_scripts: List of all Script objects for steering files
    @var list_of_packages: List of all packages which contributed scripts
    @var packages: The packages to be included in the validation (from cmd arg)
    @var basf2_options: The options to be given to the basf2 command
    @var mode: Whether to run locally or on a cluster
    @var quiet: No progress bar in quiet mode
    @var dry: Dry runs do not actually start any scripts (for debugging)
    """

    def __init__(self, tag='current'):
        """!
        The default constructor. Initializes all those variables that will be
        globally accessible later on. Does not return anything.
        """

        # The name which will be used to create a folder in the results
        # directory. Default is 'current'.
        self.tag = tag

        # This dictionary holds the paths to the local and central release dir
        # (or 'None' if one of them does not exist)
        self.basepaths = validationpath.get_basepath()

        # Folder used for the intermediate and final results of the validation
        self.work_folder = os.path.abspath(os.getcwd())

        # The logging-object for the validation (Instance of the logging-
        # module). Initialize the log as 'None' and then call the method
        # 'create_log()' to create the actual log.
        self.log = None
        self.create_log()

        # The list which holds all steering file objects
        # (as instances of class Script)
        self.list_of_scripts = []

        # A list of all packages from which we have collected steering files
        self.list_of_packages = []

        # The list of packages to be included in the validation. If we are
        # running a complete validation, this will be None.
        self.packages = None

        # This list of packages which will be ignored by default. This is only
        # the validation package itself, because it only creates test-plots
        # for validation development. To see only the validation-package output,
        # use the --test command line flag
        self.ignored_packages = ["validation-test"]

        # Additional arguments for basf2, if we received any from the command
        #  line arguments
        self.basf2_options = ''

        # A variable which holds the mode, i.e. 'local' for local
        #  multi-processing and 'cluster' for cluster usage
        self.mode = None

        # Defines whether the validation is run in quiet mode, i.e. without
        #  the dynamic progress bar
        self.quiet = False

        # Defines if a dry run is performed, i.e. a run where the steering
        #  files are not actually started (for debugging purposes)
        self.dry = False

        # If this is set, dependencies will be ignored.
        self.ignore_dependencies = False

        #: reporting time (in minutes)
        # the time in minutes when there will be there first logoutput if a script
        # is still not complete
        # This prints every 30 minutes which scripts are still running
        self.running_script_reporting_interval = 30

        # The maximum time before a script is skipped, if it does not terminate
        # The curren limit is 10h
        self.script_max_runtime_in_minutes = 720

    def get_useable_basepath(self):
        """
        Checks if a local path is available. If only a central release is available, return the path
        to this central release
        """
        if self.basepaths["local"]:
            return self.basepaths["local"]
        else:
            return self.basepaths["central"]

    @staticmethod
    def get_available_job_control():
        """
        insert the possible backend controls, they will be checed via their
        is_supported method if they actually can be executed in the current environment
        """
        return [localcontrol.Local,
                clustercontrol.Cluster,
                clustercontrolsge.Cluster,
                clustercontroldrmaa.Cluster]

    @staticmethod
    def get_available_job_control_names():
        return [c.name() for c in Validation.get_available_job_control()]

    def build_dependencies(self):
        """!
        This method loops over all Script objects in self.list_of_scripts and
        calls their compute_dependencies()-method.
        @return: None
        """
        for script_object in self.list_of_scripts:
            script_object.compute_dependencies(self.list_of_scripts)

        # The following code is only necessary while there are still a lot of
        # steering files without proper headers.
        # It adds all steering files from the validation-folder as a default
        # dependency, because a lot of scripts depend on one data script that
        # is created by a steering file in the validation-folder.
        default_depend = [script for script in self.list_of_scripts
                          if script.package == 'validation']
        for script_object in self.list_of_scripts:
            if not script_object.header and script_object.package != \
                    'validation':
                script_object.dependencies += default_depend

        # Make sure dependent scripts of skipped scripts are skipped, too.
        for script_object in self.list_of_scripts:
            if script_object.status == ScriptStatus.skipped:
                self.skip_script(script_object)

    def build_headers(self):
        """!
        This method loops over all Script objects in self.list_of_scripts and
        calls their load_header()-method.
        @return: None
        """
        for script_object in self.list_of_scripts:
            script_object.load_header()

    def skip_script(self, script_object):
        """!
        This method sets the status of the given script and all dependent ones
        to 'skipped'.
        @return: None
        """
        # Print a warning if the status of the script is changed and then
        # set it to 'skipped'.
        if script_object.status not in [ScriptStatus.skipped, ScriptStatus.failed]:
            self.log.warning('Skipping ' + script_object.path)
            script_object.status = ScriptStatus.skipped

        # Also skip all dependent scripts.
        for dependent_script in self.list_of_scripts:
            if script_object in dependent_script.dependencies:
                self.skip_script(dependent_script)

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
        # We define the new level and tell 'self.log' what to do when we use it
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
        log_dir = validationpath.get_results_tag_general_folder(self.work_folder, self.tag)
        if not os.path.exists(log_dir):
            print("Creating " + log_dir)
            os.makedirs(log_dir)

        # Define the handler and its level (=DEBUG to get everything)
        file_handler = logging.FileHandler(
            os.path.join(
                validationpath.get_results_tag_general_folder(self.work_folder,
                                                              self.tag), 'validate_basf2.log'), 'w+')
        file_handler.setLevel(logging.DEBUG)

        # Format the handler. We want the datetime, the module that produced
        # the message, the LEVEL of the message and the message itself
        file_format = logging.Formatter('%(asctime)s - %(module)s - '
                                        '%(levelname)s - %(message)s',
                                        datefmt='%Y-%m-%d %H:%M:%S')
        file_handler.setFormatter(file_format)

        # Add the file handler to self.log
        self.log.addHandler(file_handler)

    def collect_steering_files(self, intervalSelector):
        """!
        This function will collect all steering files from the local and
        central release directory and will store the corresponding paths in
        self.list_of_sf_paths.
        @return: None
        """

        # Get all folders that contain steering files, first the local ones
        validation_folders = get_validation_folders('local', self.basepaths, self.log)

        # Then add those central folders that do not have a local match
        for (package, folder) in get_validation_folders('central', self.basepaths, self.log).items():
            if package not in validation_folders:
                validation_folders[package] = folder

        # remove packages which have been explicitly ignored
        for ignored in self.ignored_packages:
            if ignored in validation_folders:
                del validation_folders[ignored]

        # Now write to self.list_of_packages which packages we have collected
        self.list_of_packages = list(validation_folders.keys())

        # Finally, we collect the steering files from each folder we have
        # collected:
        for (package, folder) in validation_folders.items():

            # Collect only *.C and *.py files
            c_files = scripts_in_dir(folder, self.log, '.C')
            py_files = scripts_in_dir(folder, self.log, '.py')
            for steering_file in c_files + py_files:
                script = Script(steering_file, package, self.log)

                script.load_header()
                # only select this script, if this interval has been selected
                if intervalSelector.in_interval(script):
                    self.list_of_scripts.append(script)

        # Thats it, now there is a complete list of all steering files on
        # which we are going to perform the validation in self.list_of_scripts

    def get_log_folder(self):
        """!
        Get the log folder for this validation run. The command log files (successful, failed)
        scripts will be recorded there
        """
        return validationpath.get_results_tag_folder(self.work_folder, self.tag)

    def log_failed(self):
        """!
        This method logs all scripts with property failed into a single file
        to be read in run_validation_server.py
        """

        # Open the log for writing
        list_failed = open(os.path.join(self.get_log_folder(), "list_of_failed_scripts.log"), "w+")

        # Select only failed scripts
        list_of_failed_scripts = [script for script in self.list_of_scripts if
                                  script.status == ScriptStatus.failed]

        # log the name of all failed scripts
        for script in list_of_failed_scripts:
            list_failed.write(script.path.split("/")[-1] + "\n")
        # close the log
        list_failed.close()

    def log_skipped(self):
        """!
        This method logs all scripts with property skipped into a single file
        to be read in run_validation_server.py
        """

        # Open the log for writing
        list_skipped = open(os.path.join(self.get_log_folder(), "list_of_skipped_scripts.log"), "w+")

        # Select only failed scripts
        list_of_skipped_scripts = [script for script in self.list_of_scripts if
                                   script.status == ScriptStatus.skipped]

        # log the name of all failed scripts
        for script in list_of_skipped_scripts:
            list_skipped.write(script.path.split("/")[-1] + "\n")
        # close the log
        list_skipped.close()

    def set_runtime_data(self):
        """!
        This method sets runtime property of each script.
        """
        # Set up temporary dict
        run_times = {}

        # Open the data
        runtimes = open(validationpath.get_results_runtime_file(self.work_folder), "r")

        # Get our data
        for line in runtimes:
            run_times[line.split("=")[0].strip()] = line.split("=")[1].strip()

        # And try to set a property for each script
        for script in self.list_of_scripts:
            try:
                script.runtime = float(run_times[script.name])
            # If we don't have runtime data, then set it to an average of all runtimes
            except KeyError:
                suma = 0.0
                for dict_key in run_times:
                    suma += float(run_times[dict_key])
                script.runtime = suma / len(run_times)
        runtimes.close()

    def get_script_by_name(self, name):
        """!

        """

        l = [s for s in self.list_of_scripts if s.name == name]
        if len(l) == 1:
            return l[0]
        else:
            return None

    def apply_package_selection(self, selected_packages, ignore_dependencies=False):
        """
        Only select packages from a specfic set of packages, but still honor the
        dependencies to outside scripts which may exist
        """

        to_keep_dependencies = set()

        # compile the dependencies of selected scripts
        # todo: won't work for nested dependencies
        if not ignore_dependencies:
            for script_obj in self.list_of_scripts:
                if script_obj.package in selected_packages:
                    for dep in script_obj.dependencies:
                        to_keep_dependencies.add(dep.unique_name())

        # now, remove all scripts from the script list, which are either
        # not in the selected packages or have a dependency to them
        self.list_of_scripts = [
            s for s in self.list_of_scripts if (
                s.package in selected_packages) or (
                s.unique_name() in to_keep_dependencies)]

    def apply_script_selection(self, script_selection, ignore_dependencies=False):
        """!
        This method will take the validation file name ( e.g. "FullTrackingValidation.py" ), determine
        all the script it depends on and set the status of theses scripts to "waiting",
        The status of all other scripts will be set to "skipped", which means they will not be executed
        in the validation run.  If ignore_dependencies is True, dependencies will also be set to "skipped".
        """

        # change file extension
        script_selection = [Script.sanitize_file_name(s) for s in script_selection]

        scripts_to_enable = set()

        # find the dependencies of each selected script
        for script in script_selection:
            scripts_to_enable.add(script)
            script_obj = self.get_script_by_name(script)

            if script_obj is None:
                self.log.error("Script with name {0} cannot be found, skipping for selection"
                               .format(script))
                continue

            others = script_obj.get_recursive_dependencies(self.list_of_scripts)
            if not ignore_dependencies:
                scripts_to_enable = scripts_to_enable.union(others)

        # enable all selections and dependencies
        for script_obj in self.list_of_scripts:
            if script_obj.name in scripts_to_enable:
                self.log.warning("Enabling script {0} because it was selected or a selected "
                                 "script depends on it."
                                 .format(script_obj.name))
                script_obj.status = ScriptStatus.waiting
            else:
                self.log.warning("Disabling script {0} because it was not "
                                 "selected."
                                 .format(script_obj.name))
                script_obj.status = ScriptStatus.skipped

    def apply_script_caching(self):
        cacheable_scripts = [s for s in self.list_of_scripts if s.is_cacheable()]

        output_dir_datafiles = validationpath.get_results_tag_folder(self.work_folder, self.tag)

        for s in cacheable_scripts:
            # for for all output files
            outfiles = s.get_output_files()
            files_exist = True
            for of in outfiles:
                full_path = os.path.join(output_dir_datafiles, of)
                files_exist = files_exist and os.path.isfile(full_path)

            if files_exist:
                s.status = ScriptStatus.cached

        # Remove all cached scripts from the dependencies
        # of dependent script objects, they will not be
        # executed and no one needs to wait for them
        for script in self.list_of_scripts:
            for dep_script in script.dependencies:
                # check if the script this one is depending on is
                # in cached execution
                if dep_script.status == ScriptStatus.cached:
                    script.dependencies.remove(dep_script)

    def store_run_results_json(self, git_hash):

        # retrieve the git hash which was used for executing this validation scripts
        json_package = []
        for p in self.list_of_packages:
            this_package_scrits = [s for s in self.list_of_scripts if s.package == p]
            json_scripts = [s.to_json(self.tag) for s in this_package_scrits]

            # count the failed scripts
            fail_count = sum([s.status == ScriptStatus.failed for s in this_package_scrits])
            json_package.append(json_objects.Package(p, scriptfiles=json_scripts, fail_count=fail_count))

        # todo: assign correct color here
        rev = json_objects.Revision(label=self.tag,
                                    creation_date=datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
                                    creation_timezone=validationfunctions.get_timezone(),
                                    packages=json_package,
                                    git_hash=git_hash)
        json_objects.dump(validationpath.get_results_tag_revision_file(self.work_folder, self.tag), rev)

    def add_script(self, script):
        """!
        Explictly add a script object. In normal operation, scripts are auto-discovered
        but this method is useful for testing
        """

        self.list_of_scripts.append(script)

    def sort_scripts(self, script_list):
        """
        Sort the list of scripts that have to be processed by runtime,
        execute slow scripts first
        If no runtime information is available from the last execution, run the scripts in the
        validation package first because they are log running and used as input for other scripts
        """
        script_list.sort(key=lambda x: x.runtime or x.package == "validation", reverse=True)

    def run_validation(self):
        """!
        This method runs the actual validation, i.e. it loops over all
        scripts, checks which of them are ready for execution, and runs them.
        @return: None
        """

        # Use the local execution for all plotting scripts
        local_control = localcontrol.\
            Local(max_number_of_processes=self.parallel)

        # Depending on the selected mode, load either the controls for the
        # cluster or for local multi-processing

        selected_control = [(c.name(), c.description(), c) for c in self.get_available_job_control() if c.name() == self.mode]

        if not len(selected_control) == 1:
            print("Selected mode {} does not exist".format(self.mode))
            sys.exit(1)

        self.log.note(selected_control[0][1])
        if not selected_control[0][2].is_supported():
            print("Selected mode {} is not supported on your system".format(self.mode))
            sys.exit(1)

        # instantiate the selected job control backend
        control = selected_control[0][2]()

        # read the git hash which is used to produce this validation
        src_basepath = self.get_useable_basepath()
        git_hash = validationfunctions.get_compact_git_hash(src_basepath)
        self.log.note("Git hash of repository located at {} is {}".format(src_basepath,
                                                                          git_hash))

        # If we do have runtime data, then read them
        if os.path.exists("./runtimes.dat") and os.stat("./runtimes.dat").st_size:
            self.set_runtime_data()
            if os.path.exists("./runtimes-old.dat"):
                # If there is an old data backup, delete it, we backup only one run
                os.remove("./runtimes-old.dat")
            if self.mode == "local":
                # Backup the old data file
                shutil.copyfile("./runtimes.dat", "./runtimes-old.dat")

        # Open runtimes log and start logging, but log only if we are
        # running in the local mode
        if self.mode == "local":
            runtimes = open('./runtimes.dat', 'w+')

        if not self.quiet:
            # This variable is needed for the progress bar function
            progress_bar_lines = 0
            print()

        # The list of scripts that have to be processed
        remaining_scripts = [script for script in self.list_of_scripts
                             if script.status == ScriptStatus.waiting]

        # Sort the list of scripts that have to be processed by runtime,
        # execute slow scripts first
        self.sort_scripts(remaining_scripts)

        # While there are scripts that have not yet been executed...
        while remaining_scripts:

            # Loop over all steering files / Script objects
            for script_object in remaining_scripts:

                # If the script is currently running
                if script_object.status == ScriptStatus.running:

                    # Check if the script has finished:
                    result = script_object.control.\
                        is_job_finished(script_object)

                    # If it has finished:
                    if result[0]:

                        # Write to log that the script finished
                        self.log.debug('Finished: ' + script_object.path)

                        # If we are running locally, log a runtime
                        script_object.runtime = time.time() - script_object.start_time
                        if self.mode == "local":
                            runtimes.write(script_object.name + "=" + str(script_object.runtime) + "\n")

                        # Check for the return code and set variables
                        # accordingly
                        script_object.status = ScriptStatus.finished
                        script_object.returncode = result[1]
                        if result[1] != 0:
                            script_object.status = ScriptStatus.failed
                            self.log.warning('exit_status was {0} for {1}'
                                             .format(result[1],
                                                     script_object.path))

                            # Skip all dependent scripts
                            self.skip_script(script_object)

                        else:
                            # Remove this script from the dependencies
                            # of dependent script objects
                            for dependent_script in remaining_scripts:
                                if script_object in \
                                   dependent_script.dependencies:
                                    dependent_script.dependencies.\
                                        remove(script_object)

                        # Some printout in quiet mode
                        if self.quiet:
                            waiting = [script for script in remaining_scripts
                                       if script.status == ScriptStatus.waiting]
                            running = [script for script in remaining_scripts
                                       if script.status == ScriptStatus.running]
                            print('Finished [{0},{1}]: {2} -> {3}'.format(
                                len(waiting), len(running),
                                script_object.path,
                                script_object.status))
                    else:

                        if (time.time() - script_object.last_report_time) / 60.0 > self.running_script_reporting_interval:
                            print(
                                "Script {} running since {} seconds".format(
                                    script_object.name_not_sanitized,
                                    time.time() - script_object.start_time))
                            # explicit flush so this will show up in log file right away
                            sys.stdout.flush()

                            # not finished yet, log time
                            script_object.last_report_time = time.time()

                        # check for the maximum time a script is allow to run and terminate if exceeded
                        total_runtime_in_minutes = (time.time() - script_object.start_time) / 60.0
                        if total_runtime_in_minutes > self.script_max_runtime_in_minutes:
                            script_object.status = ScriptStatus.failed
                            self.log.warning('Script {0} did not finish after {1} minutes, skipping '
                                             .format(script_object.path, total_runtime_in_minutes))
                            # kill the running process
                            script_object.control.terminate(script_object)
                            # Skip all dependent scripts
                            self.skip_script(script_object)

                # Otherwise (the script is waiting) and if it is ready to be
                # executed
                elif not script_object.dependencies:

                    # Determine the way of execution depending on whether
                    # data files are created
                    if script_object.header and \
                       script_object.header.get('output', []):
                        script_object.control = control
                    else:
                        script_object.control = local_control

                    # Do not spawn processes if there are already too many!
                    if script_object.control.available():

                        # Write to log which script is being started
                        self.log.debug('Starting ' + script_object.path)

                        # Set script object variables accordingly
                        if script_object.status == ScriptStatus.failed:
                            self.log.warning('Starting of {0} failed'.
                                             format(script_object.path))
                        else:
                            script_object.status = ScriptStatus.running

                        # Actually start the script execution
                        script_object.control.execute(script_object,
                                                      self.basf2_options,
                                                      self.dry, self.tag)

                        # Log the script execution start time
                        script_object.start_time = time.time()
                        script_object.last_report_time = time.time()

                        # Some printout in quiet mode
                        if self.quiet:
                            waiting = [_ for _ in remaining_scripts
                                       if _.status == ScriptStatus.waiting]
                            running = [_ for _ in remaining_scripts
                                       if _.status == ScriptStatus.running]
                            print('Started [{0},{1}]: {2}'.format(
                                len(waiting), len(running),
                                script_object.path))

            # Update the list of scripts that have to be processed
            remaining_scripts = [script for script in remaining_scripts if
                                 script.status in [ScriptStatus.waiting, ScriptStatus.running]]

            # Sort them again, Justin Case
            self.sort_scripts(remaining_scripts)

            # Wait for one second before starting again
            time.sleep(1)

            # If we are not running in quiet mode, draw the progress bar
            if not self.quiet:
                progress_bar_lines = draw_progress_bar(progress_bar_lines, self.list_of_scripts)

        # Log failed and skipped scripts
        self.log_failed()
        self.log_skipped()

        # And close the runtime data file
        if self.mode == "local":
            runtimes.close()
        print()

        self.store_run_results_json(git_hash)
        # todo: update list of available revisions with the current run

    def create_plots(self):
        """!
        This method prepares the html directory for the plots if necessary
        and creates the plots that include the results from this validation.
        @return: None
        """

        # Go to the html directory and create a link to the results folder
        # if it is not yet existing
        save_dir = os.getcwd()
        if not os.path.exists(validationpath.folder_name_html):
            os.mkdir(validationpath.folder_name_html)

        if not os.path.exists(validationpath.folder_name_results):
            self.log.error("Folder {} not found in the current directory {}, please run validate_basf2 first".format(
                           validationpath.folder_name_results,
                           save_dir))

        os.chdir(validationpath.folder_name_html)

        # import and run plot function
        validationplots.create_plots(force=True, work_folder=self.work_folder)

        # restore original working directory
        os.chdir(save_dir)


def execute(tag=None, isTest=None):
    """!
    Parses the comnmand line and executes the full validation suite
    """

    # If there is no release of basf2 set up, we can stop the execution right here!
    if os.environ.get('BELLE2_RELEASE', None) is None:
        sys.exit('Error: No basf2 release set up!')

    # Otherwise we can start the execution.
    # The mainpart is wrapped in a try/except-contruct to fetch keyboard interrupts
    try:

        # Now we process the command line arguments.
        # First of all, we read them in:
        cmd_arguments = parse_cmd_line_arguments(tag=tag, isTest=isTest,
                                                 modes=Validation.get_available_job_control_names())

        # overwrite with default settings with parameters give in method
        # call
        if tag is not None:
            cmd_arguments.tag = tag
        if isTest is not None:
            cmd_arguments.test = isTest

        # Create the validation object. 'validation' holds all global variables
        # and provides the logger!
        # Argument is the tag, i.e. the name which will be used for the folder
        # within the results directory to store the steering file outputs
        validation = Validation(cmd_arguments.tag)

        # Write to log that we have started the validation process
        validation.log.note('Starting validation...')
        validation.log.note('Results will stored in a folder named "{0}"...'.
                            format(validation.tag))

        # Now we check whether we are running a complete validation or only
        # validating a certain set of packages:
        if validation.packages:
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
        validation.mode = cmd_arguments.mode

        # Set if we have a limit on the maximum number of local processes
        validation.parallel = cmd_arguments.parallel

        # Check if we are running in quiet mode (no progress bar)
        if cmd_arguments.quiet:
            validation.quiet = True

        # Check if we are performing a dry run (don't actually start scripts)
        if cmd_arguments.dry:
            validation.dry = True

        # If running in test mode, only execute scripts in validation packgase
        if cmd_arguments.test:
            validation.ignored_packages = []
            validation.packages = ["validation-test"]
            validation.log.note('Running in test mode')

        validation.log.note("Release Folder: {} Local Folder:{}".format(validation.basepaths["central"],
                                                                        validation.basepaths["local"]))

        # Now collect the steering files which will be used in this validation.
        # This will fill validation.list_of_sf_paths with values.
        validation.log.note('Collecting steering files...')
        intervals = cmd_arguments.intervals.split(",")
        validation.collect_steering_files(IntervalSelector(intervals))

        # Build headers for every script object we have created
        validation.log.note('Building headers for Script objects...')
        validation.build_headers()

        # Build dependencies for every script object we have created, unless we're asked
        # to ignore them.
        if not cmd_arguments.select_ignore_dependencies:
            validation.log.note('Building dependencies for Script objects...')
            validation.build_dependencies()

        if cmd_arguments.packages:
            validation.packages = cmd_arguments.packages

        if validation.packages:
            validation.apply_package_selection(validation.packages)

        # select only specific scripts, if this option has been set
        if cmd_arguments.select:
            validation.log.note("Applying selection for validation scripts")
            validation.apply_script_selection(cmd_arguments.select,
                                              ignore_dependencies=False)

        # select only specific scripts and ignore their dependencies if option is set
        if cmd_arguments.select_ignore_dependencies:
            validation.log.note("Applying selection for validation scripts, "
                                "ignoring their dependencies")
            validation.apply_script_selection(cmd_arguments.select_ignore_dependencies,
                                              ignore_dependencies=True)

        # check if the scripts which are cacheable can be skipped, because their output
        # is already available
        if cmd_arguments.use_cache:
            validation.log.note("Checking for cached script output")
            validation.apply_script_caching()

        # Start the actual validation
        validation.log.note('Starting the validation...')
        validation.run_validation()

        # Log that the validation has finished and that we are creating plots
        validation.log.note('Validation finished...')
        if not validation.dry:
            validation.log.note('Start creating plots...')
            validation.create_plots()
            validation.log.note('Plots have been created...')
        else:
            validation.log.note('Skipping plot creation (dry run)...')

        # Log that everything is finished
        validation.log.note('Validation finished! Total runtime: {0}s'
                            .format(int(timeit.default_timer() - get_start_time())))

        if cmd_arguments.view:
            # run local webserver
            validationserver.run_server(openSite=True)

    except KeyboardInterrupt:
        validation.log.note('Validation terminated by user!')
