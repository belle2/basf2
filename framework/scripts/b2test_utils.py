#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
b2test_utils - Helper functions useful for test scripts
-------------------------------------------------------

This module contains functions which are commonly needed for tests like changing
log levels or switching to an empty working directory
"""

import sys
import os
import tempfile
from contextlib import contextmanager
import multiprocessing
import basf2


def skip_test(reason):
    """Skip a test script with a given reason. This function will end the script
    and not return.

    This is intended for scripts to be run in :ref:`b2test-scripts
    <b2test-scripts>` and will flag the script as skipped with the given reason
    when tests are executed.

    Useful if the test depends on some external condition like a web service and
    missing this dependency should not fail the test run.
    """
    print("TEST SKIPPED: %s" % reason, file=sys.stderr, flush=True)
    sys.exit(1)


@contextmanager
def set_loglevel(loglevel):
    """
    temporarily set the log level to the specified `LogLevel`. This returns a
    context manager so it should be used in a ``with`` statement:

    >>> with set_log_level(LogLevel.ERROR):
    >>>     # during this block the log level is set to ERROR
    """
    old_loglevel = basf2.logging.log_level
    basf2.set_log_level(loglevel)
    try:
        yield
    finally:
        basf2.set_log_level(old_loglevel)


@contextmanager
def show_only_errors():
    """temporarily set the log level to `ERROR <LogLevel.ERROR>`. This returns a
    context manager so it should be used in a ``with`` statement

    >>> with show_only_errors():
    >>>     B2INFO("this will not be shown")
    >>> B2INFO("but this might")
    """
    with set_loglevel(basf2.LogLevel.ERROR):
        yield


@contextmanager
def working_directory(path):
    """temprarily change the working directory to path

    >>> with working_directory("testing"):
    >>>     # now in subdirectory "./testing/"
    >>> # back to parent directory

    This function will not create the directory for you. If changing into the
    directory fails a `FileNotFoundError` will be raised.
    """
    dirname = os.getcwd()
    try:
        os.chdir(path)
        yield
    finally:
        os.chdir(dirname)


@contextmanager
def clean_working_directory():
    """Context manager to create a temporary directory and directly us it as
    current working directory. The directory will automatically be deleted after
    the with context is left.

    >>> with clean_working_directory() as dirname:
    >>>     # now we are in an empty directory, name is stored in dirname
    >>>     assert(os.listdir() == [])
    >>> # now we are back where we were before
    """
    with tempfile.TemporaryDirectory() as tempdir:
        with working_directory(tempdir):
            yield tempdir


def run_in_subprocess(*args, target, **kwargs):
    """Run the given ``target`` function in a child process using `multiprocessing.Process`

    This avoids side effects: anything done in the target function will not
    affect the current process. This is mostly useful for test scripts as
    ``target`` can emit a `FATAL <LogLevel.FATAL>` error without killing script execution.

    It will return the exitcode of the child process which should be 0 in case of no error
    """
    process = multiprocessing.Process(target=target, args=args, kwargs=kwargs)
    process.start()
    process.join()
    return process.exitcode


def safe_process(*args, **kwargs):
    """Run `basf2.process` with the given path in a child process using
    `multiprocessing.Process`

    This avoids side effects (`safe_process` can be safely called multiple times)
    and doesn't kill this script even if a segmentation violation or a `FATAL
    <LogLevel.FATAL>` error occurs during processing.

    It will return the exitcode of the child process which should be 0 in case of no error
    """
    run_in_subprocess(target=basf2.process, *args, **kwargs)
