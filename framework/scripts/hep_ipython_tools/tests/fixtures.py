##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
try:
    from queue import Empty
except ImportError:
    from Queue import Empty

# @cond internal_test


class MockQueue:

    def __init__(self):
        self.internal_storage = []

    def put(self, item, block=True):
        self.internal_storage.append(item)

    def get_nowait(self):
        try:
            return self.internal_storage.pop(0)
        except IndexError:
            raise Empty

# @endcond
