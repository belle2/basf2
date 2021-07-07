##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from pathlib import Path
from unittest import main
import basf2

import zmq

from zmq_daq.test_support import HLTZMQTestCase


class HLTTestCase(HLTZMQTestCase):
    """Test case"""
    #: event_data
    event_data = open(basf2.find_file("daq/hbasf2/tests/out.raw"), "br").read()

    def setUp(self):
        """Setup port numbers and necessary programs"""
        #: distributor_input_port
        self.distributor_input_port = HLTZMQTestCase.get_free_port()
        #: distributor_output_port
        self.distributor_output_port = HLTZMQTestCase.get_free_port()
        #: distributor_monitoring_port
        self.distributor_monitoring_port = HLTZMQTestCase.get_free_port()

        #: collector_input_port
        self.collector_input_port = HLTZMQTestCase.get_free_port()
        #: collector_output_port
        self.collector_output_port = HLTZMQTestCase.get_free_port()
        #: collector_monitoring_port
        self.collector_monitoring_port = HLTZMQTestCase.get_free_port()

        #: final_collector_input_port
        self.final_collector_input_port = HLTZMQTestCase.get_free_port()
        #: final_collector_output_port
        self.final_collector_output_port = HLTZMQTestCase.get_free_port()
        #: final_collector_monitoring_port
        self.final_collector_monitoring_port = HLTZMQTestCase.get_free_port()

        #: needed_programs
        self.needed_programs = {
            "distributor": [
                "b2hlt_distributor",
                "--input", f"tcp://localhost:{self.distributor_input_port}",
                "--output", f"tcp://*:{self.distributor_output_port}",
                "--monitor", f"tcp://*:{self.distributor_monitoring_port}"
            ],
            "collector": [
                "b2hlt_collector",
                "--input", f"tcp://*:{self.collector_input_port}",
                "--output", f"tcp://*:{self.collector_output_port}",
                "--monitor", f"tcp://*:{self.collector_monitoring_port}"
            ],
            "final_collector": [
                "b2hlt_finalcollector",
                "--input", f"tcp://*:{self.final_collector_input_port}",
                "--output", f"tcp://localhost:{self.final_collector_output_port}",
                "--monitor", f"tcp://*:{self.final_collector_monitoring_port}"
            ],
            "worker": [
                "python3", basf2.find_file("daq/hbasf2/tests/passthrough.no_run_py"),
                "--input", f"tcp://localhost:{self.distributor_output_port}",
                "--output", f"tcp://localhost:{self.collector_input_port}"
            ],
            "output_worker": [
                "python3", basf2.find_file("daq/hbasf2/tests/passthrough.no_run_py"),
                "--prefix", "output_",
                "--input", f"tcp://localhost:{self.collector_output_port}",
                "--output", f"tcp://localhost:{self.final_collector_input_port}"
            ],
        }
        super().setUp()

    def testFullRun(self):
        """test function"""
        distributor_monitoring_socket = self.create_socket(self.distributor_monitoring_port)
        collector_monitoring_socket = self.create_socket(self.collector_monitoring_port)
        final_collector_monitoring_socket = self.create_socket(self.final_collector_monitoring_port)

        input_socket = self.create_socket(self.distributor_input_port, socket_type=zmq.STREAM, bind=True)
        input_identity, _ = self.recv(input_socket)

        output_socket = self.create_socket(self.final_collector_output_port, socket_type=zmq.STREAM, bind=True)
        output_identity, _ = self.recv(output_socket)

        # At the beginning, everything should be at normal state
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)

        self.assertHasOutputFile("initialize_called", timeout=1)
        self.assertHasOutputFile("output_initialize_called", timeout=1)

        # Now lets try sending some events
        for _ in range(20):
            input_socket.send_multipart([input_identity, self.event_data])

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(distributor_monitoring_socket, "output.sent_events", 20)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "output.sent_events", 20)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(final_collector_monitoring_socket, "output.sent_events", 20)

        self.assertHasOutputFile("beginrun_called", timeout=1)
        self.assertHasOutputFile("output_beginrun_called", timeout=1)

        buffer = b""
        while output_socket.poll(0):
            _, msg = self.recv(output_socket)
            buffer += msg
        self.assertNothingMore(output_socket)

        # Data Size != raw data, as data is in different format, size taken from test itself
        self.assertEqual(len(buffer), 122638 * 20)

        # Now we stop this run
        self.send(distributor_monitoring_socket, "l")

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.all_stop_messages", 1)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.all_stop_messages", 1)

        # should go to the workers
        self.assertHasOutputFile("endrun_called", timeout=1)
        self.assertHasOutputFile("output_endrun_called", timeout=1)

        # but not to the output
        self.assertNothingMore(output_socket)

        # a second stop message should only reach the first workers up to the collector
        self.send(distributor_monitoring_socket, "l")

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.all_stop_messages", 1)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.all_stop_messages", 1)

        # should go to the workers (longer timeout as distributor waits a bit)
        self.assertHasOutputFile("endrun_called", timeout=5)
        self.assertNotHasOutputFile("output_endrun_called", timeout=1)

        # Now lets restart the run
        self.send(distributor_monitoring_socket, "n")
        self.send(collector_monitoring_socket, "n")
        self.send(final_collector_monitoring_socket, "n")

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(collector_monitoring_socket, "input.all_stop_messages", 0)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 20)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(final_collector_monitoring_socket, "input.all_stop_messages", 0)

        # And send some more events
        for _ in range(20):
            input_socket.send_multipart([input_identity, self.event_data])

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(distributor_monitoring_socket, "output.sent_events", 40)

        self.assertMonitoring(collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "input.received_events", 40)
        self.assertMonitoring(collector_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(collector_monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(collector_monitoring_socket, "output.sent_events", 40)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 40)
        self.assertMonitoring(final_collector_monitoring_socket, "output.sent_events", 40)

        self.assertNotHasOutputFile("endrun_called", timeout=1)
        self.assertNotHasOutputFile("output_endrun_called", timeout=1)
        self.assertHasOutputFile("beginrun_called", timeout=1)
        self.assertHasOutputFile("output_beginrun_called", timeout=1)

        buffer = b""
        while output_socket.poll(0):
            _, msg = self.recv(output_socket)
            buffer += msg
        self.assertNothingMore(output_socket)

        # Data Size != raw data, as data is in different format, size taken from test itself
        self.assertEqual(len(buffer), 122638 * 20)

        # And finally: terminate everything
        self.send(distributor_monitoring_socket, "x")

        self.assertIsDown("collector")
        self.assertIsDown("final_collector")
        self.assertIsDown("distributor")
        self.assertIsDown("worker", timeout=1)
        self.assertIsDown("output_worker")


class DyingHLTTestCase(HLTZMQTestCase):
    """Test case"""
    #: event_data
    event_data = open(basf2.find_file("daq/hbasf2/tests/out.raw"), "br").read()

    def setUp(self):
        """Setup port numbers and necessary programs"""
        #: distributor_input_port
        self.distributor_input_port = HLTZMQTestCase.get_free_port()
        #: distributor_output_port
        self.distributor_output_port = HLTZMQTestCase.get_free_port()
        #: distributor_monitoring_port
        self.distributor_monitoring_port = HLTZMQTestCase.get_free_port()

        #: final_collector_input_port
        self.final_collector_input_port = HLTZMQTestCase.get_free_port()
        #: final_collector_output_port
        self.final_collector_output_port = HLTZMQTestCase.get_free_port()
        #: final_collector_monitoring_port
        self.final_collector_monitoring_port = HLTZMQTestCase.get_free_port()

        #: needed_programs
        self.needed_programs = {
            "distributor": [
                "b2hlt_distributor",
                "--input", f"tcp://localhost:{self.distributor_input_port}",
                "--output", f"tcp://*:{self.distributor_output_port}",
                "--monitor", f"tcp://*:{self.distributor_monitoring_port}"
            ],
            "final_collector": [
                "b2hlt_finalcollector", "--input", f"tcp://*:{self.final_collector_input_port}",
                "--output", f"tcp://localhost:{self.final_collector_output_port}",
                "--monitor", f"tcp://*:{self.final_collector_monitoring_port}"
            ],
            "worker": [
                "python3", basf2.find_file("daq/hbasf2/tests/passthrough.no_run_py"),
                "--input", f"tcp://localhost:{self.distributor_output_port}",
                "--output", f"tcp://localhost:{self.final_collector_input_port}"
            ],
            "dying_worker": [
                "python3", basf2.find_file("daq/hbasf2/tests/passthrough.no_run_py"),
                "--prefix", "dying_", "--exit",
                "--input", f"tcp://localhost:{self.distributor_output_port}",
                "--output", f"tcp://localhost:{self.final_collector_input_port}"
            ],
        }
        super().setUp()

    def testFullRun(self):
        """test function"""
        distributor_monitoring_socket = self.create_socket(self.distributor_monitoring_port)
        final_collector_monitoring_socket = self.create_socket(self.final_collector_monitoring_port)

        input_socket = self.create_socket(self.distributor_input_port, socket_type=zmq.STREAM, bind=True)
        input_identity, _ = self.recv(input_socket)

        output_socket = self.create_socket(self.final_collector_output_port, socket_type=zmq.STREAM, bind=True)
        output_identity, _ = self.recv(output_socket)

        # At the beginning, everything should be at normal state
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 40)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 2)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 2)

        self.assertHasOutputFile("initialize_called", timeout=1)
        self.assertHasOutputFile("dying_initialize_called", timeout=1)

        # Now lets try sending some events
        for _ in range(100):
            input_socket.send_multipart([input_identity, self.event_data])

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 40)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 2)
        self.assertMonitoring(distributor_monitoring_socket, "output.sent_events", 100)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 2)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 100)
        self.assertMonitoring(final_collector_monitoring_socket, "output.sent_events", 100)

        self.assertHasOutputFile("beginrun_called", timeout=1)
        self.assertHasOutputFile("dying_beginrun_called", timeout=1)

        buffer = b""
        while output_socket.poll(0):
            _, msg = self.recv(output_socket)
            buffer += msg
        self.assertNothingMore(output_socket)

        # Data Size != raw data, as data is in different format, size taken from test itself
        self.assertEqual(len(buffer), 122638 * 100)

        # Now we kill one of the workers on purpose
        Path("dying_exit_request").touch()

        # And send some more events
        for _ in range(100):
            input_socket.send_multipart([input_identity, self.event_data])

        self.assertHasOutputFile("dying_exit_called", timeout=1)
        self.assertIsDown("dying_worker", timeout=10)

        buffer = b""
        while output_socket.poll(0):
            _, msg = self.recv(output_socket)
            buffer += msg
        self.assertNothingMore(output_socket)

        # We expect to have at least some events (but some will be missing)!
        # 100 - 20 (queue size of dying worker) - 1 (the event that "caused" the problem) = 79
        self.assertEqual(len(buffer), 122638 * 79)

        # The collector should have it removed
        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)

        # Also a stop signal should be transported correctly
        self.send(distributor_monitoring_socket, "l")

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        # the distributor does not know about the dying worker, but this is also no problem
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 2)
        self.assertMonitoring(distributor_monitoring_socket, "output.sent_events", 200)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 179)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.all_stop_messages", 1)

        # should go to the workers
        self.assertHasOutputFile("endrun_called", timeout=1)

        # Now lets restart the run
        self.send(distributor_monitoring_socket, "n")
        self.send(final_collector_monitoring_socket, "n")

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 2)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 179)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(final_collector_monitoring_socket, "input.all_stop_messages", 0)

        for _ in range(100):
            input_socket.send_multipart([input_identity, self.event_data])

        self.assertMonitoring(distributor_monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(distributor_monitoring_socket, "input.socket_disconnects", 0)
        self.assertMonitoring(distributor_monitoring_socket, "output.ready_queue_size", 20)
        self.assertMonitoring(distributor_monitoring_socket, "output.registered_workers", 2)
        self.assertMonitoring(distributor_monitoring_socket, "output.sent_events", 300)

        self.assertMonitoring(final_collector_monitoring_socket, "input.registered_workers", 1)
        self.assertMonitoring(final_collector_monitoring_socket, "input.received_events", 279)
        self.assertMonitoring(final_collector_monitoring_socket, "output.sent_events", 279)

        self.assertNotHasOutputFile("endrun_called", timeout=1)
        self.assertHasOutputFile("beginrun_called", timeout=1)

        buffer = b""
        while output_socket.poll(0):
            _, msg = self.recv(output_socket)
            buffer += msg
        self.assertNothingMore(output_socket)

        # Now all events should be transported again
        self.assertEqual(len(buffer), 122638 * 100)

        # And finally: terminate everything
        self.send(distributor_monitoring_socket, "x")

        self.assertIsDown("final_collector")
        self.assertIsDown("distributor")
        self.assertIsDown("worker")


if __name__ == '__main__':
    main()
