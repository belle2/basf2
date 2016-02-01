import os
import re
from datetime import datetime
from subprocess import check_output
from ipython_tools import viewer


class Basf2Information:

    """
    Helper class for accessing the information about basf2
    from the environment variables.
    """

    def __init__(self):
        """
        Get the variables from the environment variables.
        """
        #: Externals version
        self.externals_version = os.environ.get("BELLE2_EXTERNALS_VERSION")
        #: Compile options of externals
        self.externals_option = os.environ.get("BELLE2_EXTERNALS_OPTION")
        #: Compile options of Belle2
        self.option = os.environ.get("BELLE2_OPTION")
        #: OS
        self.architecture = os.environ.get("BELLE2_ARCH")
        #: Release version
        self.release = os.environ.get("BELLE2_RELEASE")
        #: Release folder
        self.release_folder = os.environ.get("BELLE2_LOCAL_DIR")
        #: Revision number (cached, the real getter is the property)
        self._cached_revision = None

    @property
    def revision_number(self):
        """
        Get the cached revision number from SVN or get it from SVN directly.
        """
        if not self._cached_revision:
            self._cached_revision = self.get_current_revision_number()

        return self._cached_revision

    def __str__(self):
        """
        A nice representation.
        """
        result = ""
        result += "externals_version: " + self.externals_version + "\n"
        result += "externals_option: " + self.externals_option + "\n"
        result += "option: " + self.option + "\n"
        result += "architecture: " + self.architecture + "\n"
        result += "release: " + self.release + "\n"
        result += "release folder: " + self.release_folder + "\n"
        result += "revision number: " + self.revision_number + "\n"
        result += "date: " + datetime.now().strftime("%Y-%m-%d") + "\n"
        return result

    def __repr__(self):
        """
        Also for ipython.
        """
        return self.__str__()

    def get_current_revision_number(self):
        """
        Try to download the current revision number from SVN.
        """
        try:
            info = check_output(["svn", "info"], cwd=self.release_folder)
        except:
            try:
                info = check_output(["git", "svn", "info"], cwd=self.release_folder)
            except:
                return ""

        for line in info.decode().split("\n"):
            match = re.search(r"Revision:\s(.*)", line)
            if match:
                return match.group(1)


class ModulesInformation:
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

        v = viewer.PathViewer(filter_modules(), standalone=True)
        v.show()
