#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
This script calculates some numbers about the online book, e.g. number of
exercises etc.
"""

from pathlib import Path
import os
import pprint
import re


class Statistics:
    """
    Statistics base class
    """
    def __init__(self):
        """Initialize members"""
        self.code_inclusions: int = 0
        self.hints: int = 0
        self.solutions: int = 0
        self.exercises: int = 0
        self.overview_boxes: int = 0
        self.key_points: int = 0
        self.figures: int = 0
        self.characters: int = 0

    def print_summary(self):
        """Print the summary"""
        pprint.pprint(self.__dict__)


class StatisticsVisitor:
    """
    Statistics visitor class
    """
    #: Dictionary keys must match attributes of Statistics class
    regexes = dict(
        code_inclusions=re.compile("(code-block\\s*::)|(literalinclude\\s*::)"),
        hints=re.compile(":class:.*hint"),
        solutions=re.compile(":class:.*solution"),
        exercises=re.compile(":class:.*exercise"),
        overview_boxes=re.compile(":class:.*overview"),
        key_points=re.compile(":class:.*key-points"),
        figures=re.compile("figure\\s*::")
    )

    def __init__(self):
        """Initialize class"""
        #: statistics object
        self.statistics = Statistics()

    def read_rst_file(self, path: Path):
        """Read rst file"""
        text = path.read_text()
        self.statistics.characters += len(text)
        for line in text.split("\n"):
            for key, regex in self.regexes.items():
                if regex.findall(line):
                    setattr(
                        self.statistics, key, getattr(self.statistics, key) + 1
                    )

    def walk_directory(self, path: Path):
        """Loop over all directories in the path"""
        print(f"Walking {path}")
        for root, _, files in os.walk(path):
            for file in files:
                path = Path(root) / file
                if path.suffix == ".rst":
                    self.read_rst_file(path)


def main():
    this_dir = Path(__file__).resolve().parent
    sv = StatisticsVisitor()
    sv.walk_directory(this_dir)
    sv.statistics.print_summary()


if __name__ == "__main__":
    main()
