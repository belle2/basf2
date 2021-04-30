#!/usr/bin/env python3

import re
import os
from typing import Optional, List
import logging

# Import XML Parser. Use C-Version, if available
try:
    import xml.etree.cElementTree as XMLTree
except ImportError:
    import xml.etree.ElementTree as XMLTree

import json_objects


# todo [code quality, low prio, easy]: This should be an enum
class ScriptStatus:

    """!
    Enumeration of the states a script can be during its execution
    cycle
    """

    # script is waiting for execution
    class waiting:
        pass

    # script is running atm
    class running:
        pass

    # script execution has been successfully finished
    class finished:
        pass

    # script execution has failed
    class failed:
        pass

    # script has been marked to be skipped
    class skipped:
        pass

    # script output is already cached, do not execute script
    class cached:
        pass


class Script:

    """!
    The object representation of a steering file.

    @var path: The path to the steering file
    @var name: The name of the file, but without special chars or spaces
    @var package: The package to which the steering file belongs to
    @var header: The contents of the XML file header
    @var dependencies: On which scripts does the steering file depend
    @var status: The current status, e.g. 'running' or 'finished'
    @var control: Execute locally or on the cluster?
    @var returncode: The returncode of the steering file
    @var _object: Pointer to the object itself. Is this even necessary?
    """

    def __init__(self, path: str, package: str, log: Optional[logging.Logger]):
        """!
        The default constructor.
        """

        # Pointer to the script object itself
        # Is this necessary?
        self._object = self

        # stores the reference to the logging object used in this validation
        # run
        self.log = log

        # The (absolute) path of the steering file
        self.path = path

        # The runtime of the script
        self.runtime = None  # type: Optional[int]
        self.start_time = None  # type: Optional[int]

        # The name of the steering file. Basically the file name of the
        # steering file, but everything that is not a letter is replaced
        # by an underscore. Useful e.g. for cluster controls.
        self.name = self.sanitize_file_name(str(os.path.basename(self.path)))
        # useful when displaying the filename to the user
        self.name_not_sanitized = str(os.path.basename(self.path))

        # The package to which the steering file belongs
        self.package = package

        #: The information from the file header
        self._header = dict()
        #: Set if we tried to parse this but there were issues during parsing
        self.header_parsing_errors = False
        #: Set to true once header was parsed
        self._header_parsing_attempted = False

        # A list of script objects, on which this script depends
        self.dependencies = []

        # Current status of the script.
        # Possible values: waiting, running, finished, failed, skipped
        self.status = ScriptStatus.waiting

        # Which control is used for executing the script, i.e. cluster or
        # local. Useful when using different script level, e.g. data creation
        # scripts are being run on the cluster, but plotting scripts are
        # executed locally
        self.control = None

        # The returncode of the script. Should be 0 if all went well.
        self.returncode = None  # type: Optional[int]

        #: Id of job for job submission. This is set by some of the
        #: cluster controls in order to terminate the job if it exceeds the
        #: runtime.
        self.job_id = None  # type: Optional[str]

    @staticmethod
    def sanitize_file_name(file_name):
        """!
        Replaces the . between the file name and extension with an underscore _
        """
        return re.sub(r'[\W_]+', '_', file_name)

    def to_json(self, current_tag):

        string_status = ""

        if self.status == ScriptStatus.failed:
            string_status = "failed"
        elif self.status == ScriptStatus.finished:
            string_status = "finished"
        elif self.status == ScriptStatus.running:
            string_status = "running"
        elif self.status == ScriptStatus.skipped:
            string_status = "skipped"
        elif self.status == ScriptStatus.waiting:
            string_status = "waiting"
        elif self.status == ScriptStatus.cached:
            string_status = "cached"

        return json_objects.Script(
            self.name_not_sanitized,
            self.path,
            string_status,
            log_url=os.path.join(self.package, self.name_not_sanitized) +
            ".log",
            return_code=self.returncode
        )

    def get_recursive_dependencies(self, scripts, level=0):
        """!
        Loops over all dependencies of this script and recursively retrieves
        their sub-dependencies
        """

        if level > 50:
            self.log.error(
                f'Recurisve dependency lookup reached level {level} and will '
                f'quit now. Possibly circular dependcencies in the validation '
                f'scripts ? '
            )

        all_deps = set()
        for dep in self.dependencies:
            # only add, if not already in the dependencies list
            all_deps.add(dep.name)

            next_level = level + 1

            # find script object
            dep_script = [x for x in scripts if x.name == dep.name]
            rec_deps = []
            if len(dep_script) == 1:
                rec_deps = dep_script[0].get_recursive_dependencies(
                    scripts, next_level)
            else:
                self.log.error(
                    f'Depending script with the name {dep.name} could not be '
                    f'found in the list of registered scripts. '
                )

            # only add, if not already in the dependencies list
            for rc in rec_deps:
                all_deps.add(rc)

        return all_deps

    def unique_name(self):
        """
        Generates a unique name from the package and name of the script
        which only occurs once in th whole validation suite
        """
        return f"script_unique_name_{self.package}_{self.name}"

    def compute_dependencies(self, scripts):
        """!
        Loops over the input files given in the header and tries to find the
        corresponding Script objects, which will then be stored in the
        script.dependencies-list
        @return: None
        """
        # Loop over all the dependencies given in the header information
        for root_file in self.input_files:

            # Find the script which is responsible for the creation of
            # the input file (in the same package or in validation folder)
            creator = find_creator(
                root_file,
                self.package,
                scripts,
                self.log
            )

            # If no creator could be found, raise an error!
            if creator is None:
                self.log.error(
                    f'Unmatched dependency for {self.path}: {root_file} '
                    f'has no creator! This means that we will have to skip '
                    f'this script.')
                self.status = ScriptStatus.skipped

            # If creator(s) could be found, add those scripts to the
            # list of scripts on which self depends
            else:
                self.dependencies += creator

        # remove double entries
        self.dependencies = list(set(self.dependencies))

    def load_header(self):
        """!
        This method opens the file given in self.path, tries to extract the
        XML-header of it and then parse it.
        It then fills the self.header variable with a dict containing the
        values that were read from the XML header.
        @return: None
        """
        if self._header_parsing_attempted:
            return

        self._header_parsing_attempted = True

        # Read the file as a whole
        # We specify encoding and errors here to avoid exceptions for people
        # with strange preferred encoding settings in their OS
        with open(self.path, encoding="utf-8", errors="replace") as data:
            steering_file_content = data.read()

        # Define the regex to extract everything between the <header>-tags
        pat = re.compile('(<header>.*?</header>)', re.DOTALL | re.M)

        # Apply the regex, i.e. filter out the <header>...</header> part of
        # each steering file.
        try:
            xml = pat.findall(steering_file_content)[0].strip()
        except IndexError:
            self.log.error('No file header found: ' + self.path)
            self.header_parsing_errors = True
            return

        # Create an XML tree from the plain XML code.
        try:
            xml_tree = XMLTree.ElementTree(XMLTree.fromstring(xml)).getroot()
        except XMLTree.ParseError:
            self.log.error('Invalid XML in header: ' + self.path)
            self.header_parsing_errors = True
            return

        # we have a header
        self._header = {}

        # Loop over that tree
        for branch in xml_tree:

            # The keywords that should be parsed into a list
            list_tags = ['input', 'output', 'contact']

            # If the tag is empty branch.text is None. Replacing None with an
            # empty string in this case.
            branch_text = branch.text or ""

            # Format the values of each branch
            if branch.tag.strip() in list_tags:
                branch_value = [__.strip() for __ in branch_text.split(',')]
                if branch_value == ['']:
                    branch_value = []
            else:
                branch_value = re.sub(' +', ' ', branch_text.replace('\n', ''))
                branch_value = branch_value.strip()

            # Append the branch and its values to the header-dict. This
            # implementation technically allows multiple occurrences of the
            # same <tag></tag>-pair, which will be bundled to the same key in
            # the key in the returned dictionary
            if branch.tag.strip() in self._header:
                self._header[branch.tag.strip()] += branch_value
            else:
                self._header[branch.tag.strip()] = branch_value

    # Below are all of the getter methods for accessing data from the header
    # If the header isn't loaded at the time they are called, we do that.

    @property
    def input_files(self):
        """
        return a list of input files which this script will read.
        This information is only available, if load_header has been called
        """
        self.load_header()
        return self._header.get('input', [])

    @property
    def output_files(self):
        """
        return a list of output files this script will create.
        This information is only available, if load_header has been called
        """
        self.load_header()
        return self._header.get('output', [])

    @property
    def is_cacheable(self):
        """
        Returns true, if the script must not be executed if its output
        files are already present.
        This information is only available, if load_header has been called
        """
        self.load_header()
        return 'cacheable' in self._header

    @property
    def noexecute(self) -> bool:
        """ A flag set in the header that tells us to simply ignore this
        script for the purpose of running the validation.
        """
        self.load_header()
        return "noexecute" in self._header

    @property
    def description(self) -> str:
        """ Description of script as set in header """
        self.load_header()
        return self._header.get("description", "")

    @property
    def contact(self) -> str:
        """ Contact of script as set in header """
        self.load_header()
        return self._header.get("contact", "")

    @property
    def interval(self) -> str:
        """ Interval of script executation as set in header """
        self.load_header()
        return self._header.get("interval", "nightly")


def find_creator(
        outputfile: str,
        package: str,
        scripts: List[Script],
        log: logging.Logger
) -> Optional[List[Script]]:
    """!
    This function receives the name of a file and tries to find the file
    in the given package which produces this file, i.e. find the file in
    whose header 'outputfile' is listed under <output></output>.
    It then returns a list of all Scripts who claim to be creating 'outputfile'

    @param outputfile: The file of which we want to know by which script is
        created
    @param package: The package in which we want to search for the creator
    @param scripts: List of all script objects/candidates
    @param log: Logger
    """

    # Get a list of all Script objects for scripts in the given package as well
    # as from the validation-folder
    candidates = [script for script in scripts
                  if script.package in [package, 'validation']]

    # Reserve some space for the results we will return
    results = []

    # Loop over all candidates and check if they have 'outputfile' listed
    # under their outputs
    for candidate in candidates:
        if outputfile in candidate.output_files:
            results.append(candidate)

    # Return our results and warn if there is more than one creator
    if len(results) == 0:
        return None
    if len(results) > 1:
        log.warning('Found multiple creators for' + outputfile)
    return results
