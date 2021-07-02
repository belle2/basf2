##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
"""
Log Filter class to replace string occurences in log messages to simplify testing
"""
import re


class LogReplacementFilter:
    """
    Simple class to intercept anything written to python stdout and replace
    a given set of strings with placehholders to improve reproducibility.
    """
    def __init__(self, out, replacements):
        """Setup the forwarding and replacements

        Parameters:
            out (file object): Where to forward the output too
            replacements (dict(str, str)): Dictionary of strings and their replacements
        """
        #: where to forward output
        self._out = out
        #: what to replace
        self._replacements = replacements
        #: build a regular expression from dictionary keys
        self._regex = re.compile("|".join(re.escape(e) for e in replacements))

    def write(self, data):
        """Check all messages for strings to replace"""
        replaced = self._regex.sub(lambda m: self._replacements[m[0]], data)
        self._out.write(replaced)

    def __getattr__(self, name):
        """Forward all other methods from the out stream"""
        return getattr(self._out, name)
