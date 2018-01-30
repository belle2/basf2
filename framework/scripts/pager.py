#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
pager
-----

Provides paginated output for Python code.
"""

import tempfile
import sys
import subprocess


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
        #: prompt string
        self.prompt = prompt
        #: flag indicating whether the pager should automatically exit if the
        # content fits on one screen
        self.quit_if_one_screen = quit_if_one_screen

    def __enter__(self):
        """ entering context """
        #: temporary file for capturing output
        self.tmp_file = tempfile.NamedTemporaryFile(mode='w')

        sys.stdout = self.tmp_file

    def __exit__(self, exc_type, exc_val, exc_tb):
        """ exiting context """
        sys.stdout.flush()
        if self.prompt is None:
            self.prompt = '%f'  # same as default prompt
        self.prompt += ' (press h for help or q to quit)'
        less = ['less', '-R', '-Ps' + self.prompt]
        if self.quit_if_one_screen:
            less += ['-F', '-X']
        p = subprocess.Popen(less + [self.tmp_file.name])
        p.communicate()
        self.tmp_file.close()
        sys.stdout = sys.__stdout__


if __name__ == '__main__':
    with Pager():
        for i in range(30):
            print("This is an example on how to use the pager.")
