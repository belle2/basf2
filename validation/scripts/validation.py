#!/usr/bin/env python3

# basf2 specific imports
from basf2 import statistics
from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa
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
from typing import List, Optional

import json_objects
import mail_log

# A pretty printer. Prints prettier lists, dicts, etc. :)
import pprint

from validationscript import Script, ScriptStatus
from validationfunctions import get_start_time, get_validation_folders, \
    scripts_in_dir, parse_cmd_line_arguments, get_log_file_paths, \
    terminal_title_line
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


pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)


def statistics_plots(
    file_name='',
    timing_methods=None,
    memory_methds=None,
    contact='',
    job_desc='',
    prefix='',
):
    """
    Add memory usage and execution time validation plots to the given root
    file. The current root file will be used if the fileName is empty (
    default).
    """

    if not timing_methods:
        timing_methods = [statistics.INIT, statistics.EVENT]
    if not memory_methds:
        memory_methds = [statistics.INIT, statistics.EVENT]

    # Open plot file
    save_dir = ROOT.gDirectory
    plot_file = None
    if file_name:
        plot_file = ROOT.TFile.Open(file_name, 'UPDATE')

    if not job_desc:
        job_desc = sys.argv[1]

    # Global timing
    method_name = {}
    h_global_timing = ROOT.TH1D(
        prefix + 'GlobalTiming',
        'Global Timing',
        5, 0, 5
    )
    h_global_timing.SetStats(0)
    h_global_timing.GetXaxis().SetTitle('method')
    h_global_timing.GetYaxis().SetTitle('time/call [ms]')
    h_global_timing.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Description',
            """The (average) time of the different basf2 execution phases
            for {}. The error bars show the rms of the time
            distributions.""".format(job_desc)
        )
    )
    h_global_timing.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            """There should be no significant and persistent increases in
            the the run time of the methods. Only cases where the increase
            compared to the reference or previous versions persists for at
            least two consecutive revisions should be reported since the
            measurements can be influenced by load from other processes on
            the execution host."""
        )
    )
    if contact:
        h_global_timing.GetListOfFunctions().Add(
            ROOT.TNamed('Contact', contact)
        )
    for (index, method) in statistics.StatisticCounters.values.items():
        method_name[method] = str(method)[0] \
            + str(method).lower()[1:].replace('_r', 'R')
        if index == 5:
            break
        h_global_timing.SetBinContent(
            index + 1,
            statistics.get_global().time_mean(method) * 1e-6
        )
        h_global_timing.SetBinError(
            index + 1,
            statistics.get_global().time_stddev(method) * 1e-6
        )
        h_global_timing.GetXaxis().SetBinLabel(
            index + 1,
            method_name[method]
        )
    h_global_timing.Write()

    # Timing per module for the different methods
    modules = statistics.modules
    h_module_timing = ROOT.TH1D(
        prefix + 'ModuleTiming',
        'Module Timing',
        len(modules), 0, len(modules)
    )
    h_module_timing.SetStats(0)
    h_module_timing.GetXaxis().SetTitle('module')
    h_module_timing.GetYaxis().SetTitle('time/call [ms]')
    h_module_timing.GetListOfFunctions().Add(
        ROOT.TNamed(
            'Check',
            """There should be no significant and persistent increases in
            the run time of a module. Only cases where the increase compared
            to the reference or previous versions persists for at least two
            consecutive revisions should be reported since the measurements
            can be influenced by load from other processes on the execution
            host."""))
    if contact:
        h_module_timing.GetListOfFunctions().Add(ROOT.TNamed(
            'Contact', contact)
        )
    for method in timing_methods:
        h_module_timing.SetTitle('Module %s Timing' % method_name[method])
        h_module_timing.GetListOfFunctions().Add(
            ROOT.TNamed(
                'Description',
                """The (average) execution time of the %s method of modules
                for %s. The error bars show the rms of the time
                distributions.""" %
                (method_name[method],
                 job_desc)))
        index = 1
        for modstat in modules:
            h_module_timing.SetBinContent(
                index, modstat.time_mean(method) * 1e-6)
            h_module_timing.SetBinError(
                index, modstat.time_stddev(method) * 1e-6)
            h_module_timing.GetXaxis().SetBinLabel(
                index, modstat.name)
            index += 1
        h_module_timing.Write('{}{}Timing'.format(prefix, method_name[method]))
        h_module_timing.GetListOfFunctions().RemoveLast()

    # Memory usage profile
    memory_profile = ROOT.Belle2.PyStoreObj('VirtualMemoryProfile', 1)
    if memory_profile:
        memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
            'Description',
            f'The virtual memory usage vs. the event number for {job_desc}.')
        )
        memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
            'Check',
            """The virtual memory usage should be flat for high event
            numbers. If it keeps rising this is an indication of a memory
            leak.<br>There should also be no significant increases with
            respect to the reference (or previous revisions if no reference
            exists).""")
        )
        if contact:
            memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
                'Contact', contact)
            )
        memory_profile.obj().Write(prefix + 'VirtualMemoryProfile')

    # Rss Memory usage profile
    memory_profile = ROOT.Belle2.PyStoreObj('RssMemoryProfile', 1)
    if memory_profile:
        memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
            'Description',
            f'The rss memory usage vs. the event number for {job_desc}.')
        )
        memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
            'Check',
            """The rss memory usage should be flat for high event numbers.
            If it keeps rising this is an indication of a memory
            leak.<br>There should also be no significant increases with
            respect to the reference (or previous revisions if no reference
            exists). In the (rare) case that memory is swapped by the OS,
            the rss memory usage can decrease.""")
        )
        if contact:
            memory_profile.obj().GetListOfFunctions().Add(ROOT.TNamed(
                'Contact', contact)
            )
        memory_profile.obj().Write(prefix + 'RssMemoryProfile')

    # Memory usage per module for the different methods
    sqrt_n = 1 / math.sqrt(statistics.get_global().calls() - 1)
    h_module_memory = ROOT.TH1D(
        prefix + 'ModuleMemory',
        'Virtual Module Memory',
        len(modules), 0, len(modules)
    )
    h_module_memory.SetStats(0)
    h_module_memory.GetXaxis().SetTitle('module')
    h_module_memory.GetYaxis().SetTitle('memory increase/call [kB]')
    h_module_memory.GetListOfFunctions().Add(ROOT.TNamed(
        'Description',
        f'The (average) increase in virtual memory usage per call of the '
        f'{method_name[method]} method of modules for {job_desc}.')
    )
    h_module_memory.GetListOfFunctions().Add(ROOT.TNamed(
        'Check',
        'The increase in virtual memory usage per call for each module '
        'should be consistent with zero or the reference.')
    )
    if contact:
        h_module_memory.GetListOfFunctions().Add(ROOT.TNamed(
            'Contact', contact)
        )
    for method in memory_methds:
        h_module_memory.SetTitle('Module %s Memory' % method_name[method])
        index = 1
        for modstat in modules:
            h_module_memory.SetBinContent(index, modstat.memory_mean(method))
            h_module_memory.SetBinError(
                index,
                modstat.memory_stddev(method) * sqrt_n
            )
            h_module_memory.GetXaxis().SetBinLabel(index, modstat.name)
            index += 1
        h_module_memory.Write('{}{}Memory'.format(prefix, method_name[method]))
        h_module_memory.GetListOfFunctions().RemoveLast()

    if plot_file:
        plot_file.Close()
    save_dir.cd()


def event_timing_plot(
    data_file,
    file_name='',
    max_time=20.0,
    burn_in=1,
    contact='',
    job_desc='',
    prefix='',
):
    """
    Add a validation histogram of event execution time to the given root file.
    The current root file will be used if the fileName is empty (default).
    The data file has to contain the profile information created by the Profile
    module.
    """

    if not job_desc:
        job_desc = os.path.basename(sys.argv[0])

    # Get histogram with time vs event number
    save_dir = ROOT.gDirectory
    data = ROOT.TFile.Open(data_file)
    tree = data.Get("tree")
    entries = tree.GetEntries()
    tree.Draw('Entry$>>hEventTime(%d,-0.5,%d.5)' % (entries, entries - 1),
              'ProfileInfo.m_timeInSec', 'goff')
    # load the histogram created by the above Draw command
    h_event_time = data.Get("hEventTime")
    h_event_time.SetDirectory(0)
    data.Close()
    save_dir.cd()

    # Open plot file
    plot_file = None
    if file_name:
        plot_file = ROOT.TFile.Open(file_name, 'UPDATE')

    # Create and fill histogram with event execution time distribution
    stat = ROOT.gStyle.GetOptStat()
    ROOT.gStyle.SetOptStat(101110)
    h_timing = ROOT.TH1D(prefix + 'Timing', 'Event Timing', 100, 0, max_time)
    h_timing.UseCurrentStyle()
    h_timing.GetXaxis().SetTitle('time [s]')
    h_timing.GetYaxis().SetTitle('events')
    h_timing.GetListOfFunctions().Add(ROOT.TNamed(
        'Description',
        f'The distribution of event execution times for {job_desc}.')
    )
    h_timing.GetListOfFunctions().Add(ROOT.TNamed(
        'Check',
        'The distribution should be consistent with the reference (or '
        'previous revisions if no reference exists).')
    )
    if contact:
        h_timing.GetListOfFunctions().Add(ROOT.TNamed('Contact', contact))
    for event in range(1 + burn_in, entries + 1):
        h_timing.Fill(
            h_event_time.GetBinContent(event) -
            h_event_time.GetBinContent(event - 1)
        )
    h_timing.Write()
    ROOT.gStyle.SetOptStat(stat)

    if plot_file:
        plot_file.Close()
    save_dir.cd()


def draw_progress_bar(delete_lines: int, scripts: List[Script], barlength=50):
    """
    This function plots a progress bar of the validation, i.e. it shows which
    percentage of the scripts has been executed yet.
    It furthermore also shows which scripts are currently running, as well as
    the total runtime of the validation.

    @param delete_lines: The amount of lines which need to be deleted before
        we can redraw the progress bar
    @param scripts: List of all Script obejcts
    @param barlength: The length of the progess bar (in characters)
    @return: The number of lines that were printed by this function call.
        Usefule if this function is called repeatedly.
    """

    # Get statistics: Number of finished scripts + number of scripts in total
    finished_scripts = len([
        _ for _ in scripts
        if _.status in [
            validationscript.ScriptStatus.finished,
            validationscript.ScriptStatus.failed,
            validationscript.ScriptStatus.skipped
        ]
    ])
    all_scripts = len(scripts)
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
    print(
        f'\x1b[0G[{progressbar}] {percent:6.1f}% '
        f'({finished_scripts}/{all_scripts})')

    # Print the total runtime:
    print(f'Runtime: {runtime}s')

    # Print the list of currently running scripts:
    running = [os.path.basename(__.path) for __ in scripts
               if __.status == validationscript.ScriptStatus.running]

    # If nothing is repeatedly running
    if not running:
        running = ['-']

    print(f'Running: {running[0]}')
    for __ in running[1:]:
        print('{} {}'.format(len('Running:') * " ", __))

    return len(running) + 2


class IntervalSelector:
    """
    This can be used to parse the execution intervals of validation scripts
    and can check whether a script object is in the list of intervals
    configured in this class.
    """

    def __init__(self, intervals):
        """
        Initializes the IntervalSelector class with a list of intervals which
        should be selected
        """

        #: stores the intervals which have been selected
        self.intervals = [x.strip() for x in intervals]

    def in_interval(self, script_object: Script) -> bool:
        """
        checks whether the interval listed in a script object's header is
        within the selected
        """

        return script_object.interval in self.intervals


###############################################################################
#                              Class Definition                               #
###############################################################################


# todo: [Ref, low prio, low work] Denote private methods with underscore
#  /klieret
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
    @var scripts: List of all Script objects for steering files
    @var packages: List of all packages which contributed scripts
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
        self.log = self.create_log()

        # The list which holds all steering file objects
        # (as instances of class Script)
        self.scripts = []  # type: List[Script]

        # A list of all packages from which we have collected steering files
        self.packages = []  # type: List[str]

        # This list of packages which will be ignored by default. This is
        # only the validation package itself, because it only creates
        # test-plots for validation development. To see only the
        # validation-package output, use the --test command line flag
        self.ignored_packages = ["validation-test"]

        # Additional arguments for basf2, if we received any from the command
        #  line arguments
        self.basf2_options = ''

        # A variable which holds the mode, i.e. 'local' for local
        # multi-processing and 'cluster' for cluster usage
        self.mode = None

        # Defines whether the validation is run in quiet mode, i.e. without
        # the dynamic progress bar
        self.quiet = False

        # Defines if a dry run is performed, i.e. a run where the steering
        # files are not actually started (for debugging purposes)
        self.dry = False

        # If this is set, dependencies will be ignored.
        self.ignore_dependencies = False

        # : reporting time (in minutes)
        # the time in minutes when there will
        # be the first log output if a script is still not complete This
        # prints every x minutes which scripts are still running
        self.running_script_reporting_interval = 30

        #: The maximum time before a script is skipped, if it does not
        #: terminate. Unit: minutes. Set to <= 0 to skip this limitation
        #: entirely.
        self.script_max_runtime_in_minutes = 60 * 5

        #: Number of parallel processes
        self.parallel = None

    def get_useable_basepath(self):
        """
        Checks if a local path is available. If only a central release is
        available, return the path to this central release
        """
        if self.basepaths["local"]:
            return self.basepaths["local"]
        else:
            return self.basepaths["central"]

    @staticmethod
    def get_available_job_control():
        """
        insert the possible backend controls, they will be checed via their
        is_supported method if they actually can be executed in the current
        environment
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
        This method loops over all Script objects in self.scripts and
        calls their compute_dependencies()-method.
        @return: None
        """
        for script_object in self.scripts:
            script_object.compute_dependencies(self.scripts)

        # Make sure dependent scripts of skipped scripts are skipped, too.
        for script_object in self.scripts:
            if script_object.status == ScriptStatus.skipped:
                self.skip_script(
                    script_object,
                    reason=f"Depends on '{script_object.path}'"
                )

    def build_headers(self):
        """!
        This method loops over all Script objects in self.scripts and
        calls their load_header()-method.
        @return: None
        """
        for script_object in self.scripts:
            script_object.load_header()

    def skip_script(self, script_object, reason=""):
        """!
        This method sets the status of the given script and all dependent ones
        to 'skipped'.
        @param script_object: Script object to be skipped.
        @param reason: Reason for skipping object
        @return: None
        """
        # Print a warning if the status of the script is changed and then
        # set it to 'skipped'.
        if script_object.status not in [ScriptStatus.skipped,
                                        ScriptStatus.failed]:
            self.log.warning('Skipping ' + script_object.path)
            if reason:
                self.log.debug(f"Reason for skipping: {reason}.")
            script_object.status = ScriptStatus.skipped

        # Also skip all dependent scripts.
        for dependent_script in self.scripts:
            if script_object in dependent_script.dependencies:
                self.skip_script(
                    dependent_script,
                    reason=f"Depends on '{script_object.path}'"
                )

    def create_log(self) -> logging.Logger:
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
        log = logging.getLogger('validate_basf2')
        log.setLevel(logging.DEBUG)

        # Now we add another custom level 'NOTE'. This is because we don't
        # want to print ERRORs and WARNINGs to the console output, therefore
        # we need a higher level.
        # We define the new level and tell 'log' what to do when we use it
        logging.NOTE = 100
        logging.addLevelName(logging.NOTE, 'NOTE')
        log.note = lambda msg, *args: log._log(logging.NOTE,
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

        # Add the console handler to log
        log.addHandler(console_handler)

        # Now set up the file handler. The file handler will redirect
        # _everything_ we log to a logfile so that we have all possible
        # information available for debugging later.

        # Make sure the folder for the log file exists
        log_dir = self.get_log_folder()
        if not os.path.exists(log_dir):
            print("Creating " + log_dir)
            os.makedirs(log_dir)

        # Define the handler and its level (=DEBUG to get everything)
        file_handler = logging.FileHandler(
            os.path.join(log_dir, 'validate_basf2.log'),
            'w+'
        )
        file_handler.setLevel(logging.DEBUG)

        # Format the handler. We want the datetime, the module that produced
        # the message, the LEVEL of the message and the message itself
        file_format = logging.Formatter('%(asctime)s - %(module)s - '
                                        '%(levelname)s - %(message)s',
                                        datefmt='%Y-%m-%d %H:%M:%S')
        file_handler.setFormatter(file_format)

        # Add the file handler to log
        log.addHandler(file_handler)
        return log

    def collect_steering_files(self, interval_selector):
        """!
        This function will collect all steering files from the local and
        central release directory.
        @return: None
        """

        # Get all folders that contain steering files, first the local ones
        validation_folders = get_validation_folders(
            'local', self.basepaths, self.log)

        # Then add those central folders that do not have a local match
        for (package, folder) in get_validation_folders(
                'central', self.basepaths, self.log).items():
            if package not in validation_folders:
                validation_folders[package] = folder

        # remove packages which have been explicitly ignored
        for ignored in self.ignored_packages:
            if ignored in validation_folders:
                del validation_folders[ignored]

        # Now write to self.packages which packages we have collected
        self.packages = list(validation_folders.keys())

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
                if interval_selector.in_interval(script) and not script.noexecute:
                    self.scripts.append(script)

        # Thats it, now there is a complete list of all steering files on
        # which we are going to perform the validation in self.scripts

    def get_log_folder(self):
        """!
        Get the log folder for this validation run. The command log
        files (successful, failed) scripts will be recorded there
        """
        return validationpath.get_results_tag_folder(self.work_folder,
                                                     self.tag)

    def log_failed(self):
        """!
        This method logs all scripts with property failed into a single file
        to be read in run_validation_server.py
        """

        failed_log_path = os.path.join(
            self.get_log_folder(),
            "list_of_failed_scripts.log"
        )
        self.log.note(f"Writing list of failed scripts to {failed_log_path}.")

        # Select only failed scripts
        failed_scripts = [
            script for script in self.scripts
            if script.status == ScriptStatus.failed
        ]

        with open(failed_log_path, "w+") as list_failed:
            # log the name of all failed scripts
            for script in failed_scripts:
                list_failed.write(script.path.split("/")[-1] + "\n")

    def log_skipped(self):
        """!
        This method logs all scripts with property skipped into a single file
        to be read in run_validation_server.py
        """

        skipped_log_path = os.path.join(
            self.get_log_folder(),
            "list_of_skipped_scripts.log"
        )
        self.log.note(
            f"Writing list of skipped scripts to {skipped_log_path}."
        )

        # Select only failed scripts
        skipped_scripts = [
            script for script in self.scripts
            if script.status == ScriptStatus.skipped
        ]

        with open(skipped_log_path, "w+") as list_skipped:
            # log the name of all failed scripts
            for script in skipped_scripts:
                list_skipped.write(script.path.split("/")[-1] + "\n")

    def report_on_scripts(self):
        """!
        Print a summary about all scripts, especially highlighting
        skipped and failed scripts.
        """

        failed_scripts = [
            script.package + "/" + script.name for script in self.scripts
            if script.status == ScriptStatus.failed
        ]
        skipped_scripts = [
            script.package + "/" + script.name for script in self.scripts
            if script.status == ScriptStatus.skipped
        ]

        self.log.note("")
        self.log.note(terminal_title_line(
            "Summary of script execution", level=0
        ))
        self.log.note(f"Total number of scripts: {len(self.scripts)}")
        self.log.note("")
        if skipped_scripts:
            self.log.note("{}/{} scripts were skipped".format(
                len(skipped_scripts), len(self.scripts)))
            for s in skipped_scripts:
                self.log.note(f"* {s}")
            self.log.note("")
        else:
            self.log.note("No scripts were skipped. Nice!")
            self.log.note("")

        if failed_scripts:
            self.log.note("{}/{} scripts failed".format(
                len(failed_scripts), len(self.scripts)))
            for s in failed_scripts:
                self.log.note(f"* {s}")
            self.log.note("")
        else:
            self.log.note("No scripts failed. Nice!")
            self.log.note("")

        print(validationfunctions.congratulator(
            total=len(self.scripts),
            failure=len(failed_scripts) + len(skipped_scripts)
        ))

    def set_runtime_data(self):
        """!
        This method sets runtime property of each script.
        """

        run_times = {}
        path = validationpath.get_results_runtime_file(self.work_folder)
        with open(path) as runtimes:

            # Get our data
            for line in runtimes:
                run_times[line.split("=")[0].strip()] = \
                    line.split("=")[1].strip()

            # And try to set a property for each script
            for script in self.scripts:
                try:
                    script.runtime = float(run_times[script.name])
                # If we don't have runtime data, then set it to an average of
                # all runtimes
                except KeyError:
                    suma = 0.0
                    for dict_key in run_times:
                        suma += float(run_times[dict_key])
                    script.runtime = suma / len(run_times)

    def get_script_by_name(self, name: str) -> Optional[Script]:
        """!

        """

        l_arr = [s for s in self.scripts if s.name == name]
        if len(l_arr) == 1:
            return l_arr[0]
        else:
            return None

    def apply_package_selection(self, selected_packages,
                                ignore_dependencies=False):
        """!
        Only select packages from a specific set of packages, but still
        honor the dependencies to outside scripts which may exist
        """

        to_keep_dependencies = set()

        # compile the dependencies of selected scripts
        # todo: won't work for nested dependencies
        if not ignore_dependencies:
            for script_obj in self.scripts:
                if script_obj.package in selected_packages:
                    for dep in script_obj.dependencies:
                        to_keep_dependencies.add(dep.unique_name())
        # now, remove all scripts from the script list, which are either
        # not in the selected packages or have a dependency to them
        self.scripts = [
            s for s in self.scripts if (
                s.package in selected_packages) or (
                s.unique_name() in to_keep_dependencies)]

        # Check if some of the selected_packages were not found.
        packages = {s.package for s in self.scripts}
        packages_not_found = list(set(selected_packages) - packages)
        if packages_not_found:
            msg = "You asked to select the package(s) {}, but they were not " \
                  "found.".format(', '.join(packages_not_found))
            self.log.note(msg)
            self.log.warning(msg)

    def apply_script_selection(self, script_selection,
                               ignore_dependencies=False):
        """!
        This method will take the validation file name ( e.g.
        "FullTrackingValidation.py" ), determine all the script it depends on
        and set the status of theses scripts to "waiting", The status of all
        other scripts will be set to "skipped", which means they will not be
        executed in the validation run.  If ignore_dependencies is True,
        dependencies will also be set to "skipped".
        """

        # change file extension
        script_selection = [
            Script.sanitize_file_name(s) for s in script_selection
        ]

        scripts_to_enable = set()

        # find the dependencies of each selected script
        for script in script_selection:
            scripts_to_enable.add(script)
            script_obj = self.get_script_by_name(script)

            if script_obj is None:
                self.log.error(
                    f"Script with name {script} cannot be found, skipping for "
                    f"selection"
                )
                continue

            others = script_obj.get_recursive_dependencies(self.scripts)
            if not ignore_dependencies:
                scripts_to_enable = scripts_to_enable.union(others)

        # enable all selections and dependencies
        for script_obj in self.scripts:
            if script_obj.name in scripts_to_enable:
                self.log.warning(
                    f"Enabling script {script_obj.name} because it was "
                    f"selected or a selected script depends on it."
                )
                script_obj.status = ScriptStatus.waiting
            else:
                self.log.warning(
                    f"Disabling script {script_obj.name} because it was "
                    f"not selected."
                )
                script_obj.status = ScriptStatus.skipped

        # Check if some of the selected_packages were not found.
        script_names = {
            Script.sanitize_file_name(s.name) for s in self.scripts
        }
        scripts_not_found = set(script_selection) - script_names
        if scripts_not_found:
            msg = "You requested script(s) {}, but they seem to not have " \
                  "been found.".format(", ".join(scripts_not_found))
            self.log.note(msg)
            self.log.warning(msg)

    def apply_script_caching(self):
        cacheable_scripts = [s for s in self.scripts if s.is_cacheable]

        output_dir_datafiles = validationpath.get_results_tag_folder(
            self.work_folder, self.tag)

        for s in cacheable_scripts:
            # for for all output files
            outfiles = s.output_files
            files_exist = True
            for of in outfiles:
                full_path = os.path.join(output_dir_datafiles, of)
                files_exist = files_exist and os.path.isfile(full_path)

            if files_exist:
                s.status = ScriptStatus.cached

        # Remove all cached scripts from the dependencies
        # of dependent script objects, they will not be
        # executed and no one needs to wait for them
        for script in self.scripts:
            for dep_script in script.dependencies:
                # check if the script this one is depending on is
                # in cached execution
                if dep_script.status == ScriptStatus.cached:
                    script.dependencies.remove(dep_script)

    def store_run_results_json(self, git_hash):

        # retrieve the git hash which was used for executing this validation
        # scripts
        json_package = []
        for p in self.packages:
            this_package_scrits = [
                s for s in self.scripts if s.package == p
            ]
            json_scripts = [s.to_json(self.tag) for s in this_package_scrits]

            # count the failed scripts
            fail_count = sum([
                s.status == ScriptStatus.failed for s in this_package_scrits
            ])
            json_package.append(json_objects.Package(
                p,
                scriptfiles=json_scripts,
                fail_count=fail_count)
            )

        # todo: assign correct color here
        rev = json_objects.Revision(
            label=self.tag,
            creation_date=datetime.datetime.now().strftime("%Y-%m-%d %H:%M"),
            creation_timezone=validationfunctions.get_timezone(),
            packages=json_package,
            git_hash=git_hash
        )
        json_objects.dump(
            validationpath.get_results_tag_revision_file(
                self.work_folder, self.tag
            ),
            rev
        )

    def add_script(self, script: Script):
        """!
        Explictly add a script object. In normal operation, scripts are
        auto-discovered but this method is useful for testing
        """

        self.scripts.append(script)

    @staticmethod
    def sort_scripts(script_list: List[Script]):
        """
        Sort the list of scripts that have to be processed by runtime,
        execute slow scripts first If no runtime information is available
        from the last execution, run the scripts in the validation package
        first because they are log running and used as input for other scripts
        """
        script_list.sort(
            key=lambda x: x.runtime or x.package == "validation",
            reverse=True
        )

    # todo: if you have to indent by 9 tabs, you know that it's time to refactor /klieret
    def run_validation(self):
        """!
        This method runs the actual validation, i.e. it loops over all
        scripts, checks which of them are ready for execution, and runs them.
        @return: None
        """

        # Use the local execution for all plotting scripts
        self.log.note("Initializing local job control for plotting.")
        local_control = localcontrol.\
            Local(max_number_of_processes=self.parallel)

        # Depending on the selected mode, load either the controls for the
        # cluster or for local multi-processing

        self.log.note("Selecting job control for all other jobs.")

        selected_controls = [
            c for c in self.get_available_job_control()
            if c.name() == self.mode
        ]

        if not len(selected_controls) == 1:
            print(f"Selected mode {self.mode} does not exist")
            sys.exit(1)

        selected_control = selected_controls[0]

        self.log.note("Controller: {} ({})".format(
            selected_control.name(),
            selected_control.description()
        ))

        if not selected_control.is_supported():
            print(f"Selected mode {self.mode} is not supported on your system")
            sys.exit(1)

        # instantiate the selected job control backend
        if selected_control.name() == "local":
            control = selected_control(max_number_of_processes=self.parallel)
        else:
            control = selected_control()

        # read the git hash which is used to produce this validation
        src_basepath = self.get_useable_basepath()
        git_hash = validationfunctions.get_compact_git_hash(src_basepath)
        self.log.debug(
            f"Git hash of repository located at {src_basepath} is {git_hash}"
        )

        # todo: perhaps we want to have these files in the results folder, don't we? /klieret
        # If we do have runtime data, then read them
        if os.path.exists("./runtimes.dat") and \
                os.stat("./runtimes.dat").st_size:
            self.set_runtime_data()
            if os.path.exists("./runtimes-old.dat"):
                # If there is an old data backup, delete it, we backup only
                # one run
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
        remaining_scripts = [script for script in self.scripts
                             if script.status == ScriptStatus.waiting]

        # Sort the list of scripts that have to be processed by runtime,
        # execute slow scripts first
        self.sort_scripts(remaining_scripts)

        def handle_finished_script(script_obj: Script):
            # Write to log that the script finished
            self.log.debug('Finished: ' + script_obj.path)

            # If we are running locally, log a runtime
            script_obj.runtime = time.time() - script_obj.start_time
            if self.mode == "local":
                runtimes.write(script_obj.name + "=" +
                               str(script_obj.runtime) + "\n")

            # Check for the return code and set variables accordingly
            script_obj.status = ScriptStatus.finished
            script_obj.returncode = result[1]
            if result[1] != 0:
                script_obj.status = ScriptStatus.failed
                self.log.warning(
                    f'exit_status was {result[1]} for {script_obj.path}'
                )
                script_obj.remove_output_files()

                # Skip all dependent scripts
                self.skip_script(
                    script_obj,
                    reason="Script '{}' failed and we set it's status to "
                           "skipped so that all dependencies are "
                           "also skipped.".format(script_object.path)
                )

            else:
                # Remove this script from the dependencies of dependent
                # script objects
                for dependent_script in remaining_scripts:
                    if script_obj in dependent_script.dependencies:
                        dependent_script.dependencies.remove(script_obj)

            # Some printout in quiet mode
            if self.quiet:
                waiting = [script for script in remaining_scripts
                           if script.status == ScriptStatus.waiting]
                running = [script for script in remaining_scripts
                           if script.status == ScriptStatus.running]
                print(
                    'Finished [{},{}]: {} -> {}'.format(
                        len(waiting),
                        len(running),
                        script_obj.path,
                        script_obj.status
                    )
                )

        def handle_unfinished_script(script_obj: Script):
            if (time.time() - script_obj.last_report_time) / 60.0 > \
                    self.running_script_reporting_interval:
                print(
                    "Script {} running since {} seconds".format(
                        script_obj.name_not_sanitized,
                        time.time() - script_obj.start_time))
                # explicit flush so this will show up in log file right away
                sys.stdout.flush()

                # not finished yet, log time
                script_obj.last_report_time = time.time()

            # check for the maximum time a script is allow to run and
            # terminate if exceeded
            total_runtime_in_minutes = \
                (time.time() - script_obj.start_time) / 60.0
            if total_runtime_in_minutes > self.script_max_runtime_in_minutes > 0:
                script_obj.status = ScriptStatus.failed
                self.log.warning(
                    f'Script {script_obj.path} did not finish after '
                    f'{total_runtime_in_minutes} minutes, attempting to '
                    f'terminate. '
                )
                # kill the running process
                script_obj.control.terminate(script_obj)
                # Skip all dependent scripts
                self.skip_script(
                    script_obj,
                    reason=f"Script '{script_object.path}' did not finish in "
                           f"time, so we're setting it to 'failed' so that all "
                           f"dependent scripts will be skipped."
                )

        def handle_waiting_script(script_obj: Script):
            # Determine the way of execution depending on whether
            # data files are created
            if script_obj.output_files:
                script_obj.control = control
            else:
                script_obj.control = local_control

            # Do not spawn processes if there are already too many!
            if script_obj.control.available():

                # Write to log which script is being started
                self.log.debug('Starting ' + script_obj.path)

                # Set script object variables accordingly
                if script_obj.status == ScriptStatus.failed:
                    self.log.warning(
                        f'Starting of {script_obj.path} failed'
                    )
                else:
                    script_obj.status = ScriptStatus.running

                # Actually start the script execution
                script_obj.control.execute(
                    script_obj,
                    self.basf2_options,
                    self.dry,
                    self.tag
                )

                # Log the script execution start time
                script_obj.start_time = time.time()
                script_obj.last_report_time = time.time()

                # Some printout in quiet mode
                if self.quiet:
                    waiting = [_ for _ in remaining_scripts
                               if _.status == ScriptStatus.waiting]
                    running = [_ for _ in remaining_scripts
                               if _.status == ScriptStatus.running]
                    print('Started [{},{}]: {}'.format(
                        len(waiting), len(running),
                        script_obj.path)
                    )

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
                        handle_finished_script(script_object)
                    else:
                        handle_unfinished_script(script_object)

                # Otherwise (the script is waiting) and if it is ready to be
                # executed
                elif not script_object.dependencies:
                    handle_waiting_script(script_object)

            # Update the list of scripts that have to be processed
            remaining_scripts = [
                script for script in remaining_scripts
                if script.status in [ScriptStatus.waiting,
                                     ScriptStatus.running]
            ]

            # Sort them again, Justin Case
            self.sort_scripts(remaining_scripts)

            # Wait for one second before starting again
            time.sleep(1)

            # If we are not running in quiet mode, draw the progress bar
            if not self.quiet:
                progress_bar_lines = draw_progress_bar(
                    progress_bar_lines,
                    self.scripts
                )

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

        html_folder = validationpath.get_html_folder(self.work_folder)
        results_folder = validationpath.get_results_folder(
            self.work_folder
        )

        os.makedirs(html_folder, exist_ok=True)

        if not os.path.exists(results_folder):
            self.log.error(
                f"Folder {results_folder} not found in "
                f"the work directory {self.work_folder}, please run "
                f"b2validation first"
            )

        validationplots.create_plots(force=True, work_folder=self.work_folder)


def execute(tag=None, is_test=None):
    """!
    Parses the command line and executes the full validation suite
    :param tag The name that will be used for the current revision.
        Default None means automatic.
    :param is_test Run in test mode? Default None means that we read this
        from the command line arguments (which default to False).
    :returns None
    """

    # Note: Do not test tag and is_test, but rather cmd_arguments.tag
    # and cmd_arguments.is_test!
    # Also note that we modify some cmd_arguments below
    # (e.g. cmd_arguments.packages is updated if cmd_arguments.test is
    # specified).

    # If there is no release of basf2 set up, we can stop the execution
    # right here!
    if os.environ.get('BELLE2_RELEASE_DIR', None) is None and os.environ.get('BELLE2_LOCAL_DIR', None) is None:
        sys.exit('Error: No basf2 release set up!')

    # Otherwise we can start the execution. The mainpart is wrapped in a
    # try/except-contruct to fetch keyboard interrupts
    # fixme: except instructions make only sense after Validation obj is
    #   initialized ==> Pull everything until there out of try statement
    try:

        # Now we process the command line arguments.
        # First of all, we read them in:
        cmd_arguments = parse_cmd_line_arguments(
            modes=Validation.get_available_job_control_names()
        )

        # overwrite with default settings with parameters give in method
        # call
        if tag is not None:
            cmd_arguments.tag = tag
        if is_test is not None:
            cmd_arguments.test = is_test

        # Create the validation object.
        validation = Validation(cmd_arguments.tag)

        # Write to log that we have started the validation process
        validation.log.note('Starting validation...')
        validation.log.note(
            f'Results will stored in a folder named "{validation.tag}"...')
        validation.log.note('The (full) log file(s) can be found at {}'.format(
            ', '.join(get_log_file_paths(validation.log))
        ))
        validation.log.note("Please check these logs when encountering "
                            "unexpected results, as most of the warnings and "
                            "errors are not written to stdout/stderr.")

        # Check if we received additional arguments for basf2
        if cmd_arguments.options:
            validation.basf2_options = ' '.join(cmd_arguments.options)
            validation.log.note(
                f'Received arguments for basf2: {validation.basf2_options}'
            )

        # Check if we are using the cluster or local multiprocessing:
        validation.mode = cmd_arguments.mode

        # Set if we have a limit on the maximum number of local processes
        validation.parallel = cmd_arguments.parallel

        # Check if we are running in quiet mode (no progress bar)
        if cmd_arguments.quiet:
            validation.log.note("Running in quiet mode (no progress bar).")
            validation.quiet = True

        # Check if we are performing a dry run (don't actually start scripts)
        if cmd_arguments.dry:
            validation.log.note("Performing a dry run; no scripts will be "
                                "started.")
            validation.dry = True

        # If running in test mode, only execute scripts in validation packgase
        if cmd_arguments.test:
            validation.log.note('Running in test mode')
            validation.ignored_packages = []
            cmd_arguments.packages = ["validation-test"]

        validation.log.note(
            "Release Folder: {}".format(validation.basepaths["central"])
        )
        validation.log.note(
            "Local Folder: {}".format(validation.basepaths["local"])
        )

        # Now collect the steering files which will be used in this validation.
        validation.log.note('Collecting steering files...')
        intervals = cmd_arguments.intervals.split(",")
        validation.collect_steering_files(IntervalSelector(intervals))

        # Build headers for every script object we have created
        validation.log.note('Building headers for Script objects...')
        validation.build_headers()

        # Build dependencies for every script object we have created,
        # unless we're asked to ignore them.
        if not cmd_arguments.select_ignore_dependencies:
            validation.log.note('Building dependencies for Script objects...')
            validation.build_dependencies()

        if cmd_arguments.packages:
            validation.log.note(
                "Applying package selection for the following package(s): " +
                ", ".join(cmd_arguments.packages)
            )
            validation.apply_package_selection(cmd_arguments.packages)

        # select only specific scripts, if this option has been set
        if cmd_arguments.select:
            validation.log.note("Applying selection for validation scripts")
            validation.apply_script_selection(cmd_arguments.select,
                                              ignore_dependencies=False)

        # select only specific scripts and ignore their dependencies if
        # option is set
        if cmd_arguments.select_ignore_dependencies:
            validation.log.note("Applying selection for validation scripts, "
                                "ignoring their dependencies")
            validation.apply_script_selection(
                cmd_arguments.select_ignore_dependencies,
                ignore_dependencies=True
            )

        # check if the scripts which are cacheable can be skipped, because
        # their output is already available
        if cmd_arguments.use_cache:
            validation.log.note("Checking for cached script output")
            validation.apply_script_caching()

        # Allow to change the maximal run time of the scripts
        if cmd_arguments.max_run_time is not None:
            if cmd_arguments.max_run_time > 0:
                validation.log.note(
                    f"Setting maximal run time of the steering files "
                    f"to {cmd_arguments.max_run_time} minutes."
                )
            else:
                validation.log.note(
                    "Disabling run time limitation of steering files as "
                    "requested (max run time set to <= 0)."
                )
            validation.script_max_runtime_in_minutes = \
                cmd_arguments.max_run_time

        # Start the actual validation
        validation.log.note('Starting the validation...')
        validation.run_validation()

        # Log that the validation has finished and that we are creating plots
        validation.log.note('Validation finished...')
        if not validation.dry:
            validation.log.note('Start creating plots...')
            validation.create_plots()
            validation.log.note('Plots have been created...')
            # send mails
            if cmd_arguments.send_mails:
                mails = mail_log.Mails(validation)
                validation.log.note('Start sending mails...')
                # send mails to all users with failed scripts/comparison
                if cmd_arguments.send_mails_mode == "incremental":
                    incremental = True
                elif cmd_arguments.send_mails_mode == "full":
                    incremental = False
                else:
                    incremental = None
                mails.send_all_mails(
                    incremental=incremental
                )
                validation.log.note(
                    'Save mail data to {}'.format(
                        validation.get_log_folder()
                    )
                )
                # save json with data about outgoing mails
                mails.write_log()
        else:
            validation.log.note('Skipping plot creation and mailing '
                                '(dry run)...')

        validation.report_on_scripts()

        # Log that everything is finished
        validation.log.note(
            'Validation finished! Total runtime: {}s'.format(
                int(timeit.default_timer() - get_start_time())
            )
        )

        if cmd_arguments.view:
            # run local webserver
            validationserver.run_server(open_site=True)

    except KeyboardInterrupt:
        validation.log.note('Validation terminated by user!')
