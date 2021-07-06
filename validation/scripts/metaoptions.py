#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# std
from typing import Optional, Iterable, List


class MetaOptionParser:
    """
    Class to simplify the parsing of plot options
    supplied by the MetaOption named object attached
    to root plots.

    A typical meta options list might look like this:
    ["pvalue-warn=0.9", "pvalue-error=0.4"]
    """

    def __init__(self, meta_option_list: Optional[Iterable] = None):
        """
        Initialize MetaOptionParser
        @param meta_option_list: list of meta options read from ROOT object
            or None
        """
        if meta_option_list is None:
            meta_option_list = []
        meta_option_list = list(meta_option_list)
        #: store the meta option list for usage in the functions below
        self.mo: List[str] = meta_option_list

    def has_option(self, option_name: str) -> bool:
        """
        Checks whether an option is contained in the
        meta options
        @param option_name: name of the option to check for
        @return: True if the option is contained in the meta option list
                 False otherwise
        """
        return option_name in self.mo

    def pvalue_warn(self) -> float:
        """
        @return: The custom warning level for the pvalue setting of plot
                 comparison. None if no custom value was set for the plot.
        """
        return self.float_value("pvalue-warn")

    def pvalue_error(self) -> float:
        """
        @return: The custom error level for the pvalue setting of plot
                 comparison. None if no custom value was set for the plot.
        """
        return self.float_value("pvalue-error")

    def float_value(self, key, default: Optional[float] = None) -> float:
        """
        Extract the float value from a meta option list
        @param key: the key to identify the value from the list
        @param default: default value
        @return: The float value or the default value if this key did not exist
                 or the float value could not be parsed.
        """
        v = self.parse_key_value(key)
        if v is None:
            return default
        try:
            return float(v)
        except ValueError:
            return default

    def int_value(self, key, default: Optional[int] = None) -> int:
        """
        Extract the int value from a meta option list
        @param key: the key to identify the value from the list
        @param default: default value
        @return: The int value or None if this key did not exist
                 or the float value could not be parsed.
        """
        v = self.parse_key_value(key)
        if v is None:
            return default
        try:
            return int(v)
        except ValueError:
            return default

    def parse_key_value(self, key: str):
        """
        Searches the meta options list for a key value entry and parses it
        @param key: The key to look for
        @retun: The value which was associated to the key or None if the
                key was not found.
        """
        it = [s for s in self.mo if s.startswith(key + "=")]
        if len(it) == 0:
            return None

        key_value_pair = it[0].split("=")

        if len(key_value_pair) < 2:
            return None

        return key_value_pair[1]
