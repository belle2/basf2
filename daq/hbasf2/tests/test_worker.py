import atexit
import os
from pathlib import Path
from time import sleep
from unittest import main

from zmq_daq.test_support import HLTZMQTestCase


class WorkerTestCase(HLTZMQTestCase):
    """Test case"""
    #: input_port
    input_port = HLTZMQTestCase.get_free_port()
    #: output_port
    output_port = HLTZMQTestCase.get_free_port()
    #: needed_programs
    needed_programs = {"worker": ["python3", os.path.abspath("passthrough.py"),
                                  "--input", f"tcp://localhost:{input_port}",
                                  "--output", f"tcp://localhost:{output_port}"]}

    #: event_data
    event_data = open(os.path.abspath("out.raw"), "br").read()

    def start(self):
        """start the needed sockets and send some hello messages"""
        #: input_socket
        self.input_socket = self.create_router_socket(self.input_port)
        #: output_socket
        self.output_socket = self.create_router_socket(self.output_port)

        # There should be a hello message
        #: output_identity
        self.output_identity = self.assertIsMsgType(self.output_socket, "h", router=True)[0].decode()
        self.send(self.output_socket, "c", identity=self.output_identity)

        # There are probably many more ready messages, but we are only interested in at least one here
        #: input_identity
        self.input_identity = self.assertIsMsgType(self.input_socket, "r", router=True, final=False)[0].decode()

        # Store some example events
        #: some data
        self.first_run_event_data = [self.event_data, self.event_data]
        #: some data
        self.second_run_event_data = [self.event_data, self.event_data]

    def testInitialization(self):
        """test function"""
        self.start()

        # Initialisation should be called
        self.assertHasOutputFile("initialize_called", timeout=1)

    def testRunSending(self):
        """test function"""
        self.start()

        # Send first event (should trigger begin run again)
        self.send(self.input_socket, "u", first_data=self.first_run_event_data[0], identity=self.input_identity)
        self.assertHasOutputFile("beginrun_called", timeout=0.5)
        self.assertIsMsgType(self.output_socket, "w", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # Send second event (should not trigger begin run again)
        self.send(self.input_socket, "u", first_data=self.first_run_event_data[1], identity=self.input_identity)
        self.assertNotHasOutputFile("beginrun_called", timeout=0.5)
        self.assertIsMsgType(self.output_socket, "w", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

    def testEndRun(self):
        """test function"""
        self.start()

        # TODO: do I want to test which run was ended?
        # end run trigger
        self.send(self.input_socket, "l", identity=self.input_identity)
        self.assertHasOutputFile("endrun_called", timeout=2)
        self.assertIsMsgType(self.output_socket, "l", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # Also the second one should give us an end run
        self.send(self.input_socket, "l", identity=self.input_identity)
        self.assertHasOutputFile("endrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "l", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # Sneak in an event in between -> should give beginRun
        self.send(self.input_socket, "u", first_data=self.first_run_event_data[0], identity=self.input_identity)
        self.assertHasOutputFile("beginrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "w", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # And end the run again
        self.send(self.input_socket, "l", identity=self.input_identity)
        self.assertHasOutputFile("endrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "l", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # A second time...
        self.send(self.input_socket, "l", identity=self.input_identity)
        self.assertHasOutputFile("endrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "l", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # Sneak in a second event in between -> should give beginRun (as it is a new run)
        self.send(self.input_socket, "u", first_data=self.second_run_event_data[0], identity=self.input_identity)
        self.assertHasOutputFile("beginrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "w", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # And end the run again
        self.send(self.input_socket, "l", identity=self.input_identity)
        self.assertHasOutputFile("endrun_called", timeout=1)
        self.assertIsMsgType(self.output_socket, "l", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)

        # Termination should also work
        self.send(self.input_socket, "x", identity=self.input_identity)
        # Attention: terminate is called in the different order
        self.assertIsMsgType(self.output_socket, "x", router=True)
        self.send(self.output_socket, "c", identity=self.output_identity)
        self.assertHasOutputFile("terminate_called", timeout=1)

        # And the termination should cause the process to go down
        self.assertIsDown("worker", timeout=200)


class DyingWorkerTestCase(HLTZMQTestCase):
    """Test case"""
    #: input_port
    input_port = HLTZMQTestCase.get_free_port()
    #: output_port
    output_port = HLTZMQTestCase.get_free_port()
    #: needed_programs
    needed_programs = {"dying_worker": ["python3", os.path.abspath("passthrough.py"),
                                        "--exit", "--prefix", "dying_",
                                        "--input", f"tcp://localhost:{input_port}",
                                        "--output", f"tcp://localhost:{output_port}"]}

    #: event_data
    event_data = open(os.path.abspath("out.raw"), "br").read()

    def start(self):
        """start the needed sockets and send some hello messages"""
        #: input_socket
        self.input_socket = self.create_router_socket(self.input_port)
        #: output_socket
        self.output_socket = self.create_router_socket(self.output_port)

        # There should be a hello message
        #: output_identity
        self.output_identity = self.assertIsMsgType(self.output_socket, "h", router=True)[0].decode()
        self.send(self.output_socket, "c", identity=self.output_identity)

        # There are probably many more ready messages, but we are only interested in at least one here
        #: input_identity
        self.input_identity = self.assertIsMsgType(self.input_socket, "r", router=True, final=False)[0].decode()

    def testUnregistration(self):
        """test function"""
        self.start()

        # lets send some events
        for _ in range(10):
            self.send(self.input_socket, "u", first_data=self.event_data, identity=self.input_identity)
            self.assertIsMsgType(self.output_socket, "w", router=True)
            self.send(self.output_socket, "c", identity=self.output_identity)

        self.assertHasOutputFile("dying_beginrun_called", timeout=1)

        # Now we kill one of the workers
        Path("dying_exit_request").touch()

        self.send(self.input_socket, "u", first_data=self.event_data, identity=self.input_identity)
        self.assertHasOutputFile("dying_exit_called", timeout=1)
        msg = self.assertIsMsgType(self.output_socket, "d", router=True)
        # the message content should be the worker that has died
        self.assertEqual(msg[2].decode(), self.output_identity)
        self.send(self.output_socket, "c", identity=msg[0].decode())

        self.assertIsDown("dying_worker", timeout=10)


if __name__ == '__main__':
    main()
