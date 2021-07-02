#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

#############################################################################
# You have several options about how to make a mapping of file paths -> IoV
#
# In order of most efficient -> least efficient
#
# 1. If you know the file name or the directory it is stored in contains the
#    IoV information you don't need to call b2file-metadata-show. You could just parse
#    the file path instead. This is much faster.
#
# 2. Call b2file-metadata-show on each file, but try to do it in a multiprocessed way.
#
# Run this script by calling
#
# basf2 create_file_to_iov_map.py -- --option='metadata'
# --file_path_pattern='/hsm/belle2/bdata/Data/Raw/e0003/r*/**/*.root'


import pickle
from pprint import PrettyPrinter
import sys
import argparse
parser = argparse.ArgumentParser(
    description="Make a mapping file of file paths -> IoV")
parser.add_argument(
    '--file_path_patterns',
    default='/hsm/belle2/bdata/Data/Raw/e0003/r*/**/*.root',
    type=str,
    help='Lets take some file patterns. We could have put wildcards in more places but this is enough for testing.')
parser.add_argument('--output', default='file_iov_map.pkl', type=str,
                    help='Name of the output mapping file.')
parser.add_argument(
    '--option',
    default='filepath',
    type=str,
    help='Either take IoV from FileMetaData (option=metadata) or parse it from filepath (option=filepath).')
args = parser.parse_args()

file_path_patterns = [args.file_path_patterns, ]
print(file_path_patterns)

# Here's how to do 1.


def from_raw_data_file_paths(file_path_patterns):
    from caf.utils import find_absolute_file_paths, parse_raw_data_iov
    # First get the absolute file paths from the patterns
    file_paths = find_absolute_file_paths(file_path_patterns)
    file_to_iov = {}
    # Now loop over them and parse the IoVs out by looking at their
    # directory/filenames
    for file_path in file_paths:
        file_to_iov[file_path] = parse_raw_data_iov(file_path)
    return file_to_iov


# Here's how to do 2.

def from_metadata_of_files(file_path_patterns):

    from caf.utils import make_file_to_iov_dictionary

    def run_in_one_process():
        """
        Creates the file_to_iov dictionary but only one file at a time.
        """
        return make_file_to_iov_dictionary(file_path_patterns)

    def run_with_multiprocessing(max_processes):
        """
        Creates the file_to_iov dictionary but using a Pool object to control the number of subprocesses.
        Note that even though we're using a ThreadPool, we aren't bound by the GIL because we are subprocessing to
        run b2file-metadata-show in each Thread.
        """
        from multiprocessing.pool import ThreadPool
        tp = ThreadPool(processes=max_processes)
        mapping = make_file_to_iov_dictionary(
            file_path_patterns, polling_time=5, pool=tp)
        tp.close()
        tp.join()
        return mapping

    # return run_in_one_process()
    return run_with_multiprocessing(max_processes=6)


if args.option == "metadata":
    file_to_iov = from_metadata_of_files(file_path_patterns)
elif args.option == "filepath":
    file_to_iov = from_raw_data_file_paths(file_path_patterns)
else:
    print("That wasn't one of the available options for this script. Run it again with no arguments to see the options.")
    sys.exit(1)

pp = PrettyPrinter(indent=2)
pp.pprint(file_to_iov)

# Save for later use
with open(args.output, 'bw') as iov_map_file:
    pickle.dump(file_to_iov, iov_map_file)
    print("Saved dictionary to a file for later use.")

# To read it in, usually in a separate process/steering file
# fils_to_iov = pickle.load(open("file_iov_map.pkl", 'rb'))
