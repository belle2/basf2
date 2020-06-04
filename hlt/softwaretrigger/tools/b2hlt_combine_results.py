#!/usr/bin/env python3

from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True
PyConfig.StartGuiThread = False

from argparse import ArgumentParser
import os
import uproot
import root_pandas
import pandas as pd

__author__ = "Sam Cunliffe"
__email__ = "sam.cunliffe@desy.de"


def get_parser():
    """Get the command line options

    Returns:
        argparse.ArgumentParser for this tool
    """
    parser = ArgumentParser(
        description="Combines several ``software_trigger_result`` files.")
    parser.add_argument("input", nargs='*',
                        help="Wildcard to select ``software_trigger_results`` files.")
    parser.add_argument("--output",
                        help="The combined output ``software_trigger_result`` file name.",
                        default="software_trigger_results_combined.root")
    return parser


if __name__ == "__main__":

    args = get_parser().parse_args()

    # get input file list
    if not all([os.path.exists(f) for f in args.input]):
        raise FileNotFoundError("Could not find input files: %s" % args.input)

    # loop over SWTRs
    sum_out = pd.DataFrame()
    for fi in args.input:

        # might have swtr files with no events selected: skip these
        swtr = uproot.open(fi)["software_trigger_results"].pandas.df()
        if not swtr['total_events'][0].any():
            continue

        # add up all non-zero dataframes
        if sum_out.empty:
            sum_out = swtr
        else:
            sum_out = sum_out.add(swtr)

    root_pandas.to_root(sum_out, key='software_trigger_results', path=args.output)
    # uproot.newtree doesn't work in the current externals version but when it does this can be root free
    print("Created file %s" % args.output)
