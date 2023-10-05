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
                        help='Identifier produced by basf2_mva_teacher')
    parser.add_argument('-did', '--db_identifiers', dest='db_identifiers', type=str, required=True, action='append', nargs='+',
                        help='New database identifier for the method')
    parser.add_argument('-be', '--begin_experiment', dest='begin_experiment', type=str, required=False, action='append', nargs='+',
                        help='First experiment for which the weightfile is valid')
    parser.add_argument('-ee', '--end_experiment', dest='end_experiment', type=str, required=False, action='append', nargs='+',
                        help='Last experiment for which the weightfile is valid')
    parser.add_argument('-br', '--begin_run', dest='begin_run', type=str, required=False, action='append', nargs='+',
                        help='First run for which the weightfile is valid')
    parser.add_argument('-er', '--end_run', dest='end_run', type=str, required=False, action='append', nargs='+',
                        help='Last run for which the weightfile is valid')
    return parser


if __name__ == '__main__':

    parser = get_argument_parser()
    args = parser.parse_args()
    basf2_mva_util.upload(args.identifier, args.db_identifier, args.begin_experiment, args.begin_run,
                          args.end_experiment, args.end_run)
