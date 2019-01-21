#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
pager
-----

Provides paginated output for Python code.
"""

import sys
import subprocess
import os
import io


class Pager(object):
    """
    Context manager providing page-wise output using ``less``, similar to how
    git handles long output of for example ``git diff``.  Paging will only be
    active if the output is to a terminal and not piped into a file or to a
    different program.

    Warning:
        To be able to see `basf2` log messages like `B2INFO() <basf2.B2INFO>`
        on the paged output you have to set
        `basf2.logging.enable_python_logging = True
        <basf2.LogPythonInterface.enable_python_logging>`

    .. versionchanged:: release-03-00-00
       the pager no longer waits until all output is complete but can
       incrementally show output. It can also show output generated in C++

    You can set the environment variable ``$PAGER`` to an empty string or to
    ``cat`` to disable paging or to a different program (for example ``more``)
    which should retrieve the output and display it.

    >>> with Pager():
    >>>     for i in range(30):
    >>>         print("This is an example on how to use the pager.")

    Parameters:
        prompt (str): a string argument allows overriding the description
            provided by ``less``. Special characters may need escaping.
            Will only be shown if paging is used and the pager is actually ``less``.
        quit_if_one_screen (bool): indicating whether the Pager should quit
            automatically if the content fits on one screen. This implies that
            the content stays visible on pager exit. True is similar to the
            behavior of :program:`git diff`, False is similar to :program:`git
            --help`
    """

    def __init__(self, prompt=None, quit_if_one_screen=False):
        """ constructor just remembering the arguments """
        #: pager program to use
        self._pager = os.environ.get("PAGER", "less")
        # treat "cat" as no pager at all
        if self._pager == "cat":
            self._pager = ""
        #: prompt string
        self._prompt = prompt
        #: flag indicating whether the pager should automatically exit if the
        # content fits on one screen
        self._quit_if_one_screen = quit_if_one_screen
        #: Pager subprocess
        self._pager_process = None
        #: Original file descriptor for stdout before entering the context
        self._original_stdout_fd = None
        #: Original file descriptor for stderr before entering the context
        self._original_stderr_fd = None
        #: Original sys.__stdout__ before entering the context
        self._original_stdout = None
        #: Original sys.__stderr__ before entering the context
        self._original_stderr = None

    def __enter__(self):
        """ entering context """
        if not sys.stdout.isatty() or self._pager == "":
            return

        # save old sys.__stderr__ and sys.__stdout__ objects
        self._original_stderr = sys.__stderr__
        self._original_stderr = sys.__stdout__
        try:
            # and duplicate the current output file descriptors
            self._original_stdout_fd = os.dup(sys.stdout.fileno())
            self._original_stderr_fd = os.dup(sys.stderr.fileno())
        except Exception:
            # jupyter notebook stdout/stderr objects don't have a fileno so
            # don't support paging
            return

        # This is a bit annoying: Usually in python the sys.__stdout__ and
        # sys.__stderr__ objects point to the original stdout/stderr on program start.
        #
        # However we modify the file descriptors directly so these objects will
        # also be redirected automatically. The documentation for
        # sys.__stdout__ says it "could be useful to print to the actual
        # standard stream no matter if the sys.std* object has been
        # redirected". Also,  querying the terminal size looks at
        # sys.__stdout__ which would no longer be pointing to a tty.
        #
        # So lets provide objects pointing to the original file descriptors so
        # that they behave as expected, i.e. as if we would only have
        # redirected sys.stdout and sys.stderr ...
        sys.__stdout__ = io.TextIOWrapper(os.fdopen(self._original_stdout_fd, "wb"))
        sys.__stderr__ = io.TextIOWrapper(os.fdopen(self._original_stdout_fd, "wb"))

        # fine, everything is saved, start the pager
        pager_cmd = [self._pager]
        if self._pager == "less":
            if self._prompt is None:
                self._prompt = ''  # same as default prompt
            self._prompt += ' (press h for help or q to quit)'
            pager_cmd += ['-R', '-Ps' + self._prompt.strip()]
            if self._quit_if_one_screen:
                pager_cmd += ['-F', '-X']
        self._pager_process = subprocess.Popen(pager_cmd + ["-"], restore_signals=True,
                                               stdin=subprocess.PIPE)
        # and attach stdout to the pager stdin
        pipe_fd = self._pager_process.stdin.fileno()
        # and if stderr was a tty do the same for stderr
        os.dup2(pipe_fd, sys.stdout.fileno())
        if sys.stderr.isatty():
            os.dup2(pipe_fd, sys.stderr.fileno())

    def __exit__(self, exc_type, exc_val, exc_tb):
        """ exiting context """
        sys.stdout.flush()
        # no pager, nothing to do
        if self._pager_process is None:
            return

        # restore output
        os.dup2(self._original_stdout_fd, sys.stdout.fileno())
        os.dup2(self._original_stderr_fd, sys.stderr.fileno())
        # and the original __stdout__/__stderr__ object just in case. Will also
        # close the copied file descriptors
        sys.__stderr__ = self._original_stderr
        sys.__stdout__ = self._original_stdout

        # wait for pager
        self._pager_process.communicate()


if __name__ == '__main__':
    import time
    with Pager():
        for i in range(30):
            print("This is an example on how to use the pager.")
            time.sleep(0.1)
