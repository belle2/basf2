import os
import re

import hep_ipython_tools.ipython_handler_basf2.viewer
from hep_ipython_tools import viewer
from hep_ipython_tools.information import EnvironmentInformation


class Basf2EnvironmentInformation(EnvironmentInformation):

    """
    Helper class for accessing the information about ipython_handler_basf2
    from the environment variables.
    """

    def __init__(self):
        """
        Get the variables from the environment variables.
        """
        super().__init__()

        self.externals_version = os.environ.get("BELLE2_EXTERNALS_VERSION")
        self.externals_option = os.environ.get("BELLE2_EXTERNALS_OPTION")
        self.option = os.environ.get("BELLE2_OPTION")
        self.architecture = os.environ.get("BELLE2_ARCH")
        self.release = os.environ.get("BELLE2_RELEASE")
        self.release_folder = os.environ.get("BELLE2_LOCAL_DIR")


class Basf2ModulesInformation:
    """
    A helper class to perform module lookup.
    """

    def __init__(self):
        """
        Initialize with the module list from the framework.
        """
        from basf2 import fw
        self.module_list = fw.list_available_modules()

    def search(self, regex_string):
        """
        Search for a given module. You can give ane regular expression you like.
         The results will be printed as a nice tabbed view with the modules and their parameters
         and descriptions.
        """
        def filter_modules():
            for module_name in self.module_list:
                if re.search(regex_string, module_name):
                    yield module_name

        v = hep_ipython_tools.ipython_handler_basf2.viewer.PathViewer(filter_modules(), standalone=True)
        v.show()
