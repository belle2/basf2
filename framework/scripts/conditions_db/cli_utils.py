#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Utility functions for the ConditionsDB command line interface
"""

import re
from basf2 import B2ERROR


class ItemFilter:
    """
    Class to filter a list of tags/payloads/iovs with a common interface, that
    is same set of parameters and easy handling of checking.

    This class defines -f,--filter/-e,--exclude/-r/--regex parameters which will
    be added to a :class:`argparse.ArgumentParser` when calling `add_arguments`.
    """
    def __init__(self, args):
        """initilization, just remember the arguments or parser"""
        #: arguments, either :class:`argparse.ArgumentParser` on initialization
        # or :class:`argparse.Namespace` after argument parsing
        self._args = args
        #: regular expression for filtering items which don't match
        self._filter = None
        #: regular expression for excluding items which match
        self._exclude = None

    def add_arguments(self, name):
        """
        Add arguments to the parser

        Parameters:
          name: Name of the objects to be filtered in the help text
        """
        self._args.add_argument("-f", "--filter", metavar="SEARCHTERM",
                                help="only {} matching this pattern will be "
                                "included. Pattern is case insensitive".format(name))
        self._args.add_argument("-e", "--exclude", default=None, type=str,
                                help="{} matching this pattern will be excluded. "
                                "Pattern is case insensitive".format(name))
        self._args.add_argument("-r", "--regex", action="store_true", default=False,
                                help="if given, --filter or --exclude options will be "
                                "interpreted as a python regular expression "
                                "(see https://docs.python.org/3/library/re.html)")

    def __str__(self):
        """
        Convert to a text representation of the form ' [regex filter=TERM exclude=TERM]'.

        The space in the beginnin is intentional to allow easy adding to a
        description without extra spaces

        If no filtering is performed an empty string is returned. If the
        patterns are non-regex (without --regex), the regex will be missing in
        the string.  If one of the --filter/--exclude option was not given, that
        part is removed from the string
        """
        text = []
        for attribute in ["filter", "exclude"]:
            term = getattr(self._args, attribute, None)
            if term is not None:
                text.append("{}={}".format(attribute, term))
        if not text:
            return ""
        if getattr(self._args, "regex", False):
            text.insert(0, "regex")

        return " [{}]".format(" ".join(text))

    def check_arguments(self):
        """
        Check if the arguments are valid (only if --regex is given) and compile into
        regular expressions. If False is returned there was an error with the
        regular expressions.
        """
        for attribute in ["filter", "exclude"]:
            term = getattr(self._args, attribute, None)
            if term is not None:
                try:
                    if not getattr(self._args, "regex", False):
                        term = re.escape(term)
                    setattr(self, "_"+attribute, re.compile(term, re.IGNORECASE))
                except Exception as e:
                    B2ERROR("--{}: '{}' is not a valid regular expression: {}'".format(attribute, term, e))
                    return False
        return True

    def check(self, item):
        """
        Check an item. True is returned if it should be kept, False if it is
        filtered out or excluded.

        Parameters:
          item: item to be filtered
        """
        if self._filter is not None and not self._filter.search(item):
            return False
        if self._exclude is not None and self._exclude.search(item):
            return False
        return True


class PayloadInformation:
    """Small container class to help compare payload information for efficient
    comparison between global tags"""
    def __init__(self, payload, iov):
        """Set all internal members from the json information of the payload and the iov.

        Arguments:
            payload (dict): json information of the payload as returned by REST api
            iov (dict): json information of the iov as returned by REST api
        """
        #: name of the payload
        self.name = payload['basf2Module']['name']
        #: checksum of the payload
        self.checksum = payload['checksum']
        #: interval of validity
        self.iov = iov["expStart"], iov["runStart"], iov["expEnd"], iov["runEnd"]
        #: revision, not used for comparisons
        self.rev = payload["revision"]
        #: payload id in CDB, not used for comparisons
        self.payload_id = payload["payloadId"]
        #: iov id in CDB, not used for comparisons
        self.iov_id = iov["payloadIovId"]

    def __hash__(self):
        """Make object hashable"""
        return hash((self.name, self.checksum, self.iov))

    def __eq__(self, other):
        """Check if two payloads are equal"""
        return (self.name, self.checksum, self.iov) == (other.name, other.checksum, other.iov)

    def __lt__(self, other):
        """Sort payloads by name, iov, revision"""
        return (self.name.lower(), self.iov, self.rev) < (other.name.lower(), other.iov, other.rev)

    def readable_iov(self):
        """return a human readable name for the IoV"""
        if self.iov == (0, 0, -1, -1):
            return "always"

        e1, r1, e2, r2 = self.iov
        if e1 == e2:
            if r1 == 0 and r2 == -1:
                return f"exp {e1}"
            elif r2 == -1:
                return f"exp {e1}, runs {r1}+"
            elif r1 == r2:
                return f"exp {e1}, run {r1}"
            else:
                return f"exp {e1}, runs {r1} - {r2}"
        else:
            if e2 == -1 and r1 == 0:
                return f"exp {e1} - forever"
            elif e2 == -1:
                return f"exp {e1}, run {r1} - forever"
            if r1 == 0 and r2 == -1:
                return f"exp {e1}-{e2}, all runs"
            elif r2 == -1:
                return f"exp {e1}, run {r1} - exp {e2}, all runs"
            else:
                return f"exp {e1}, run {r1} - exp {e2}, run {r2}"
