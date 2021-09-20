#!/usr/bin/env python3

"""This script calculates some numbers about the online book, e.g. number of
exercises etc.
"""

from pathlib import Path
from typing import NamedTuple
import os
import pprint
import re


class Statistics:
    def __init__(self):
        self.code_inclusions: int = 0
        self.hints: int = 0
        self.solutions: int = 0
        self.exercises: int = 0
        self.overview_boxes = 0
        self.key_points = 0

    def print_summary(self):
        pprint.pprint(self.__dict__)


class StatisticsVisitor:
    regexes = dict(
        code_inclusions=re.compile("(code-block\\s*::)|(literalinclude\\s*::)"),
        hints=re.compile(":class:.*hint"),
        solutions=re.compile(":class:.*solution"),
        exercises=re.compile(":class:.*exercise"),
        overview_boxes=re.compile(":class:.*overview"),
        key_points=re.compile(":class:.*key-points"),
    )

    def __init__(self):
        self.statistics = Statistics()

    def read_rst_file(self, path: Path):
        text = path.read_text()
        for line in text.split("\n"):
            for key, regex in self.regexes.items():
                if regex.findall(line):
                    setattr(
                        self.statistics, key, getattr(self.statistics, key) + 1
                    )

    def walk_directory(self, path: Path):
        for root, _, files in os.walk(Path()):
            for file in files:
                path = Path(root) / file
                if path.suffix == ".rst":
                    self.read_rst_file(path)


if __name__ == "__main__":
    this_dir = Path(__file__).resolve().parent
    sv = StatisticsVisitor()
    sv.walk_directory(this_dir)
    sv.statistics.print_summary()
