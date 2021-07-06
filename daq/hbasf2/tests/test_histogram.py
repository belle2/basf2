##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import os
from unittest import main
import basf2


from zmq_daq.test_support import HLTZMQTestCase


def check_histogram_output(file_name, expected_factor):
    """Open the given file name and check if the contained histogram has exactly expected_factor * 2 entries"""
    result = False

    import ROOT
    root_file = ROOT.TFile(file_name, "READ")
    histogram = root_file.Get("my_histogram")

    if expected_factor == 0:
        if not histogram:
            result = True
    else:
        # every histogram has 2 entries from the start with, so we multiply with 2
        try:
            if histogram.GetEntries() == 2 * expected_factor:
                result = True
        except AttributeError:
            pass

    root_file.Close()
    os.unlink(file_name)

    return result


class HistogramTestCase(HLTZMQTestCase):
    """Test case"""
    #: histogram_data
    histogram_data = open(basf2.find_file("daq/hbasf2/tests/histos.raw"), "br").read()
    #: event_data
    event_data = b"""{
        "_typename" : "Belle2::EventMetaData",
        "fUniqueID" : 0,
        "fBits" : 33554432,
        "m_event" : 1,
        "m_run" : 1,
        "m_subrun" : 0,
        "m_experiment" : 1,
        "m_production" : 0,
        "m_time" : 0,
        "m_parentLfn" : "",
        "m_generatedWeight" : 1,
        "m_errorFlag" : 0
        }"""

    def setUp(self):
        """Setup port numbers and necessary programs"""
        #: first_input_port
        self.first_input_port = HLTZMQTestCase.get_free_port()
        #: first_monitoring_port
        self.first_monitoring_port = HLTZMQTestCase.get_free_port()

        #: second_input_port
        self.second_input_port = HLTZMQTestCase.get_free_port()
        #: second_monitoring_port
        self.second_monitoring_port = HLTZMQTestCase.get_free_port()

        #: final_collector_input_port
        self.final_collector_input_port = HLTZMQTestCase.get_free_port()
        #: final_collector_monitoring_port
        self.final_collector_monitoring_port = HLTZMQTestCase.get_free_port()
        #: needed_programs
        self.needed_programs = {
            "first": [
                "b2hlt_proxyhistoserver", "--input", f"tcp://*:{self.first_input_port}",
                "--output", f"tcp://localhost:{self.final_collector_input_port}",
                "--timeout", "1",
                "--monitor", f"tcp://*:{self.first_monitoring_port}"
            ],
            "second": [
                "b2hlt_proxyhistoserver",
                "--input", f"tcp://*:{self.second_input_port}",
                "--output", f"tcp://localhost:{self.final_collector_input_port}",
                "--timeout", "1",
                "--monitor", f"tcp://*:{self.second_monitoring_port}"
            ],
            "final_collector": [
                "b2hlt_finalhistoserver",
                "--input", f"tcp://*:{self.final_collector_input_port}",
                "--rootFileName", "outputFile.root",
                "--timeout", "1",
                "--monitor", f"tcp://*:{self.final_collector_monitoring_port}"
            ],
        }
        super().setUp()

    def testEventPropagation(self):
        """test function"""
        self.assertNotHasOutputFile("outputFile.root")

        first_monitoring_socket = self.create_socket(self.first_monitoring_port)
        second_monitoring_socket = self.create_socket(self.second_monitoring_port)
        final_monitoring_socket = self.create_socket(self.final_collector_monitoring_port)

        input_sockets = [
            self.create_socket(self.first_input_port, identity="1"),
            self.create_socket(self.first_input_port, identity="2"),
            self.create_socket(self.second_input_port, identity="3"),
            self.create_socket(self.second_input_port, identity="4"),
            self.create_socket(self.second_input_port, identity="5"),
        ]

        for input_socket in input_sockets:
            self.send(input_socket, "h")
            self.assertIsMsgType(input_socket, "c")

        # At the beginning, everything should be at normal state
        self.assertMonitoring(first_monitoring_socket, "input.registered_workers", 2)
        self.assertMonitoring(second_monitoring_socket, "input.registered_workers", 3)
        self.assertMonitoring(final_monitoring_socket, "input.registered_workers", 2)

        # So far no stop messages should be there
        self.assertMonitoring(first_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(second_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(final_monitoring_socket, "input.received_stop_messages", 0)

        self.assertMonitoring(first_monitoring_socket, "input.all_stop_messages", False)
        self.assertMonitoring(second_monitoring_socket, "input.all_stop_messages", False)
        self.assertMonitoring(final_monitoring_socket, "input.all_stop_messages", False)

        # send some events, which should eventually trigger a merge
        for _ in range(10):
            for input_socket in input_sockets:
                self.send(input_socket, "v", self.histogram_data, self.event_data)
                self.assertIsMsgType(input_socket, "c")

        self.assertMonitoring(first_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(second_monitoring_socket, "input.received_events", 30)
        self.assertMonitoring(final_monitoring_socket, "input.received_events", 2)

        # Make sure to get the newest data
        self.check_histogram_repeated("outputFile.root", 5)

        # send out stop messages, which should also trigger a merge
        for input_socket in input_sockets:
            self.send(input_socket, "l")
            self.assertIsMsgType(input_socket, "c")

        self.assertMonitoring(first_monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(first_monitoring_socket, "input.all_stop_messages", True)
        self.assertMonitoring(second_monitoring_socket, "input.received_stop_messages", 3)
        self.assertMonitoring(second_monitoring_socket, "input.all_stop_messages", True)

        self.assertMonitoring(final_monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(final_monitoring_socket, "input.all_stop_messages", True)
        self.assertHasOutputFile("outputFile.root", unlink=False)
        self.check_histogram_repeated("outputFile.root", 5)

        # Now we clean up
        self.send(first_monitoring_socket, "n")
        self.send(second_monitoring_socket, "n")
        self.send(final_monitoring_socket, "n")

        # which should give us a clean root file again
        self.assertNotHasOutputFile("outputFile.root", timeout=1)

        self.assertMonitoring(first_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(first_monitoring_socket, "input.all_stop_messages", False)
        self.assertMonitoring(second_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(second_monitoring_socket, "input.all_stop_messages", False)

        self.assertMonitoring(final_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(final_monitoring_socket, "input.all_stop_messages", False)

        # and send some more events, but only to the first two clients
        for _ in range(5):
            for input_socket in input_sockets[:2]:
                self.send(input_socket, "v", self.histogram_data, self.event_data)
                self.assertIsMsgType(input_socket, "c")

        # this time we expect less entries
        self.check_histogram_repeated("outputFile.root", 2)

        # Now send a terminate message
        for input_socket in input_sockets:
            self.send(input_socket, "x")
            self.assertIsMsgType(input_socket, "c")

        # We expect another merge here
        self.assertHasOutputFile("outputFile.root", unlink=False)
        self.check_histogram_repeated("outputFile.root", 2)

        self.assertIsDown("first")
        self.assertIsDown("second")
        self.assertIsDown("final_collector")

    def check_histogram_repeated(self, file_name, expected_factor):
        """Repeatedly call check_histogram_output 5 times until it is actually fulfilled"""
        tries = 0
        while tries < 5:
            self.assertHasOutputFile(file_name, unlink=False, timeout=2)
            if check_histogram_output(file_name, expected_factor):
                break
            tries += 1
        else:
            raise AssertionError("Even after retry, the output was not correct!")


class HistogramStopTestCase(HLTZMQTestCase):
    """Test case"""
    #: input_port
    input_port = HLTZMQTestCase.get_free_port()
    #: monitoring_port
    monitoring_port = HLTZMQTestCase.get_free_port()

    #: needed_programs
    needed_programs = {"histoserver": ["b2hlt_finalhistoserver", "--input", f"tcp://*:{input_port}",
                                       "--rootFileName", "outputFile.root",
                                       "--timeout", "0",  # we remove the timeout on purpose
                                       "--monitor", f"tcp://*:{monitoring_port}"],
                       }

    #: histogram_data
    histogram_data = open(basf2.find_file("daq/hbasf2/tests/histos.raw"), "br").read()
    #: event_data
    event_data = b"""{
        "_typename" : "Belle2::EventMetaData",
        "fUniqueID" : 0,
        "fBits" : 33554432,
        "m_event" : 1,
        "m_run" : 1,
        "m_subrun" : 0,
        "m_experiment" : 1,
        "m_production" : 0,
        "m_time" : 0,
        "m_parentLfn" : "",
        "m_generatedWeight" : 1,
        "m_errorFlag" : 0
        }"""

    def testStopPropagation(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        input_socket = self.create_socket(self.input_port)
        self.send(input_socket, "h")
        self.assertIsMsgType(input_socket, "c")

        second_input_socket = self.create_socket(self.input_port, identity="other_socket")
        self.send(second_input_socket, "h")
        self.assertIsMsgType(second_input_socket, "c")

        # At the beginning, everything should be at normal state
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 2)
        self.assertNotHasOutputFile("outputFile.root", timeout=1)

        # So far no stop messages should be there
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)

        # the first stop message should not trigger a transmission
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)
        self.assertNotHasOutputFile("outputFile.root", timeout=1)

        # The second stop message should also not, as there are no histograms so far
        self.send(second_input_socket, "l")
        self.assertIsMsgType(second_input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)
        self.assertNotHasOutputFile("outputFile.root", timeout=1)

        # Reset everything
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)

        # Now lets send some events
        self.send(input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(input_socket, "c")

        self.send(input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(input_socket, "c")

        self.send(second_input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(second_input_socket, "c")

        self.send(input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(input_socket, "c")

        self.send(second_input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(second_input_socket, "c")

        self.send(second_input_socket, "v", self.histogram_data, self.event_data)
        self.assertIsMsgType(second_input_socket, "c")

        # This should not be enough to trigger a merge
        self.assertMonitoring(monitoring_socket, "input.received_events", 6)
        self.assertNotHasOutputFile("outputFile.root", timeout=1)

        # But if we again send the stop messages
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.send(second_input_socket, "l")
        self.assertIsMsgType(second_input_socket, "c")

        # .. it should have merged it. We expect 2 entries, as we have 2 clients (no matter how often they sent)
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)
        self.assertHasOutputFile("outputFile.root", unlink=False)
        self.assertTrue(check_histogram_output("outputFile.root", 2))

        # Now send a terminate message
        self.send(input_socket, "x")
        self.assertIsMsgType(input_socket, "c")
        self.send(second_input_socket, "x")
        self.assertIsMsgType(second_input_socket, "c")

        # There should be no merge happening, as the files are already written
        self.assertNotHasOutputFile("outputFile.root")

        self.assertIsDown("histoserver")


if __name__ == '__main__':
    main()
