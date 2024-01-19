#!/usr/bin/env python3

"""
Test HepMC reader and writer by reading in a hepmc file and writing out one
again and checking if they match.

"""

import os
import sys
import math
import basf2 as b2
import modularAnalysis as ma
from tempfile import TemporaryDirectory

# analogous to lhereader.py
#: \cond Doxygen_suppress
input_filename = b2.find_file("generators/tests/event.hepmc")
#: \endcond
if len(input_filename) == 0:
    sys.stderr.write("TEST SKIPPED: input file " + input_filename + " not found.")
    sys.exit(-1)

with TemporaryDirectory() as tmp:

    #: \cond Doxygen_suppress
    output_filename = "event.hepmc.out"

    os.chdir(tmp)
    path = ma.create_path()
    path.add_module("HepMCInput", inputFileList=[input_filename], expNum=0, runNum=0)
    path.add_module("HepMCOutput", OutputFilename=output_filename)
    b2.process(path)
    #: \endcond

    def valid_line(li):
        """
        Check only lines that contain numbers

        HepMC lines look like this:
        Letter Number Number Number Number ...

        with letters:
        E: event, V: vertex, P: particle
        """
        return li.split()[0] in ["E", "V", "P"]

    #: \cond Doxygen_suppress
    def parse_line(li):
        tokens = li.split()
        start_index = 1
        if tokens[0] == "E":
            # For events, the first number is the event number
            # We don't check this here, since it will be incremented in basf2
            start_index = 2
        return [float(t) for t in tokens[start_index:]]

    def get_nonempty_lines(f):
        return [li for li in f if not li.isspace()]

    with open(input_filename) as inputfile, open(output_filename) as outputfile:
        lines_input = get_nonempty_lines(inputfile)
        lines_output = get_nonempty_lines(outputfile)
        for line_input, line_output in zip(lines_input, lines_output):
            if valid_line(line_input):
                assert valid_line(line_output)
                numbers_input = parse_line(line_input)
                numbers_output = parse_line(line_output)
                for number_input, number_output in zip(numbers_input, numbers_output):
                    assert math.isclose(number_input, number_output, rel_tol=1e-5)
    #: \endcond
