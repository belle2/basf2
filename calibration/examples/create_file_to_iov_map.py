#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#############################################################################
# You have several options about how to make a mapping of file paths -> IoV
#
# In order of most efficient -> least efficient
#
# 1. Get the mapping from some outside source of information like a
#    database/file catalog. Then turn it into a dictionary.
#    Right now you are unlikely to do this.
#
# 2. If you know the file name or the directory it is stored in contains the
#    IoV information you don't need to call b2file-metadata-show. You could just parse
#    the file path instead. This is much faster.
#
# 3. Call b2file-metadata-show on each file, but try to do it in a multiprocessed way.

import sys

if len(sys.argv) != 2:
    print("Usage: python3 create_file_to_iov_map.py <metadata|filepath>")
    sys.exit(1)
else:
    option = sys.argv[1]

# Lets take some file patterns. We could have put wildcards in more places but this is enough for testing
file_path_patterns = ["/hsm/belle2/bdata/Data/Raw/e0002/r00135/sub00/*.root",
                      "/hsm/belle2/bdata/Data/Raw/e0002/r00137/sub00/*.root"]

# Here's how to do 2.


def from_raw_data_file_paths(file_path_patterns):
    from caf.utils import find_absolute_file_paths, parse_raw_data_iov
    # First get the absolute file paths from the patterns
    file_paths = find_absolute_file_paths(file_path_patterns)
    files_to_iovs = {}
    # Now loop over them and parse the IoVs out by looking at their directory/filenames
    for file_path in file_paths:
        files_to_iovs[file_path] = parse_raw_data_iov(file_path)
    return files_to_iovs


# Here's how to do 3.

def from_metadata_of_files(file_path_patterns):

    from caf.utils import make_files_to_iovs_dictionary

    def run_in_one_process():
        """
        Creates the files_to_iovs dictionary but only one file at a time.
        """
        return make_files_to_iovs_dictionary(file_path_patterns)

    def run_with_multiprocessing(max_processes):
        """
        Creates the files_to_iovs dictionary but using a Pool object to control the number of subprocesses.
        Note that even though we're using a ThreadPool, we aren't bound by the GIL because we are subprocessing to
        run b2file-metadata-show in each Thread.
        """
        from multiprocessing.pool import ThreadPool
        tp = ThreadPool(processes=max_processes)
        mapping = make_files_to_iovs_dictionary(file_path_patterns, polling_time=5, pool=tp)
        tp.close()
        tp.join()
        return mapping

    # files_to_iovs = run_in_one_process()
    files_to_iovs = run_with_multiprocessing(max_processes=4)


if option == "metadata":
    files_to_iovs = from_metadata_of_files(file_path_patterns)
if option == "filepath":
    files_to_iovs = from_raw_data_file_paths(file_path_patterns)
else:
    print("That wasn't one of the available options for this script. Run it again with no arguments to see the options.")
    sys.exit(1)

from pprint import PrettyPrinter
pp = PrettyPrinter(indent=2)
pp.pprint(files_to_iovs)

import pickle
# Save for later use
with open("file_iov_map.pkl", 'bw') as iov_map_file:
    pickle.dump(files_to_iovs, iov_map_file)
    print("Saved dictionary to a file for later use.")

# To read it in, usually in a separate process/steering file
# fils_to_iov = pickle.load(open("file_iov_map.pkl", 'rb'))
