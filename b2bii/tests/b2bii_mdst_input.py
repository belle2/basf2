#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

set_random_seed("something important")
# make sure FATAL messages don't have the function signature as this makes
# problems with clang printing namespaces differently
logging.set_info(LogLevel.FATAL, logging.get_info(LogLevel.ERROR))

main = create_path()
input = register_module('B2BIIMdstInput')
input.param('inputFileNames', [
                               Belle2.FileSystem.findFile('b2bii/tests/chaintest_1.mdst'),
                               Belle2.FileSystem.findFile('b2bii/tests/chaintest_2.mdst')
                              ])
main.add_module(input)
processed_event_numbers = []


class TestingModule(Module):
    def event(self):
        global processed_event_numbers
        emd = Belle2.PyStoreObj('EventMetaData')
        processed_event_numbers.append(emd.obj().getEvent())


main.add_module(TestingModule())

process(main)

expected_event_numbers = [3, 1, 2, 4, 6, 5, 8, 9, 3, 4, 2, 1, 7, 6]
assert expected_event_numbers == processed_event_numbers

# The first file contains the following event numbers (in this order)
# 3, 1, 2, 4, 6, 5, 8, 9
# The second file contains the following event numbers (in this order)
# 3, 4, 2, 1, 7, 6
# We select more event than the file contains, to check if it works anyway
main = create_path()
input = register_module('B2BIIMdstInput')
input.param('inputFileNames', [
                               Belle2.FileSystem.findFile('b2bii/tests/chaintest_1.mdst'),
                               Belle2.FileSystem.findFile('b2bii/tests/chaintest_2.mdst')
                              ])
input.param('entrySequences', ['1:2,4:6', '0,2:3,5:100'])
main.add_module(input)
main.add_module(TestingModule())

expected_event_numbers = [1, 2, 6, 5, 8, 3, 2, 1, 6]
processed_event_numbers = []
process(main)

assert expected_event_numbers == processed_event_numbers

# The first file contains the following event numbers (in this order)
# 3, 1, 2, 4, 6, 5, 8, 9
# The second file contains the following event numbers (in this order)
# 3, 4, 2, 1, 7, 6
# We select the complete first file and specific elements of the of the subsequent one.
main = create_path()
input = register_module('B2BIIMdstInput')
input.param('inputFileNames', [
    Belle2.FileSystem.findFile('b2bii/tests/chaintest_1.mdst'),
    Belle2.FileSystem.findFile('b2bii/tests/chaintest_2.mdst')
])
input.param('entrySequences', [':', '2:3,5:100'])
main.add_module(input)
main.add_module(TestingModule())

expected_event_numbers = [3, 1, 2, 4, 6, 5, 8, 9, 2, 1, 6]
processed_event_numbers = []
process(main)
assert expected_event_numbers == processed_event_numbers

# The first file contains the following event numbers (in this order)
# 3, 1, 2, 4, 6, 5, 8, 9
# The second file contains the following event numbers (in this order)
# 3, 4, 2, 1, 7, 6
# We do not select any element from the first file but specific elements of the subsequent one.
main = create_path()
input = register_module('B2BIIMdstInput')
input.param('inputFileNames', [
    Belle2.FileSystem.findFile('b2bii/tests/chaintest_1.mdst'),
    Belle2.FileSystem.findFile('b2bii/tests/chaintest_2.mdst')
])
input.param('entrySequences', ['', '2:3,5:100'])
main.add_module(input)
main.add_module(TestingModule())

expected_event_numbers = [2, 1, 6]
processed_event_numbers = []
process(main)
assert expected_event_numbers == processed_event_numbers
