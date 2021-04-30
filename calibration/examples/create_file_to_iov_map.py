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

import fnmatch
from pprint import PrettyPrinter
pp = PrettyPrinter(indent=2)

# Lets take some file patterns. We could have put wildcards in more places but this is enough for testing
file_path_patterns = ["/hsm/belle2/bdata/Data/Raw/e0003/r0495[5,6]*/**/*.root"]


def bad_run_finder(filepath):
    """
    Returns True if a file path contains '.bad' after the run number
    """
    return fnmatch.fnmatch(filepath, "*r?????.bad/*")

# Here's how to do 2.


def from_raw_data_file_paths(file_path_patterns):
    from caf.utils import find_absolute_file_paths, parse_raw_data_iov
    # First get the absolute file paths from the patterns
    file_paths = find_absolute_file_paths(file_path_patterns)
    # Now remove bad runs using our filter function
    import itertools
    file_paths = list(itertools.filterfalse(bad_run_finder, file_paths))

    file_to_iov = {}
    # Now loop over them and parse the IoVs out by looking at their directory/filenames
    for file_path in file_paths:
        file_to_iov[file_path] = parse_raw_data_iov(file_path)
    return file_to_iov


# Here's how to do 3.

def from_metadata_of_files(file_path_patterns):

    from caf.utils import make_file_to_iov_dictionary

    def run_in_one_process():
        """
        Creates the file_to_iov dictionary but only one file at a time.
        Uses bad_run_finder to filter out runs marked as bad from our glob pattern.
        """
        return make_file_to_iov_dictionary(file_path_patterns, filterfalse=bad_run_finder)

    def run_with_multiprocessing(max_processes):
        """
        Creates the file_to_iov dictionary but using a Pool object to control the number of subprocesses.
        Note that even though we're using a ThreadPool, we aren't bound by the GIL because we are subprocessing to
        run b2file-metadata-show in each Thread.

        Uses bad_run_finder to filter out runs marked as bad from our glob pattern.
        """
        from multiprocessing.pool import ThreadPool
        tp = ThreadPool(processes=max_processes)
        mapping = make_file_to_iov_dictionary(file_path_patterns, polling_time=5, pool=tp, filterfalse=bad_run_finder)
        tp.close()
        tp.join()
        return mapping

    return run_in_one_process()
#    return run_with_multiprocessing(max_processes=4)


# Define a map of our possible argparse choices to functions that run them
function_map = {"filepath": from_raw_data_file_paths,
                "metadata": from_metadata_of_files}


def get_argparser():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("method", help="The method by which you want to create the mapping of file paths -> IoVs.",
                        choices=function_map.keys())
    return parser


def main():
    # Do argument parsing
    parser = get_argparser()
    args = parser.parse_args()

    # Run the method we asked for from command line
    file_to_iov = function_map[args.method](file_path_patterns)

    print("Created the file to IoV map:")
    pp.pprint(file_to_iov)

    # Save for later use
    import pickle
    filename = "file_iov_map.pkl"
    with open(filename, 'bw') as iov_map_file:
        pickle.dump(file_to_iov, iov_map_file)
        print("Saved dictionary to the file '{}' for later use.".format(filename))

    # To read it in, usually in a separate process/steering file
    # fils_to_iov = pickle.load(open("file_iov_map.pkl", 'rb'))


if __name__ == "__main__":
    import sys
    sys.exit(main())
