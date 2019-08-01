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
import subprocess
import unittest


def skip_test(reason, py_case=None):
    """Skip a test script with a given reason. This function will end the script
    and not return.

    This is intended for scripts to be run in :ref:`b2test-scripts
    <b2test-scripts>` and will flag the script as skipped with the given reason
    when tests are executed.

    Useful if the test depends on some external condition like a web service and
    missing this dependency should not fail the test run.
    """
    if py_case:
        py_case.skipTest(reason)
    else:
        print("TEST SKIPPED: %s" % reason, file=sys.stderr, flush=True)
    sys.exit(1)


def require_file(filename, data_type="", py_case=None):
    """Check for the existence of a test input file before attempting to open it.
    Skips the test if not found.

    Wraps `basf2.find_file` for use in test scripts run as
    :ref`b2test-scripts <b2test-scripts>`

    Parameters:
        filename (str): relative filename to look for, either in a central place or in the current working directory
        data_type (str): case insensitive data type to fine.  Either empty string or one of `""examples"`` or ``"validation"``.
        py_case (unittest.TestCase): if this is to be skipped within python's native unittest then pass the TestCase instance

    Returns:
        Full path to the test input file
    """
    try:
        fullpath = basf2.find_file(filename, data_type, silent=False)
    except FileNotFoundError as fnf:
        skip_test('Cannot find: %s' % fnf.filename, py_case)
    return fullpath


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


@contextmanager
def local_software_directory():
    """Context manager to make sure we are executed in the top software
    directory by switching to $BELLE2_LOCAL_DIR.

    >>> with local_software_directory():
    >>>    assert(os.listdir().contains("analysis"))
    """
    try:
        directory = os.environ["BELLE2_LOCAL_DIR"]
    except KeyError:
        raise RuntimeError("Cannot find local Belle 2 software directory, "
                           "have you setup the software correctly?")

    with working_directory(directory):
        yield directory


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
    return run_in_subprocess(target=basf2.process, *args, **kwargs)


def check_error_free(tool, toolname, package, filter=lambda x: False):
    """Calls the `tool` with argument `package` and check that the output is
    error-free. Optionally `filter` the output in case of error messages that
    can be ignored.

    In case there is some output left, then prints the error message and exits
    (failing the test).

    The test is only executed for a full local checkout: If the ``BELLE2_RELEASE_DIR``
    environment variable is set or if ``BELLE2_LOCAL_DIR`` is unset the test is
    skipped: The program exits with an appropriate message.

    Warnings:
        If the test is skipped or the test contains errors this function does
        not return but will directly end the program.

    Arguments:
        tool(str): executable to call
        toolname(str): human readable name of the tool
        package(str): package to run over. Also the first argument to the tool
        filter(lambda): function which gets called for each line of output and
           if it returns True the line will be ignored.
    """

    if "BELLE2_RELEASE_DIR" in os.environ:
        skip_test("Central release is setup")
    if "BELLE2_LOCAL_DIR" not in os.environ:
        skip_test("No local release is setup")

    with local_software_directory():
        try:
            output = subprocess.check_output([tool, package], encoding="utf8")
        except subprocess.CalledProcessError as error:
            print(error)
            output = error.output

    clean_log = [e for e in output.splitlines() if e and not filter(e)]
    if len(clean_log) > 0:
        print(f"""\
The {package} package has some {toolname} issues, which is now not allowed.
Please run:

  $ {tool} {package}

and fix any issues you have introduced. Here is what {toolname} found:\n""")
        print("\n".join(clean_log))
        sys.exit(1)


def get_streamer_checksums(objects):
    """
    Extract the version and streamer checksum of the C++ objects in the given list
    by writing them all to a TMemFile and getting back the streamer info list
    automatically created by ROOT afterwards.
    Please note, that this list also includes the streamer infos of all
    base objects of the objects you gave.

    Returns a dictionary object name -> (version, checksum).
    """
    import ROOT

    # Write out the objects to a mem file
    f = ROOT.TMemFile("test_mem_file", "RECREATE")
    f.cd()

    for o in objects:
        o.Write()
    f.Write()

    # Go through all streamer infos and extract checksum and version
    streamer_checksums = dict()
    for streamer_info in f.GetStreamerInfoList():
        if not isinstance(streamer_info, ROOT.TStreamerInfo):
            continue
        streamer_checksums[streamer_info.GetName()] = (streamer_info.GetClassVersion(), streamer_info.GetCheckSum())

    f.Close()
    return streamer_checksums


def get_object_with_name(object_name, root=None):
    """
    (Possibly) recursively get the object with the given name from the Belle2 namespace.

    If the object name includes a ".", the first part will be turned into an object (probably a module)
    and the function is continued with this object as the root and the rest of the name.

    If not, the object is extracted via a getattr call.
    """
    if root is None:
        from ROOT import Belle2
        root = Belle2

    if "." in object_name:
        namespace, object_name = object_name.split(".", 1)

        return get_object_with_name(object_name, get_object_with_name(namespace, root=root))

    return getattr(root, object_name)
