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
    Context manager providing page-wise output using ``less`` for
    some Python code. Output is delayed until all commands are
    finished.

    >>> with Pager():
    >>>     for i in range(30):
    >>>         print("This is an example on how to use the pager.")

    @param prompt a string argument allows overriding
                  the description provided by ``less``. Special characters may need
                  escaping. By default, the temporary filename is shown.
    @param quit_if_one_screen a bool indicating whether the Pager should quit
        automatically if the content fits on one screen. This implies that the
        content stays visible on pager exit. True is similar to the behavior of
        :program:`git diff`, False is similar to :program:`git --help`
    """

    def __init__(self, prompt=None, quit_if_one_screen=False):
        """ constructor just remembering the arguments """
        #: pager program to use
        self.pager = os.environ.get("PAGER", "less")
        # treat "cat" as no pager at all
        if self.pager == "cat":
            self.pager = ""
        #: prompt string
        self.prompt = prompt
        #: flag indicating whether the pager should automatically exit if the
        # content fits on one screen
        self.quit_if_one_screen = quit_if_one_screen
        #: Pager subprocess
        self._pager_process = None
        #: Original file descriptor for stdout before entering the context
        self._original_stdout_fd = os.dup(sys.stdout.fileno())
        #: Original file descriptor for stderr before entering the context
        self._original_stderr_fd = os.dup(sys.stderr.fileno())
        #: Original sys.__stdout__ before entering the context
        self._original_stdout = None
        #: Original sys.__stderr__ before entering the context
        self._original_stderr = None

    def __enter__(self):
        """ entering context """
        if not sys.stdout.isatty() or self.pager == "":
            return

        # save old sys.__stderr__ and sys.__stdout__ objects
        self._original_stderr = sys.__stderr__
        self._original_stderr = sys.__stdout__
        # and duplicate the curent output file descriptors
        self._original_stdout_fd = os.dup(sys.stdout.fileno())
        self._original_stderr_fd = os.dup(sys.stderr.fileno())

        # this is a bit annoying: Usually in python the __stdout__ and
        # sys.__stderr__ point to the original stdout/stderr. However we modify
        # the file descriptors for those so they acutually will not, they will
        # also be redirected. Butquerying the terminal size looks as
        # sys.__stdout__ which is no longer a tty ...
        # So lets provide objects pointing to the original file descriptors
        sys.__stdout__ = io.TextIOWrapper(os.fdopen(self._original_stdout_fd, "wb"))
        sys.__stderr__ = io.TextIOWrapper(os.fdopen(self._original_stdout_fd, "wb"))

        # fine, everything is saved, start the pager
        pager_cmd = [self.pager]
        if self.pager == "less":
            if self.prompt is None:
                self.prompt = ''  # same as default prompt
            self.prompt += ' (press h for help or q to quit)'
            pager_cmd += ['-R', '-Ps' + self.prompt.strip()]
            if self.quit_if_one_screen:
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
