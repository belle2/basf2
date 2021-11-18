##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2
from zmq_daq import test_support
from unittest import main


class CollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: this is not a final collector test case
    final_collector = False


class FinalCollectorTestCase(test_support.BaseCollectorTestCase):
    """Test case"""
    #: this is a final collector test case
    final_collector = True


if __name__ == '__main__':
    #: Number of failed for loops
    number_of_failures = 0

    for i in range(test_support.ZMQ_TEST_FOR_LOOPS):
        try:
            main(exit=False)
        except AssertionError:
            number_of_failures += 1

    #: Exit message
    message = f'Number of failed for loops: {number_of_failures}/{test_support.ZMQ_TEST_FOR_LOOPS}'
    if number_of_failures <= test_support.ZMQ_TEST_MAX_FAILURES:
        basf2.B2INFO(message)
    else:
        basf2.B2FATAL(message)
