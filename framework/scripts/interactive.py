#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
interactive
-----------

Defines a function embed() that can be used to drop into an interactive python
shell from within a steering file or python module. If available, IPython will
be used. Use Ctrl+D to exit the shell.

    >>> import interactive
    >>> interactive.embed()
    In [1]:

See framework/examples/interactive.py for an example.
"""

from IPython import embed
from traitlets.config.loader import Config
from IPython.terminal.prompts import Prompts, Token


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
