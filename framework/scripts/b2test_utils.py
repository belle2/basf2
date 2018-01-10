#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
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
    and not return"""
    print("TEST SKIPPED: %s" % reason, file=sys.stderr, flush=True)
    sys.exit(1)


@contextmanager
def set_loglevel(loglevel):
    """temporarily set the log level to something different"""
    old_loglevel = basf2.logging.log_level
    basf2.set_log_level(loglevel)
    try:
        yield
    finally:
        basf2.set_log_level(old_loglevel)


@contextmanager
def show_only_errors():
    """temporarily set the log level to ERROR

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
    directory fails a FileNotFoundError will be raised.
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


def safe_process(*args, **kwargs):
    """Run basf2.process with the given path in a child process.
    This avoids side effects (safe_process can be called multiple times safely)
    and doesn't kill this script even if a segfault or FATAL occurs during
    processing.

    It will return the exitcode of the processing which should be 0 in case of no error
    """
    process = multiprocessing.Process(target=basf2.process, args=args, kwargs=kwargs)
    process.start()
    process.join()
    return process.exitcode
