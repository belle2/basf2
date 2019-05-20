#!/bin/env python3
# -*- coding: utf-8 -*-

"""
Script to get the integrated luminosity of a set of runs

Based on a bash script by Mikhail Remnev mikhail.remnev@desy.de
https://confluence.desy.de/display/BI/ECL+LM+Integrated+luminosity+in+Phase+3

Usage:
    integrated_luminosity.py --exp <E> --runs <Range> [--verbose]

<E> experiment number (mandatory)
<Range> run range (mandatory)
    syntax is: xx-yy,zz
    all runs between xx and yy (included) and run zz. Can have multiple ranges xx-yy,zz-ww

"""

__author__ = "Stefano Lacaprara"
__email__ = "stefano.lacaprara@pd.infn.it"


import argparse
import re
from argparse import ArgumentParser, ArgumentTypeError
import sqlite3
import time

reco = "lum_det_shift"
DB = "/gpfs/group/belle2/group/detector/ECL/ecl_lom.db"


def parseNumRange(string):
    m = re.match(r'(\d+)(?:-(\d+))?$', string)
    if not m:
        raise ArgumentTypeError("'" + string + "' is not a range of number. Expected forms like '0-5' or '2'.")
    start = m.group(1)
    end = m.group(2) or start
    return list(range(int(start, 10), int(end, 10) + 1))


def parseNumList(string):
    result = []
    for rr in [x.strip() for x in string.split(',')]:
        result = result + parseNumRange(rr)
    return result


def argparser():
    """
    Parse options as command-line arguments.
    """

    description = "Script to get the integrated luminosity for a range or runs"

    # Re-use the base argument parser, and add options on top.
    parser = argparse.ArgumentParser(description=description, usage=__doc__, add_help=True)

    parser.add_argument("--exp",
                        dest="exp",
                        action="store",
                        type=int,
                        required=True,
                        help="Experiment number")

    parser.add_argument("--runs",
                        dest="runs",
                        action="store",
                        type=parseNumList,
                        required=True,
                        default=None,
                        help="Process only the selected run range. Syntax is xx-yy,zz")

    parser.add_argument("--verbose",
                        dest="verb",
                        action="store_true",
                        default=False,
                        help="Print each run luminosity")

    return parser


if __name__ == '__main__':

    args = argparser().parse_args()

    L = 0
    conn = sqlite3.connect(DB)
    cursor = conn.cursor()
    for run in args.runs:
        cmd = f"SELECT SUM({reco}),start_time,end_time FROM prev_bhacnt_det_run WHERE exp = {args.exp} AND run = {run}"
        cursor.execute(cmd)
        test = cursor.fetchone()
        # print(test,test[0])
        if (test[0]):
            if(args.verb):
                runDate = time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.localtime(test[1]))
                print(f"Run {run}: L={test[0]:8.2f} /nb {runDate}")
            L += float(test[0])

    conn.close()
    print("-----------------------------------------")

    print(f"TOTAL    : L={L:5.2f} /nb = {L/1E3:5.2f} /pb = {L/1E6:5.3f} /fb = {L/1E9:5.4f} /ab")
