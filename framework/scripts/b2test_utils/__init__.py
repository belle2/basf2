#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

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
from collections import OrderedDict
import multiprocessing
import basf2
import subprocess
import re
from b2test_utils import logfilter


def skip_test(reason, py_case=None):
    """Skip a test script with a given reason. This function will end the script
    and not return.

    This is intended for scripts to be run in :ref:`b2test-scripts
    <b2test-scripts>` and will flag the script as skipped with the given reason
    when tests are executed.

    Useful if the test depends on some external condition like a web service and
    missing this dependency should not fail the test run.

    Parameters:
        reason (str): the reason to skip the test.
        py_case (unittest.TestCase): if this is to be skipped within python's
            native unittest then pass the TestCase instance
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
        data_type (str): case insensitive data type to find.  Either empty string or one of ``"examples"`` or ``"validation"``.
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
    temporarily set the log level to the specified `LogLevel <basf2.LogLevel>`. This returns a
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


def configure_logging_for_tests(user_replacements=None):
    """
    Change the log system to behave a bit more appropriately for testing scenarios:

    1. Simplify log message to be just ``[LEVEL] message``
    2. Disable error summary, just additional noise
    3. Intercept all log messages and replace

        * the current working directory in log messaged with ``${cwd}``
        * the current default globaltags with ``${default_globaltag}``
        * the contents of the following environment variables with their name
          (or the listed replacement string):

            - :envvar:`BELLE2_TOOLS`
            - :envvar:`BELLE2_RELEASE_DIR` with ``BELLE2_SOFTWARE_DIR``
            - :envvar:`BELLE2_LOCAL_DIR` with ``BELLE2_SOFTWARE_DIR``
            - :envvar:`BELLE2_EXTERNALS_DIR`
            - :envvar:`BELLE2_VALIDATION_DATA_DIR`
            - :envvar:`BELLE2_EXAMPLES_DATA_DIR`
            - :envvar:`BELLE2_BACKGROUND_DIR`

    Parameters:
        user_replacements (dict(str, str)): Additional strings and their replacements to replace in the output

    Warning:
        This function should be called **after** switching directory to replace the correct directory name

    .. versionadded:: release-04-00-00
    """
    basf2.logging.reset()
    basf2.logging.enable_summary(False)
    basf2.logging.enable_python_logging = True
    basf2.logging.add_console()
    # clang prints namespaces differently so no function names. Also let's skip the line number,
    # we don't want failing tests just because we added a new line of code. In fact, let's just see the message
    for level in basf2.LogLevel.values.values():
        basf2.logging.set_info(level, basf2.LogInfo.LEVEL | basf2.LogInfo.MESSAGE)

    # now create dictionary of string replacements. Since each key can only be
    # present once order is kind of important so the less portable ones like
    # current directory should go first and might be overridden if for example
    # the BELLE2_LOCAL_DIR is identical to the current working directory
    replacements = OrderedDict()
    replacements[", ".join(basf2.conditions.default_globaltags)] = "${default_globaltag}"
    # Let's be lazy and take the environment variables from the docstring so we don't have to repeat them here
    for env_name, replacement in re.findall(":envvar:`(.*?)`(?:.*``(.*?)``)?", configure_logging_for_tests.__doc__):
        if not replacement:
            replacement = env_name
        if env_name in os.environ:
            # replace path from the environment with the name of the variable. But remove a trailing slash or whitespace so that
            # the output doesn't depend on whether there is a tailing slash in the environment variable
            replacements[os.environ[env_name].rstrip('/ ')] = f"${{{replacement}}}"
    if user_replacements is not None:
        replacements.update(user_replacements)
    # add cwd only if it doesn't overwrite anything ...
    replacements.setdefault(os.getcwd(), "${cwd}")
    sys.stdout = logfilter.LogReplacementFilter(sys.__stdout__, replacements)


@contextmanager
def working_directory(path):
    """temporarily change the working directory to path

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


def check_error_free(tool, toolname, package, filter=lambda x: False, toolopts=None):
    """Calls the ``tool`` with argument ``package`` and check that the output is
    error-free. Optionally ``filter`` the output in case of error messages that
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
        filter: function which gets called for each line of output and
           if it returns True the line will be ignored.
        toolopts(list(str)): extra options to pass to the tool.
    """

    if "BELLE2_RELEASE_DIR" in os.environ:
        skip_test("Central release is setup")
    if "BELLE2_LOCAL_DIR" not in os.environ:
        skip_test("No local release is setup")

    args = [tool]
    if toolopts:
        args += toolopts
    if package is not None:
        args += [package]

    with local_software_directory():
        try:
            output = subprocess.check_output(args, encoding="utf8")
        except subprocess.CalledProcessError as error:
            print(error)
            output = error.output

    clean_log = [e for e in output.splitlines() if e and not filter(e)]
    if len(clean_log) > 0:
        subject = f"{package} package" if package is not None else "repository"
        print(f"""\
The {subject} has some {toolname} issues, which is now not allowed.
Please run:

  $ {" ".join(args)}

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


def skip_test_if_light(py_case=None):
    """
    Skips the test if we are running in a light build (maybe this test tests
    some generation example or whatever)

    Parameters:
        py_case (unittest.TestCase): if this is to be skipped within python's
            native unittest then pass the TestCase instance
    """
    try:
        import generators  # noqa
    except ModuleNotFoundError:
        skip_test(reason="We're in a light build.", py_case=py_case)


def print_belle2_environment():
    """
    Prints all the BELLE2 environment variables on the screen.
    """
    basf2.B2INFO('The BELLE2 environment variables are:')
    for key, value in sorted(dict(os.environ).items()):
        if 'BELLE2' in key.upper():
            print(f'  {key}={value}')


def is_ci() -> bool:
    """
    Returns true if we are running a test on our CI system (currently bamboo).
    The 'BELLE2_IS_CI' environment variable is set on CI only when the unit
    tests are run.
    """
    return os.environ.get("BELLE2_IS_CI", "no").lower() in [
        "yes",
        "1",
        "y",
        "on",
    ]
