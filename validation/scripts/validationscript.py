#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import re
import os

# A pretty printer. Prints prettier lists, dicts, etc. :)
import pprint
pp = pprint.PrettyPrinter(depth=6, indent=1, width=80)

# Import XML Parser. Use C-Version, if available
try:
    import xml.etree.cElementTree as XMLTree
except ImportError:
    import xml.etree.ElementTree as XMLTree

from validationfunctions import find_creator


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

    def __init__(self, path, package, log):
        """!
        The default constructor.
        """

        # Pointer to the script object itself
        # Is this necessary?
        self._object = self

        # stores the reference to the logging object used in this validation run
        self.log = log

        # The (absolute) path of the steering file
        self.path = path

        # The runtime of the script
        self.runtime = None
        self.start_time = None

        # The name of the steering file. Basically the file name of the
        # steering file, but everything that is not a letter is replaced
        # by an underscore. Useful e.g. for cluster controls.
        self.name = re.sub(r'[\W_]+', '_', str(os.path.basename(self.path)))

        # The package to which the steering file belongs
        self.package = package

        # The information from the file header
        self.header = None

        # A list of script objects, on which this script depends
        self.dependencies = []

        # Current status of the script.
        # Possible values: 'waiting', 'running', 'finished', 'failed'
        self.status = 'waiting'

        # Which control is used for executing the script, i.e. cluster or
        # local. Useful when using different script level, e.g. data creation
        # scripts are being run on the cluster, but plotting scripts are
        # executed locally
        self.control = None

        # The returncode of the script. Should be 0 if all went well.
        self.returncode = None

    def dump(self):
        """!
        Print out all properties = attributes of a script.
        @return: None
        """
        print
        pp.pprint(vars(self))

    def get_dependencies(self, list_of_scripts):
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
                creator = find_creator(root_file, self.package, list_of_scripts, self.log)

                # If no creator could be found, raise an error!
                if creator is None:
                    self.log.error('Unmatched dependency for {0}:'
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
            in_same_pkg = [script for script in list_of_scripts
                           if script.package == self.package]

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
            self.log.error('No file header found: ' + self.path)
            return

        # Create an XML tree from the plain XML code.
        try:
            xml_tree = XMLTree.ElementTree(XMLTree.fromstring(xml)).getroot()
        except XMLTree.ParseError:
            self.log.error('Invalid XML in header: ' + self.path)
            return

        # we have a header
        self.header = {}

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
            if branch.tag.strip() in self.header:
                self.header[branch.tag.strip()] += branch_value
            else:
                self.header[branch.tag.strip()] = branch_value
