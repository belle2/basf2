#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2.core as _basf2
from ROOT import Belle2

from hep_ipython_tools.entities import StoreContentList, StoreContent


class PrintCollections(_basf2.Module):

    """
    A small module to catch the content of the store array for some events and store them in the queue.
    """

    def __init__(self, queue):
        """
        Create the module with the given queue.
        """
        super().__init__()
        #: The queue to handle
        self.queue = queue
        #: The contents of the store arrays for some events
        self.store_content_list = []
        #: The current event number
        self.event_number = 0
        #: The total number of events
        self.total_number_of_events = 0

    def initialize(self):
        """
        Get the total number of events from C++
        """
        self.total_number_of_events = Belle2.Environment.Instance().getNumberOfEvents()

    def store_content(self):
        """
        Store the current content of the store array into the internal list.
        """
        registered_store_arrays = Belle2.PyStoreArray.list()
        registered_store_objects = Belle2.PyStoreObj.list()

        event_store_content_list = []

        for store_array_name in registered_store_arrays:
            store_array = Belle2.PyStoreArray(store_array_name)
            event_store_content_list.append(StoreContent(store_array_name, len(store_array)))

        for store_array_name in registered_store_objects:
            event_store_content_list.append(StoreContent(store_array_name, 0))

        event_store_content = StoreContentList(content=event_store_content_list, event_number=self.event_number)
        self.store_content_list.append(event_store_content)

    def event(self):
        """
        Write the store array content into a list for later.
        """
        if self.total_number_of_events == 0:
            if self.event_number % 1000 == 0:
                self.store_content()

        else:
            current_percentage = 1.0 * self.event_number / self.total_number_of_events

            if 100 * current_percentage % 10 == 0:
                self.store_content()

        self.event_number += 1

    def terminate(self):
        """
        Write the store array contents from the events to the queue.
        """
        self.queue.put("ipython.store_content", self.store_content_list)


class ProgressPython(_basf2.Module):

    """
    A small module that prints every now and then the event number to the given connection.
    It is used for status viewers. Do not call it by yourself.
    """

    def __init__(self, queue):
        """ Init the module """
        super().__init__()
        #: The queue to handle
        self.queue = queue
        #: The current event number
        self.event_number = 0
        #: The total number of events
        self.total_number_of_events = 0
        self.queue.send("init")

    def initialize(self):
        """ Send start to the connection """
        self.queue.send("start")

        # Receive the total number of events

        self.total_number_of_events = Belle2.Environment.Instance().getNumberOfEvents()

    def event(self):
        """ Send the event number to the connection """
        if self.total_number_of_events == 0:
            return

        current_percentage = 1.0 * self.event_number / self.total_number_of_events

        if 100 * current_percentage % 5 == 0:
            self.queue.send(current_percentage)

        self.event_number += 1

    def terminate(self):
        """ Send stop to the connection """
        self.queue.send(1)
        self.queue.send("end")
