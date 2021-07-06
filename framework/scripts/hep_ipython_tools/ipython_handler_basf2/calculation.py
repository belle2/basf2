##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from hep_ipython_tools.calculation import Calculation

from hep_ipython_tools.ipython_handler_basf2.calculation_process import Basf2CalculationProcess
from hep_ipython_tools.ipython_handler_basf2 import viewer


class Basf2Calculation(Calculation):
    """
    Overloaded class with more functionality which is ipython_handler_basf2 specific:
      * Access the path and the module
      * Create Basf2 calculations
      * Use the Basf2 widgets
    """

    def __init__(self):
        """Create a new basf2 calculation mostly overloading the standard calculation."""
        super().__init__()

        #: Override the calculation process type with the basf2 one.
        self._calculation_process_type = Basf2CalculationProcess

    def show_path(self, index=None):
        """
        Show the underlaying ipython_handler_basf2 path in an interactive way
        """
        def f(process):
            if process.path is not None:
                return viewer.PathViewer(process.path)
            else:
                return None

        self.create_widgets_for_all_processes(f, index)

    def get_modules(self, index=None):
        """
        Return the modules in the given path.
        """
        return self.map_on_processes(lambda process: process.path.modules() if process.path is not None else None, index)

    def show_dependencies(self, index=None):
        """
        Show the dependencies among the data store entries.
        """
        def f(process):
            dependency_JSON = process.get("ipython.dependencies")
            if dependency_JSON:
                return viewer.DependencyViewer(dependency_JSON)
            else:
                return None

        self.create_widgets_for_all_processes(f, index)
