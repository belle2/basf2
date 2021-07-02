##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from hep_ipython_tools.ipython_handler_basf2.calculation import Basf2Calculation
from hep_ipython_tools.ipython_handler_basf2.viewer import StylingWidget
from hep_ipython_tools.ipython_handler import IPythonHandler
from hep_ipython_tools.ipython_handler_basf2.information import Basf2ModulesInformation, Basf2EnvironmentInformation


class Basf2IPythonHandler(IPythonHandler):
    """
    Handler class to start processes in an IPython notebook in a convenient way.
    From this whole framework you should not need to create any instances by yourself but rather use the
    given ipython handler for this.

    Usage
    -----

    Create a handler object in the beginning of your NB and use the two methods `process`
    and `process_parameter_space` to turn a path or a path creator function into a Basf2Calculation.
    Do not create calculations on you own.

        from tracking.validation.ipython_handler import handler

        path = ...

        calculation = handler.process(path)

    """

    def __init__(self):
        """
        Set the ipython_handler_basf2 related shortcuts.
        """
        super().__init__()

        #: A shortcut for returning information on the ipython_handler_basf2 environment.
        self.information = Basf2EnvironmentInformation()

        #: A shortcut for returning module information
        self.modules = Basf2ModulesInformation()

        #: Use our own calculation type.
        self._calculation_type = Basf2Calculation

    def process(self, path, result_queue=None, max_event=0):
        """
        Shortcut for convenience.
        """
        return IPythonHandler.process(self, result_queue=result_queue, path=path, max_event=max_event)

    @staticmethod
    def style():
        """
        Show a nice styling :-)
        """
        styling_widget = StylingWidget()
        styling_widget.show()
