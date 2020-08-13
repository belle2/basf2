#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# @cond dont_want_no_doxygen_warnings_this_is_sphinxed

"""
terminal_utils - Helper functions for input from/output to a terminal
---------------------------------------------------------------------

This module contains modules useful to deal with output on the terminal:

* `Pager`, a class to provide paginated output with less similar to other
  popular tools like ``git diff``
* `InputEditor`, a class to open an editor window when requesting longer inputs
  from users, similar to ``git commit``
"""

import sys
import subprocess
import os
import io
import shutil
import tempfile
import shlex


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
        except AttributeError:
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
        sys.__stderr__ = io.TextIOWrapper(os.fdopen(self._original_stderr_fd, "wb"))

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
        # no pager, nothing to do
        if self._pager_process is None:
            return

        # otherwise let's try to flush whatever is left
        try:
            sys.stdout.flush()
        except BrokenPipeError:
            # apparently pager died before we could flush ... so let's move the
            # remaining output to /dev/null and flush whatever is left
            devnull = os.open(os.devnull, os.O_WRONLY)
            os.dup2(devnull, sys.stdout.fileno())
            sys.stdout.flush()

        # restore output
        os.dup2(self._original_stdout_fd, sys.stdout.fileno())
        os.dup2(self._original_stderr_fd, sys.stderr.fileno())
        # and the original __stdout__/__stderr__ object just in case. Will also
        # close the copied file descriptors
        sys.__stderr__ = self._original_stderr
        sys.__stdout__ = self._original_stdout

        # wait for pager
        self._pager_process.communicate()

        # and if we exited due to broken pipe ... then ignore it
        return exc_type == BrokenPipeError


class InputEditor():
    """
    Class to get user input via opening a temporary file in a text editor.

    It is an alternative to the python commands ``input()`` or ``sys.stdin.readlines`` and is
    similar to the behaviour of ``git commit`` for editing commit messages.  By using an editor
    instead of the command line, the user is motivated to give expressive multi-line input,
    leveraging the full text editing capabilities of his editor.  This function cannot be used for
    example in interactive terminal scripts, whenever detailed user input is required.

    Heavily inspired by the code in this blog post:
    https://chase-seibert.github.io/blog/2012/10/31/python-fork-exec-vim-raw-input.html

    Parameters:
        editor_command: Editor to open for user input.  If ``None``, get
            default editor from environment variables.  It should be the name
            of a shell executable and can contain command line arguments.
        initial_content: Initial string to insert into the temporary file that
            is opened for user input.  Can be used for default input or to
            insert comment lines with instructions.
        commentlines_start_with: Optionally define string with which comment
            lines start
    """

    def __init__(self,
                 editor_command: str = None,
                 initial_content: str = None,
                 commentlines_start_with: str = "#"):
        """Constructor"""
        # Use provided editor command or editor command from environment variables
        editor_command_string = editor_command or self._default_environment_editor()
        #: command line for the editor, split to seperate executable name command line arguments
        self.editor_command_list = shlex.split(editor_command_string, posix=True)
        # check if editor executable exists and if not, prompt for new editor command
        if shutil.which(self.editor_command_list[0]) is None:
            self._prompt_for_editor()

        #: initial content of the editor window
        self.initial_content = initial_content
        #: string which starts comments in the file
        self.comment_string = commentlines_start_with

    def input(self):
        """
        Get user input via editing a temporary file in an editor. If opening the editor fails, fall
        back to command line input
        """
        try:
            with tempfile.NamedTemporaryFile(mode='r+') as tmpfile:
                if self.initial_content:
                    tmpfile.write(self.initial_content)
                    tmpfile.flush()
                subprocess.check_call(self.editor_command_list + [tmpfile.name])
                tmpfile.seek(0)
                input_string = tmpfile.read().strip()
            input_string = self._remove_comment_lines(input_string)

        except (FileNotFoundError, subprocess.CalledProcessError):
            # If editor not found or other problem with subprocess call, fall back to terminal input
            print(f"Could not open {self.get_editor_command()}.")
            print("Try to set your $VISUAL or $EDITOR environment variables properly.\n")
            sys.exit(1)

        return input_string

    def get_editor_command(self):
        """Get editor shell command string used for user input."""
        # Construct string from list which holds the executable and args
        return " ".join(self.editor_command_list)

    def _remove_comment_lines(self, a_string):
        """
        Remove lines from string that start with a comment character and return modified version.
        """
        if self.comment_string is not None:
            a_string = "\n".join(
                [line for line in a_string.splitlines()
                 if not line.startswith(self.comment_string)]).strip()
        return a_string

    def _default_environment_editor(self):
        """
        Return editor from environment variables. If not existing, return vi(m) as default.
        """
        editor_command = (os.environ.get('VISUAL') or os.environ.get('EDITOR') or
                          'vim' or 'vi')
        return editor_command

    def _prompt_for_editor(self):
        """
        Ask user to provide editor command
        """
        # Prompt user for editor command until one is found which exists in PATH
        while True:
            new_editor_command_string = input("Use editor: ")
            new_editor_command_list = shlex.split(new_editor_command_string, posix=True)

            if shutil.which(new_editor_command_list[0]) is not None:
                self.editor_command_list = new_editor_command_list
                return self.editor_command_list

            else:
                print(f"Editor '{self.editor_command_list[0]}' not found in $PATH.")

# @endcond
