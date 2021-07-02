#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
interactive
-----------

Defines a function embed() that can be used to drop into an interactive python
shell from within a steering file or python module. If available, IPython will
be used. Use Ctrl+D to exit the shell.

    >>> import interactive
    >>> interactive.embed()
    In [1]:

See framework/examples/interactive_python.py for an example.
"""

from traitlets.config.loader import Config
from IPython.terminal.prompts import Prompts, Token
from IPython import embed


class Basf2IPythonPrompt(Prompts):
    """Provide slightly customized prompts when running basf2 interactively"""

    def in_prompt_tokens(self, cli=None):
        """Input prompt"""
        return [(Token.Prompt, "basf2 in ["),
                (Token.PromptNum, str(self.shell.execution_count)),
                (Token.Prompt, ']: ')]

    def out_prompt_tokens(self):
        """Output prompt"""
        return [(Token.OutPrompt, "basf2 out["),
                (Token.OutPromptNum, str(self.shell.execution_count)),
                (Token.OutPrompt, ']: ')]


def basf2_shell_config():
    """Return a config object customizing the shell prompt for basf2"""
    c = Config()
    c.TerminalInteractiveShell.prompts_class = Basf2IPythonPrompt
    return c
