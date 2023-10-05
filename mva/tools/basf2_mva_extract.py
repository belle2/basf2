#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

import basf2_mva_util

import argparse


def get_argument_parser() -> argparse.ArgumentParser:
    """ Parses the command line options of the fei and returns the corresponding arguments. """
    parser = argparse.ArgumentParser()
    parser.add_argument('-id', '--identifiers', dest='identifiers', type=str, required=True, action='append', nargs='+',
                        help='Identifiers of the trained methods')
    parser.add_argument('-d', '--directory', dest='directory', type=str, required=True,
                        help='New database identifier for the method')
    return parser


if __name__ == '__main__':

    parser = get_argument_parser()
    args = parser.parse_args()
    for filename in args.identifiers:
        basf2_mva_util.extract(filename, args.directory)
