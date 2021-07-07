##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
import numpy as np

from hep_ipython_tools.entities import Statistics, StatisticsColumn


class ModuleStatistics:

    """
    Dictionary list holding the module statistics (like the C++ class ModuleStatistics)
    """

    def __init__(self, stats, categories):
        """Create a new module statistics entity."""
        #: Name property
        self.name = stats.name
        #: Time sum property
        self.time_sum = self.get_dict(stats.time_sum, categories)
        #: Time mean property
        self.time_mean = self.get_dict(stats.time_mean, categories)
        #: Time std property
        self.time_stddev = self.get_dict(stats.time_stddev, categories)
        #: Memory sum property
        self.memory_sum = self.get_dict(stats.memory_sum, categories)
        #: Memory mean property
        self.memory_mean = self.get_dict(stats.memory_mean, categories)
        #: Memory std property
        self.memory_stddev = self.get_dict(stats.memory_stddev, categories)
        #: Calls property
        self.calls = self.get_dict(stats.calls, categories)

    @staticmethod
    def get_dict(function, categories):
        """
        Call the function on each information in the categories and return a dict
        name -> function(value)
        """
        return {name: function(category) for name, category in categories}

    def __getitem__(self, item):
        """Convenience function for the display."""
        if item == "name":
            return self.name
        elif item == "calls":
            return self.calls["EVENT"]
        elif item == "mem":
            return self.memory_mean["EVENT"] * 1E-3
        elif item == "time":
            return np.round(self.time_sum["EVENT"] * 1E-9, 2)
        elif item == "eventtime":
            return np.round(self.time_mean["EVENT"] * 1E-6, 2), "&pm;", np.round(self.time_stddev["EVENT"] * 1E-6, 2)


class Basf2CalculationQueueStatistics(Statistics):

    """
    As the ipython_handler_basf2 statistics is not pickable, we can not store it into the queue.
    So we write a wrapper which unpacks the needed properties.
    """

    def __str__(self):
        """ Make the str behave like before """
        return self.str

    def __repr__(self):
        """ Make the repr behave like before """
        return self.str

    def __init__(self, statistics):
        """ Initialize with the C++ statistics """
        modules = []

        columns = [StatisticsColumn("name", "Name"),
                   StatisticsColumn("calls", "Calls"), StatisticsColumn("mem", "VMemory (MB)"),
                   StatisticsColumn("time", "Time (s)"), StatisticsColumn("eventtime", "Time (ms)/Call", True)]

        categories = [("INIT", statistics.INIT),
                      ("BEGIN_RUN", statistics.BEGIN_RUN),
                      ("EVENT", statistics.EVENT),
                      ("END_RUN", statistics.END_RUN),
                      ("TERM", statistics.TERM),
                      ("TOTAL", statistics.TOTAL)]

        for stats in statistics.modules:
            modules.append(ModuleStatistics(stats, categories))

        modules.append(ModuleStatistics(statistics.get_global(), categories))

        #: The str representation
        self.str = statistics()

        super().__init__(columns, modules)
