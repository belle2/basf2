##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# This file includes utilities for the unittests and is not needed during production cases
import atexit
import json
import os
import shutil
import signal
import subprocess
import tempfile
from time import sleep, time
from unittest import TestCase
import zmq


class HLTZMQTestCase(TestCase):
    """
    Base class for all HLT ZMQ tests helping to start the needed programs,
    create ZMQ sockets and send/receive messages easily.
    Has also some functionality for asserting test-correctness
    """
    #: The ZMQ context
    ctx = zmq.Context()
    #: The dict name -> cmd args of the programs to start, needs to be set in each test
    needed_programs = dict()

    @staticmethod
    def get_free_port():
        """
        Get a free port number by reusing ZMQ's function for this.
        """
        socket = HLTZMQTestCase.ctx.socket(zmq.ROUTER)
        port = socket.bind_to_random_port("tcp://*")
        socket.close()
        return port

    def setUp(self):
        """
        Custom setUp function to go into a temporary folder
        and start the needed programs.
        """
        #: use a temporary folder for testing
        self.test_dir = tempfile.mkdtemp()
        #: remember current working directory
        self.previous_dir = os.getcwd()
        os.chdir(self.test_dir)

        #: dict for all started programs
        self.started_programs = dict()
        for name, command in self.needed_programs.items():
            self.started_programs[name] = subprocess.Popen(command, start_new_session=True)
            self.assertIsRunning(name)

        atexit._clear()
        atexit.register(self.tearDown)

    def tearDown(self):
        """
        Custom tearDown function to kill the started programs if still present
        and remove the temporary folder again.
        """
        for name, process in self.started_programs.items():
            if self._is_running(name):
                os.killpg(process.pid, signal.SIGKILL)
            process.wait()
        os.chdir(self.previous_dir)
        shutil.rmtree(self.test_dir)

        atexit._clear()

    def _is_running(self, name):
        """
        Check if a given program is still running.
        """
        process = self.started_programs[name]
        pid, sts = process._try_wait(os.WNOHANG)
        assert pid == process.pid or pid == 0
        return pid == 0

    def assertIsDown(self, name, timeout=5, minimum_delay=0.1):
        """
        Test helper to assert the given program has terminated - at least after timeout in seconds has passed.
        Checks every "minimal_delay seconds.
        """
        endtime = time() + timeout
        while True:
            if not self._is_running(name):
                return

            remaining = endtime - time()
            self.assertFalse(remaining <= 0)

            sleep(minimum_delay)

    def assertIsRunning(self, name):
        """
        Assert that a given program is still running.
        """
        self.assertTrue(self._is_running(name))

    @staticmethod
    def create_socket(port, socket_type=zmq.DEALER, identity="socket", bind=False):
        """
        Create and return a ZMQ socket with the given type and identity and
        bind or connect it to localhost and the given port.
        """
        socket = HLTZMQTestCase.ctx.socket(socket_type)
        socket.rcvtimeo = 10000
        socket.linger = 0
        if identity:
            socket.setsockopt_string(zmq.IDENTITY, identity)
        if bind:
            if port is None:
                port = socket.bind_to_random_port("tcp://*")
                return socket, port
            else:
                socket.bind(f"tcp://*:{port}")
        else:
            if port is None:
                raise RuntimeError("Cannot connect to unknown port")

            socket.connect(f"tcp://localhost:{port}")

        return socket

    @staticmethod
    def create_router_socket(port):
        """
        Shortcut to create a ROUTER type socket with the typical parameters
        binding to the given port.
        """
        return HLTZMQTestCase.create_socket(port, socket_type=zmq.ROUTER, identity="", bind=True)

    @staticmethod
    def send(socket, message_type, first_data=b"", second_data=b"", identity=""):
        """
        Send a message consisting of the message type, the first and the second data
        either to the identity if given or without identity if omitted.
        """
        if identity:
            socket.send_multipart([identity.encode(), message_type.encode(), first_data, second_data])
        else:
            socket.send_multipart([message_type.encode(), first_data, second_data])

    @staticmethod
    def recv(socket):
        """
        Try to receive a message from the socket (or throw an assertion error if none comes after the set timeout
        of the socket).
        """
        try:
            return socket.recv_multipart()
        except zmq.error.Again:
            raise AssertionError("No answer from socket")

    def assertMonitoring(self, socket, search_key, search_value, timeout=10):
        """
        Ask the given socket for a monitoring JSON and make sure, the value related to "search_key"
        is set to "search_value" - at least after the given timeout.
        The search key can should be in the form "<category>.<key>".
        """
        end_time = time() + timeout
        monitoring = dict()
        while time() < end_time:
            HLTZMQTestCase.send(socket, "m")
            answer = self.assertIsAndGet(socket, "c")

            dict_monitoring = json.loads(answer[1])
            for parent_key, parent_dict in dict_monitoring.items():
                for key, value in parent_dict.items():
                    monitoring[parent_key + "." + key] = value

            if search_key in monitoring and monitoring[search_key] == search_value:
                break
        else:
            if monitoring:
                if search_key not in monitoring:
                    raise AssertionError(f"Monitoring did not have a result with key {search_key}")
                else:
                    raise AssertionError(
                        f"Monitoring did not show the result {search_value} for {search_key}, instead {monitoring[search_key]}")
            else:
                raise AssertionError(f"Monitoring did not answer in time.")

        self.assertNothingMore(socket)

    def assertIsAndGet(self, socket, message_type, final=True, router=False):
        """
        Assert that the next message received on the socket has the given message type.
        If final is set to True, also assert that there is no additional message on the socket.
        Use router only for router sockets.
        """
        answer = HLTZMQTestCase.recv(socket)
        type_index = 0
        if router:
            type_index = 1
        self.assertEqual(answer[type_index], message_type.encode())
        if final:
            self.assertNothingMore(socket)
        return answer

    def assertIsMsgType(self, socket, message_type, final=True, router=False):
        """
        Deprecated copy of "assertIsAndGet".
        """
        return self.assertIsAndGet(socket, message_type, final=final, router=router)

    def assertNothingMore(self, socket):
        """
        Assert that there is no pending message to be received on the socket.
        """
        self.assertFalse(socket.poll(0))

    def assertHasOutputFile(self, output_file, unlink=True, timeout=0.5, minimum_delay=0.1):
        """
        Assert that - at least after the given timeout - the output file
        is present. If unlink is set to True, remove the file after checking.
        """
        endtime = time() + timeout

        while True:
            if os.path.exists(output_file):
                if unlink:
                    os.unlink(output_file)
                return

            remaining = endtime - time()
            self.assertFalse(remaining <= 0)

            sleep(minimum_delay)

    def assertNotHasOutputFile(self, output_file, timeout=0.5):
        """
        Assert that after the timeout the given file is not present
        (a.k.a. no process has created it)
        """
        sleep(timeout)
        self.assertFalse(os.path.exists(output_file))


class BaseCollectorTestCase(HLTZMQTestCase):
    """
    As the collectors are mostly equal, use a common base test case class
    """
    #: final_collector
    final_collector = False

    def setUp(self):
        """Setup port numbers and necessary programs"""
        #: input_port
        self.input_port = HLTZMQTestCase.get_free_port()
        #: output_port
        self.output_port = HLTZMQTestCase.get_free_port()
        #: monitoring_port
        self.monitoring_port = HLTZMQTestCase.get_free_port()

        command = "b2hlt_finalcollector" if self.final_collector else "b2hlt_collector"
        output = "localhost" if self.final_collector else "*"
        self.needed_programs = {
            "collector": [
                command,
                "--input", f"tcp://*:{self.input_port}",
                "--output", f"tcp://{output}:{self.output_port}",
                "--monitor", f"tcp://*:{self.monitoring_port}"
            ]
        }
        # programs are setup, call parent setup function now
        super().setUp()

    def create_output_socket(self):
        """create the output socket depending if final collector or not"""
        if self.final_collector:
            output_socket = self.create_socket(self.output_port, socket_type=zmq.STREAM, bind=True)
            output_socket.send(b"")
            self.recv(output_socket)
        else:
            output_socket = self.create_socket(self.output_port)
            self.send(output_socket, "r")
        return output_socket

    def get_signal(self, output_socket, signal_type):
        """get a signal from the socket depending if final collector or not"""
        if self.final_collector:
            self.assertNothingMore(output_socket)
        else:
            self.assertIsMsgType(output_socket, signal_type, final=True)

    def get_event(self, output_socket):
        """get an event from the socket depending if final collector or not"""
        if self.final_collector:
            self.recv(output_socket)
            self.assertNothingMore(output_socket)
        else:
            self.assertIsMsgType(output_socket, "u")

    def testHelloAndMessageTransmission(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        # Register first worker
        input_socket = self.create_socket(self.input_port)
        self.send(input_socket, "h")

        # However the collector is only polling for ready now...
        self.assertNothingMore(input_socket)
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 0)

        # To make it actually look for input messages, it needs at least a single worker
        output_socket = self.create_output_socket()

        # Now we can go on
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 1)

        # Register second worker
        second_input_socket = self.create_socket(self.input_port, identity="other_socket")
        self.send(second_input_socket, "h")
        self.assertIsMsgType(second_input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 2)

        # So far no stop messages should be there
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)

        # the first stop message should not trigger a transmission
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)
        self.assertNothingMore(output_socket)

        # The second stop message should
        self.send(second_input_socket, "l")
        self.assertIsMsgType(second_input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)

        self.get_signal(output_socket, "l")

        # Another stop message should not change anything
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 2)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)
        self.assertNothingMore(output_socket)

        # But if we reset...
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)

        # .. it should
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.send(second_input_socket, "l")
        self.assertIsMsgType(second_input_socket, "c")

        self.get_signal(output_socket, "l")

        # Now we reset again
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)

        # send just one stop message
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")

        # and unregister the second
        self.send(second_input_socket, "d", b"other_socket")
        self.assertIsMsgType(second_input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 1)

        # which should also give us a stop message
        self.get_signal(output_socket, "l")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)

        # lets register and reset it again
        self.send(second_input_socket, "h")
        self.assertIsMsgType(second_input_socket, "c")
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 2)

        # and try the other way round: first unregister, then send stop
        self.send(second_input_socket, "d", b"other_socket")
        self.assertIsMsgType(second_input_socket, "c")
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 1)
        # which should also give us a stop message
        self.get_signal(output_socket, "l")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", True)

        # reset the state
        self.send(second_input_socket, "h")
        self.assertIsMsgType(second_input_socket, "c")
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_stop_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_stop_messages", False)
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 2)

        # The same applies to terminate messages:
        # Nothing at the beginning
        self.assertMonitoring(monitoring_socket, "input.received_terminate_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_terminate_messages", False)

        # the first terminate message should not trigger a transmission
        self.send(input_socket, "x")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_terminate_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_terminate_messages", False)
        self.assertNothingMore(output_socket)

        # Another terminate message should not change anything
        self.send(input_socket, "x")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.received_terminate_messages", 1)
        self.assertMonitoring(monitoring_socket, "input.all_terminate_messages", False)
        self.assertNothingMore(output_socket)

        # But if we reset...
        self.send(monitoring_socket, "n")
        self.assertMonitoring(monitoring_socket, "input.received_terminate_messages", 0)
        self.assertMonitoring(monitoring_socket, "input.all_terminate_messages", False)

        # ... and send again ...
        self.send(input_socket, "x")
        self.assertIsMsgType(input_socket, "c")
        self.send(second_input_socket, "x")
        self.assertIsMsgType(second_input_socket, "c")

        self.get_signal(output_socket, "x")

        # ... and the collector should have terminated
        self.assertIsDown("collector")

        # TODO: test and implement: timeout in wait for stop or terminate messages

    def testWrongRegistration(self):
        """test function"""
        # To make it actually look for input messages, it needs at least a single worker
        output_socket = self.create_output_socket()

        # Register first worker
        input_socket = self.create_socket(self.input_port)
        self.send(input_socket, "h")
        self.assertIsMsgType(input_socket, "c")

        # Send with a second, unregistered worker
        second_input_socket = self.create_socket(self.input_port, identity="other_socket")
        self.send(second_input_socket, "l")

        # The collector should die
        self.assertIsDown("collector")

    def testEventPropagation(self):
        """test function"""
        monitoring_socket = self.create_socket(self.monitoring_port)

        # Send two ready messages from the first socket
        output_socket = self.create_output_socket()
        if not self.final_collector:
            self.send(output_socket, "r")

            self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 2)
            self.assertMonitoring(monitoring_socket, "output.registered_workers", 1)

        if not self.final_collector:
            # Send two ready message from the second socket (the last one is needed to keep the collector listening)
            second_output_socket = self.create_socket(self.output_port, identity="other_socket")
            self.send(second_output_socket, "r")
            self.send(second_output_socket, "r")

            self.assertMonitoring(monitoring_socket, "output.ready_queue_size", 4)
            self.assertMonitoring(monitoring_socket, "output.registered_workers", 2)

        # Register two workers
        input_socket = self.create_socket(self.input_port)
        self.send(input_socket, "h")
        self.assertIsMsgType(input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 1)

        second_input_socket = self.create_socket(self.input_port, identity="other_socket")
        self.send(second_input_socket, "h")
        self.assertIsMsgType(second_input_socket, "c")
        self.assertMonitoring(monitoring_socket, "input.registered_workers", 2)

        # The first event should go to the first worker
        self.send(input_socket, "u", b"event data")
        self.assertIsMsgType(input_socket, "c")

        self.get_event(output_socket)
        if not self.final_collector:
            self.assertNothingMore(second_output_socket)

        # The second also
        self.send(second_input_socket, "u", b"event data")
        self.assertIsMsgType(second_input_socket, "c")

        self.get_event(output_socket)
        if not self.final_collector:
            self.assertNothingMore(second_output_socket)

        # But the third to the second worker
        self.send(input_socket, "u", b"event data")
        self.assertIsMsgType(input_socket, "c")

        if not self.final_collector:
            self.assertNothingMore(output_socket)
            self.get_event(second_output_socket)
        else:
            self.get_event(output_socket)

        # A stop message should be sent to all workers
        self.send(input_socket, "l")
        self.assertIsMsgType(input_socket, "c")

        # But only if it is complete...
        self.assertNothingMore(output_socket)
        if not self.final_collector:
            self.assertNothingMore(second_output_socket)

        self.send(second_input_socket, "l")
        self.assertIsMsgType(second_input_socket, "c")

        self.get_signal(output_socket, "l")
        if not self.final_collector:
            self.get_signal(second_output_socket, "l")

        # TODO: Test and implement: should not transmit events after stop

        # As well as a terminate message
        self.send(input_socket, "x")
        self.assertIsMsgType(input_socket, "c")

        self.assertNothingMore(output_socket)
        if not self.final_collector:
            self.assertNothingMore(second_output_socket)

        self.send(second_input_socket, "x")
        self.assertIsMsgType(second_input_socket, "c")

        self.get_signal(output_socket, "x")
        if not self.final_collector:
            self.get_signal(second_output_socket, "x")

        self.assertIsDown("collector")
