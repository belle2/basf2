#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Tests basf2 behaviour under various POSIX signals
# different possibilities for termination
# - SIGKILL: immediate, no cleanup (won't test this for obvious reasons)
# - SIGINT= safe and slow: abort event processing quickly, but call processTerminate()
# - most signals: immediate, but IPC structures cleaned up (segmentation fault, or termination in initialize())
# This should also work in parallel processing.
# In initialize(), termination should always be immediate.


import os
from signal import *
import tempfile
import shutil
from basf2 import *

# we test for stray resources later, so let's clean up first
os.system('clear_basf2_ipc')

tmpdir = tempfile.mkdtemp(prefix='b2signal_test')

set_random_seed("something important")


def run_test(init_signal, event_signal, abort, test_in_process):
    """
    @param init_signal kill in initialize()
    @param event_signal kill in event()
    @param abort Should this test give a non-zero return code?
    @param test_in_process 0 input, 1 parallel, 2 output
    """

    testFile = tempfile.NamedTemporaryFile(dir=tmpdir, delete=False)

    # main thread just monitors exit status
    if os.fork() != 0:
        retbytes = os.wait()[1]
        retcode = (retbytes & 0xff00) >> 8  # high byte
        killsig = retbytes & 0x00ff % 128  # low byte minus core dump bit
        status_ok = False
        if abort:
            if killsig in (init_signal, event_signal):
                status_ok = True
            if retcode != 0 or killsig == SIGTERM:
                status_ok = True
        else:
            if killsig == 0 and retcode == 0:
                status_ok = True

        if not status_ok:
            print(killsig, retcode)
            if killsig:
                raise RuntimeError("Killed with wrong signal %d?" % (killsig))
            else:
                raise RuntimeError("Wrong exit code %d" % (retcode))

        fileExists = os.path.isfile(testFile.name)
        if fileExists and (not abort or event_signal == SIGINT):
            # is ROOT file ok?
            file_ok_ret = os.system('showmetadata ' + testFile.name)
            B2WARNING("file_ok_ret: " + str(file_ok_ret))
            if file_ok_ret != 0:
                raise RuntimeError("Root file not properly closed!")

        # clear_basf2_ipc shouldn't find anything to clean up
        ret = os.system('clear_basf2_ipc')
        if ret != 0:
            raise RuntimeError("Some IPC structures were not cleaned up")

        B2WARNING("test ok.")
        return

    # actual test
    num_events = 5
    if abort:
        num_events = int(1e8)  # larger number to test we abort early.
    eventinfosetter = register_module('EventInfoSetter', evtNumList=[num_events])
    output = register_module('RootOutput')
    output.param('outputFileName', testFile.name)
    output.param('updateFileCatalog', False)

    def sigquithandler(sig, frame):
        B2INFO('SIGQUIT handler called')
        B2FATAL("dummy abort")

    class TestModule(Module):
        """test"""

        def __init__(self):
            """init."""
            super().__init__()

        def initialize(self):
            """reimplementation of Module::initialize()."""
            # TODO investigating test failure on bamboo. (fails to die on SIGQUIT in single process.)
            prevsig = signal.signal(signal.SIGQUIT, sigquithandler)
            print('handler installed, previous handler was: ', prevsig)

            if init_signal:
                pid = os.getpid()
                B2INFO("Killing %s in init (sig %d)" % (pid, init_signal))
                os.kill(pid, init_signal)
            B2INFO("initialize()")

        def event(self):
            """reimplementation of Module::event()."""
            if init_signal:
                B2FATAL("Processing should have been stopped in init!")
            if event_signal:
                pid = os.getpid()
                B2INFO("Killing %s in event (sig %d)" % (pid, event_signal))
                os.kill(pid, event_signal)
            B2INFO("event()")

    # Create paths
    main = create_path()
    main.add_module(eventinfosetter)
    if test_in_process == 0:
        testmod = main.add_module(TestModule())
        main.add_module('ProgressBar').set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    elif test_in_process == 1:
        testmod = main.add_module(TestModule())
        testmod.set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
    elif test_in_process == 2:
        main.add_module('ProgressBar').set_property_flags(ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        testmod = main.add_module(TestModule())
    main.add_module(output)

    B2WARNING("Running with PID " + str(os.getpid()))
    process(main)
    sys.exit(0)


# set_log_level(LogLevel.WARNING)
for nproc in [0, 3]:
    set_nprocesses(nproc)
    for in_proc in [0, 1, 2]:
        if nproc == 0 and in_proc != 0:
            break  # running more tests in single process mode doesn't make sense
        B2WARNING("== starting tests with nproc=%d, test_in_process=%d" % (nproc, in_proc))

        try:
            run_test(None, None, abort=False, test_in_process=in_proc)
            if in_proc != 1:  # worker processes do not handle the events
                run_test(SIGINT, None, abort=True, test_in_process=in_proc)
                run_test(None, SIGINT, abort=True, test_in_process=in_proc)
                run_test(SIGTERM, None, abort=True, test_in_process=in_proc)
                run_test(None, SIGTERM, abort=True, test_in_process=in_proc)
                run_test(SIGQUIT, None, abort=True, test_in_process=in_proc)
                run_test(None, SIGQUIT, abort=True, test_in_process=in_proc)

            # crashes in any process should also result in reasonable cleanup
            run_test(SIGSEGV, None, abort=True, test_in_process=in_proc)
            run_test(None, SIGSEGV, abort=True, test_in_process=in_proc)

            # SIGPIPE would be nice, too. just stops immediately now
        except Exception as e:
            # Note: Without specifying exception type, we might get those from forked processes, too
            B2WARNING("Exception occured for nproc=%d, test_in_process=%d" % (nproc, in_proc))
            raise

set_log_level(LogLevel.INFO)


print("\n")
print("=========================================================================")
print("All signal tests finished successfully!")
print("=========================================================================")

shutil.rmtree(tmpdir)
