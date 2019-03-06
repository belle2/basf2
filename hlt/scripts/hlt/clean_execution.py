import atexit
import signal
import subprocess
from time import sleep, time
import os
import basf2


class CleanBasf2Execution:
    """
    Helper class to call a given (basf2) command via subprocess
    and make sure the process is killed properly once a SIGINT or SIGTERM signal is
    send to the main process.
    To do this, the basf2 command is started in a new session group, so all child processes
    of the basf2 command will also be killed.

    When the main process receives a termination request via an SIGINT or SIGTERM,
    a SIGINT is sent to the started basf2 process.
    If the process is still alive after a given timeout (10 s by default),
    it is killed via SIGKILL and all its started child forks with it.
    After a normal or abnormal termination, the run() function returns the exit code
    and cleanup can happen afterwards.

    ATTENTION: In some rare cases, e.g. when the terminate request happens during a syscall,
    the process can not be stopped (see uninterruptable sleep process state, e.g. in
    https://stackoverflow.com/questions/223644/what-is-an-uninterruptable-process).
    In those cases, even a KILL signal does not help!

    The class can be used in a typical main method, e.g.

        from hlt.clean_execution import CleanBasf2Execution

        if __name__ == "__main__":
            execution = CleanBasf2Execution()
            try:
                execution.start(["basf2", "run.py"])
                execution.wait()
            finally:
                # Make sure to always do the cleanup, also in case of errors
                print("Do cleanup")

    """

    def __init__(self):
        """
        Create a new execution with the given parameters (list of arguments)
        """
        self._handled_processes = []
        self._handled_commands = []

    def start(self, command):
        """
        Add the execution and terminate gracefully/hard if requested via signal.
        """
        process = subprocess.Popen(command, start_new_session=True)
        pgid = os.getpgid(process.pid)
        if pgid != process.pid:
            basf2.B2WARNING("Subprocess is not session leader. Weird")

        self.install_signal_handler()
        self._handled_processes.append(process)
        self._handled_commands.append(command)

    def wait(self):
        """
        Wait until all handled calculations have finished.
        """
        while True:
            for command, process in zip(self._handled_commands, self._handled_processes):
                if self.has_process_ended(process):
                    basf2.B2RESULT("The process ", command, " died with ", process.returncode,
                                   ". Killing the remaining ones.")
                    self.kill()
                    return process.returncode
            sleep(1)

    def signal_handler(self, signal_number, signal_frame):
        """
        The signal handler called on SIGINT and SIGTERM.
        """
        self.kill()

    def kill(self):
        """
        Clean or hard shutdown of all processes.
        It tries to kill the process gracefully but if it does not react after a certain time,
        it kills it with a SIGKILL.
        """
        if not self._handled_processes:
            basf2.B2WARNING("Signal handler called without started process. This normally means, something is wrong!")
            return

        basf2.B2RESULT("Termination requested...")

        # Make sure this signal handle is not called more than once
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        signal.signal(signal.SIGTERM, signal.SIG_IGN)

        try:
            # Send a graceful stop signal to the process and give it some time to react
            for process in self._handled_processes:
                try:
                    process.send_signal(signal.SIGINT)
                    process.poll()
                except ProcessLookupError:
                    # The process is already gone! Nice
                    pass

            # Give the process some time to react
            if not self.wait_for_process(timeout=10):
                basf2.B2WARNING("Process did not react in time. Sending a SIGKILL.")
        finally:
            # In any case: kill the process
            for process in self._handled_processes:
                try:
                    os.killpg(process.pid, signal.SIGKILL)
                    if not self.wait_for_process(timeout=10, process_list=[process]):
                        backtrace = subprocess.check_output(["gdb", "-q",  "-batch", "-ex", "backtrace",  "basf2",
                                                             str(process.pid)]).decode()
                        basf2.B2ERROR("Could not end the process event with a KILL signal. "
                                      "This can happen because it is in the uninterruptable sleep state. "
                                      "I can not do anything about this!",
                                      backtrace=backtrace)
                except ProcessLookupError:
                    # The process is already gone! Nice
                    pass
                basf2.B2RESULT("...Process stopped")

            # And reinstall the signal handlers
            self.install_signal_handler()

    def wait_for_process(self, process_list=None, timeout=10, minimum_delay=0.0005):
        """
        Wait maximum "timeout" for the process to stop.
        If it did not end in this period, returns False.
        """
        if process_list is None:
            process_list = self._handled_processes

        endtime = time() + timeout
        while True:
            all_finished = all(self.has_process_ended(process) for process in process_list)
            if all_finished:
                return True

            remaining = endtime - time()
            if remaining <= 0:
                return False

            delay = min(minimum_delay, remaining)
            sleep(delay)

    def install_signal_handler(self):
        """
        Set the signal handlers for SIGINT and SIGTERM to out own one.
        """
        signal.signal(signal.SIGINT, self.signal_handler)
        signal.signal(signal.SIGTERM, self.signal_handler)
        # Just for safety, also register an exit handler
        atexit.unregister(self.signal_handler)
        atexit.register(self.signal_handler, signal.SIGTERM, None)

    @staticmethod
    def has_process_ended(process):
        """
        Check if the handled process has ended already.
        This functions does not wait.

        I would rather use self._handled_process.wait() or poll()
        which does exactly the same.
        However: the main process is also waiting for the return code
        so the threading.lock in the .wait() function will never aquire a lock :-(
        """
        pid, sts = process._try_wait(os.WNOHANG)
        assert pid == process.pid or pid == 0

        if pid == process.pid:
            return True

        return False
