##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from unittest import main
import basf2

import zmq

from zmq_daq.test_support import HLTZMQTestCase


class DistributorTestCase(HLTZMQTestCase):
    """Test case"""
    #: event_data
    event_data = open(basf2.find_file("daq/hbasf2/tests/out.raw"), "br").read()

    def setUp(self):
        """Setup port numbers and necessary programs"""
        #: input_port
        self.input_port = HLTZMQTestCase.get_free_port()
        #: output_port
        self.output_port = HLTZMQTestCase.get_free_port()
        #: monitoring_port
        self.monitoring_port = HLTZMQTestCase.get_free_port()
        #: needed_programs
        self.needed_programs = {
            "distributor": [
                "b2hlt_distributor",
                "--input", f"tcp://localhost:{self.input_port}",
                "--output", f"tcp://*:{self.output_port}",
                "--monitor", f"tcp://*:{self.monitoring_port}",
                "--stopWaitingTime", "1"]}
        super().setUp()

    def testConnectAndDisconnect(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        self.assertMonitoring(monitoring_socket, "input.socket_state", "disconnected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 0)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # To make it actually look for input messages, it needs at least a single worker
        output_socket = self.create_socket(self.output_port)
        for _ in range(5):
            self.send(output_socket, "r")

        # Now we can go on
        input_socket = self.create_socket(self.input_port, socket_type=zmq.STREAM, bind=True)

        # Still no connection
        self.assertMonitoring(monitoring_socket, "input.socket_state", "disconnected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 0)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # This should initiate a connection
        identity, _ = self.recv(input_socket)

        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # And we can also close it again
        input_socket.close()

        self.assertMonitoring(monitoring_socket, "input.socket_state", "disconnected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 1)

        # or open
        input_socket = self.create_socket(self.input_port, socket_type=zmq.STREAM, bind=True)
        identity, _ = self.recv(input_socket)

        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 2)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 1)

        # sending an event should not change anything
        input_socket.send_multipart([identity, self.event_data])
        input_socket.send_multipart([identity, self.event_data])
        input_socket.send_multipart([identity, self.event_data])
        input_socket.send_multipart([identity, self.event_data])

        self.assertMonitoring(monitoring_socket, "input.received_events", 4)

        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 2)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 1)

        input_socket.close()

        self.assertMonitoring(monitoring_socket, "input.socket_state", "disconnected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 2)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 2)

    def testEvents(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        # connect the input
        input_socket = self.create_socket(self.input_port, socket_type=zmq.STREAM, bind=True)
        identity, _ = self.recv(input_socket)

        # Start two workers
        output_socket = self.create_socket(self.output_port)
        second_output_socket = self.create_socket(self.output_port, identity="other_socket")

        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 0)

        # Every ready should give us an event at exactly this worker, when there is an event

        # 1. only send ready message
        self.send(output_socket, "r")

        # No event
        self.assertNothingMore(output_socket)
        self.assertNothingMore(second_output_socket)

        # But ready worker
        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 1)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(monitoring_socket, "output.ready_messages[socket]", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # Now the event
        input_socket.send_multipart([identity, self.event_data])

        self.assertIsMsgType(output_socket, "u")
        self.assertNothingMore(second_output_socket)

        # And no ready worker
        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(monitoring_socket, "output.ready_messages[socket]", 0)
        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # 2. Try out sending the event first
        input_socket.send_multipart([identity, self.event_data])

        # Still no event
        self.assertNothingMore(output_socket)
        self.assertNothingMore(second_output_socket)

        # And no updated monitoring
        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 1)
        self.assertMonitoring(monitoring_socket, "output.ready_messages[socket]", 0)
        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # but with a ready message
        self.send(second_output_socket, "r")

        self.assertNothingMore(output_socket)
        self.assertIsMsgType(second_output_socket, "u")

        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 2)
        self.assertMonitoring(monitoring_socket, "input.socket_state", "connected")
        self.assertMonitoring(monitoring_socket, "input.socket_connects", 1)
        self.assertMonitoring(monitoring_socket, "input.socket_disconnects", 0)

        # and again
        input_socket.send_multipart([identity, self.event_data])

        self.send(second_output_socket, "r")
        self.assertNothingMore(output_socket)
        self.assertIsMsgType(second_output_socket, "u")

        # As we have answered all ready messages, nothing should be in the queue
        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 2)

        # until we start sending more ready messages
        self.send(output_socket, "r")
        self.send(output_socket, "r")
        self.send(second_output_socket, "r")
        self.send(second_output_socket, "r")

        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 4)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 2)

    def testEndRun(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        # Start two workers
        output_socket = self.create_socket(self.output_port)
        second_output_socket = self.create_socket(self.output_port, identity="other_socket")

        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 0)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 0)

        # and register them by sending a ready
        self.send(output_socket, "r")
        self.send(second_output_socket, "r")

        self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 2)
        self.assertMonitoring(monitoring_socket, "output.registered_workers", 2)

        # a stop run should be sent to all workers
        self.send(monitoring_socket, "l")

        self.assertIsMsgType(output_socket, "l")
        self.assertIsMsgType(second_output_socket, "l")

        # also multiple times
        self.send(monitoring_socket, "l")

        self.assertIsMsgType(output_socket, "l")
        self.assertIsMsgType(second_output_socket, "l")

        # and if there are events in between they should not be transported
        input_socket = self.create_socket(self.input_port, socket_type=zmq.STREAM, bind=True)
        identity, _ = self.recv(input_socket)
        input_socket.send_multipart([identity, self.event_data])
        input_socket.send_multipart([identity, self.event_data])

        self.assertNothingMore(output_socket)
        self.assertNothingMore(second_output_socket)

        # it shouldn't matter
        self.send(monitoring_socket, "l")

        self.assertIsMsgType(output_socket, "l")
        self.assertIsMsgType(second_output_socket, "l")

        # A terminate message should also be sent
        self.send(monitoring_socket, "x")

        self.assertIsMsgType(output_socket, "x")
        self.assertIsMsgType(second_output_socket, "x")

        # and the distributor should go down
        self.assertIsDown("distributor")


if __name__ == '__main__':
    main()
