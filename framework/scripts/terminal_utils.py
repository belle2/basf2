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
import shutil
import tempfile


class Pager(object):
    """
    Context manager providing page-wise output using ``less`` for
    some Python code. Output is delayed until all commands are
    finished. Paging will only be active if the output is to a terminal and not
    piped into a file or to a different program.

    You can set the environment variable ``$PAGER`` to an empty string to
    disable paging or to a different program (for example ``more``) which should
    retrieve the output and display it.

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
        self._original_stdout_fd = None
        #: Original file descriptor for stderr before entering the context
        self._original_stderr_fd = None
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


def input_editor(initial_content=None, editor_command=None, commentlines_start_with=None):
    """Provide user input via opening a tmp file in a text editor.

    It is an alternative to the python commands ``input()`` or ``sys.stdin.readlines`` and is
    inspired by the behaviour of ``git commit`` for editing commit messages.  By using an editor
    instead of the command line, the user is motivated to give expressive multi-line input,
    leveraging the full text editing capabilities of his editor.  This function cannot be used for
    example in interactive terminal scripts, whenever detailed user input is required.

    Adapted from https://chase-seibert.github.io/blog/2012/10/31/python-fork-exec-vim-raw-input.html

    :param initial_content: Initial string to insert into the tmp file that is opened for user
        input.  Can be used for default input or to insert comment lines with instructions.
    :param editor_command: Provide editor to open for user input.  If not given, try
        $VISUAL/$EDITOR/vi(m)
    :param commentlines_start_with: Optionally define string with which comment lines start
    """
    # split editor_command in case it contains cli arguments/options seperated by whitespace
    editor_command = editor_command or get_editor()
    # split editor_command in case it contains cli arguments/options seperated by whitespace
    editor_command_list = editor_command.split()

    # check if editor in command string exists, otherwise prompt for new editor command
    while shutil.which(editor_command_list[0]) is None:
        print(f"Editor '{editor_command_list[0]}' not found.")
        if input("Enter new editor command (y/n) [y]? ").lower() not in ['y', '']:
            sys.exit(0)
        editor_command_list = input("Editor command to use instead: ").split()

    with tempfile.NamedTemporaryFile(mode='r+') as tmpfile:
        if initial_content:
            tmpfile.write(initial_content)
            tmpfile.flush()
        subprocess.check_call(editor_command_list + [tmpfile.name])
        tmpfile.seek(0)
        input_string = tmpfile.read().strip()

        # remove commented lines from input string
        if commentlines_start_with is not None:
            input_string = "\n".join([l for l in input_string.splitlines()
                                      if not l.startswith(commentlines_start_with)])
    return input_string


def get_editor():
    """
    Get editor command string from environment variables.
    """
    editor_command = (os.environ.get('VISUAL') or
                      os.environ.get('EDITOR') or
                      shutil.which('vim') or
                      shutil.which('vi'))
    return editor_command


if __name__ == '__main__':
    import time
    with Pager():
        for i in range(30):
            print("This is an example on how to use the pager.")
            time.sleep(0.1)
