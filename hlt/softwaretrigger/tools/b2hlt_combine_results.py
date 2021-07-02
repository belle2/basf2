#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import PyConfig
PyConfig.IgnoreCommandLineOptions = True  # noqa
PyConfig.StartGuiThread = False  # noqa

from argparse import ArgumentParser
import basf2 as b2
import os
import uproot
import root_pandas
import pandas as pd
import numpy as np

__author__ = "Sam Cunliffe"
__email__ = "sam.cunliffe@desy.de"

PRESCALE_ROW = 4


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


def get_prescales(df):
    """Get prescale values from a data frame

    Returns:
        a list of the prescale values of each trigger line
    """
    prescales = []
    for col in df.columns:
        if col.find('software_trigger_cut_') >= 0 and df[col][PRESCALE_ROW] > 0:
            prescales.append(df[col][PRESCALE_ROW])
    return prescales


if __name__ == "__main__":

    args = get_parser().parse_args()

    # get input file list
    if not all([os.path.exists(f) for f in args.input]):
        raise FileNotFoundError("Could not find input files: %s" % args.input)

    # loop over SWTRs
    sum_out = pd.DataFrame()
    prescales = []  # prescale values of the trigger lines in each data frame
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
        prescales.append(get_prescales(swtr))

    prescales = np.array(prescales)  # we want the prescales as numpy array for slicing
    i = 0  # index the trigger lines

    # the prescale values were also added up, to get the correct prescale values back,
    # we take the first prescale value of each trigger line
    # if the prescale value of a trigger line is changing in the files set the prescale value
    # to nan for this trigger line and give a warning

    for col in sum_out.columns:
        # loop over all trigger lines
        if col.find('software_trigger_cut_') >= 0 and sum_out[col][PRESCALE_ROW] > 0:
            prescale_changed = False
            for j in range(prescales[:, i].size - 1):
                # check if prescales are changing in one of the files
                if not prescales[j, i] == prescales[j+1, i]:
                    prescale_changed = True
                    break
            if not prescale_changed:
                # use prescale of first file
                sum_out.at[PRESCALE_ROW, col] = prescales[0, i]
            else:
                b2.B2WARNING("{}: Different prescale values found for this trigger line! ".format(col) +
                             "Final prescale value is set to NaN.")
                sum_out.at[PRESCALE_ROW, col] = np.nan
            i += 1

    root_pandas.to_root(sum_out, key='software_trigger_results', path=args.output)
    # TODO: uproot.newtree works with the current externals version so this can be root free
    # BUT, unfortunately we are not sure how to write the pandas data frame to a root tree

    print("Created file %s" % args.output)
