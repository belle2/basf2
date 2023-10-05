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
    parser.add_argument('-id', '--identifier', dest='identifier', type=str, required=True,
                        help='Identifier produced by basf2_mva_teacher')
    parser.add_argument('-did', '--db_identifier', dest='db_identifier', type=str, required=True,
                        help='New database identifier for the method')
    parser.add_argument('-be', '--begin_experiment', dest='begin_experiment', type=int, required=False,
                        help='First experiment for which the weightfile is valid', default=0)
    parser.add_argument('-ee', '--end_experiment', dest='end_experiment', type=int, required=False,
                        help='Last experiment for which the weightfile is valid', default=0)
    parser.add_argument('-br', '--begin_run', dest='begin_run', type=int, required=False,
                        help='First run for which the weightfile is valid', default=-1)
    parser.add_argument('-er', '--end_run', dest='end_run', type=int, required=False,
                        help='Last run for which the weightfile is valid', default=-1)
    return parser


if __name__ == '__main__':

    parser = get_argument_parser()
    args = parser.parse_args()
    basf2_mva_util.upload(args.identifier, args.db_identifier, args.begin_experiment, args.begin_run,
                          args.end_experiment, args.end_run)
