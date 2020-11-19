from pathlib import Path
from unittest import main
import basf2

from zmq_daq.test_support import HLTZMQTestCase


class WorkerTestCase(HLTZMQTestCase):
    """Test case baseclass to spawn a worker"""
    #: event_data
    event_data = open(basf2.find_file("daq/hbasf2/tests/out.raw"), "br").read()

    #: extra arguments to pass to the worker script
    extra_arguments = []

    def setUp(self):
        """Setup necessary sockets and programs"""
        #: input_socket
        self.input_socket, input_port = self.create_router_socket(None)
        #: output_socket
        self.output_socket, output_port = self.create_router_socket(None)
        # and set the list of necessary programs to use these sockets
        self.needed_programs = {
            "worker": [
                "python3", basf2.find_file("daq/hbasf2/tests/passthrough.no_run_py"),
                "--input", f"tcp://localhost:{input_port}",
                "--output", f"tcp://localhost:{output_port}",
            ] + self.extra_arguments
        }
        super().setUp()

    def start(self):
        """start the needed sockets and send some hello messages"""
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


class NormalWorkerTestCase(WorkerTestCase):
    """Tests for normal worker behavior"""

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


class DyingWorkerTestCase(WorkerTestCase):
    """Test case for dying workers"""

    #: set the extra arguments we need ...
    extra_arguments = ["--exit", "--prefix", "dying_"]

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

        self.assertIsDown("worker", timeout=10)


if __name__ == '__main__':
    main()
