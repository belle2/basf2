#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from multiprocessing import Queue
from queue import Empty


class Basf2CalculationQueue:

    """
    This class is a wrapper around a multiprocessing.Queue

    It can be used to send and receive values from the modules while processing the path.
    You can use it to save - for example - filepaths of outputfiles that you create on the fly.
    The added items are all of the type _Basf2CalculationQueueItem.
    The _Basf2CalculationQueue can be used as a dict. After the termination of the underlaying process
    you can access the different entries by their names you gave them when putting them on the queue.
    """

    def __init__(self):
        """
        Create a queue.
        """
        #: The multiprocessing queue to handle
        self.queue = Queue()
        #: The results to be filled in the end
        self.results = dict()

    def put(self, name, item, **kwargs):
        """
        Put an item on the queue with the given name. Please keep that adding two items with the same name
        overrides one of them!
        """
        self.queue.put(Basf2CalculationQueueItem(name, item), **kwargs)

    def fill_results(self):
        """
        Fill the internal dict with the information of the queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        Do only call this when the underlying process has ended.
        """
        while True:
            try:
                result = self.queue.get_nowait()
                self.results.update({result.name: result.item})
            except Empty:
                return

    def get(self, name):
        """
        Return the item with the given name or an Exception when it is not found.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return self.results[name]

    def get_keys(self):
        """
        Return all possible names of items saved in this queue.
        Do not call this but rather use the functions from _Basf2Calculation.
        """
        self.fill_results()
        return list(self.results.keys())


class Basf2CalculationQueueItem:

    """
    A placeholder for a tuple string, object.
    Do not create them by yourself.
    """

    def __init__(self, name, item):
        """
        Create a new queue item
        """
        #: Name of the item
        self.name = name
        #: Item to store
        self.item = item


class ModuleStatistics:

    """
    Dictionary list holding the module statistics (like the C++ class ModuleStatistics)
    """

    def __init__(self, stats, categories):
        """
        Create new module statistics based on basf2 statistics.
        """
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


class Basf2CalculationQueueStatistics():

    """
    As the basf2 statistics is not pickable, we can not store it into the queue.
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
        #: The module statistics list
        self.module = []

        categories = [("INIT", statistics.INIT),
                      ("BEGIN_RUN", statistics.BEGIN_RUN),
                      ("EVENT", statistics.EVENT),
                      ("END_RUN", statistics.END_RUN),
                      ("TERM", statistics.TERM),
                      ("TOTAL", statistics.TOTAL)]

        for stats in statistics.modules:
            self.append_module_statistics(stats, categories)

        self.append_module_statistics(statistics.getGlobal(), categories)

        #: The str representation
        self.str = statistics()

    def append_module_statistics(self, stats, categories):
        """ Helper function to append modulewise stats """
        self.module.append(ModuleStatistics(stats, categories))
