#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

##############################################################################
# Script to count how often an initial particle decays directly into another in the dec file.
# The dec file lines are written to an output file and the sum of the BFs is given.
# Dec file changes between different releases (set-up before) can be tracked..
# Initial state particles are passed using -is option. Currently 4 allowed.
# Default is [B+, B- ,B0, anti-B0]. Same possible for D, tau, pi, K, etc.
# Final state particles passed using -fs option. Also 4 allowed.
# Default is [e+, e-, mu+, mu-]. Can also look at KL, Ks, K+, K-, etc.
# The names of the particles have to agree with the exact spelling used in the dec file.
#############################################################################

import basf2
import argparse


def arg_parser():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-i', '--input',
                        default=basf2.find_file('decfiles/dec/DECAY_BELLE2.DEC'),
                        help='input file path',
                        metavar='FILE')
    parser.add_argument('-o', '--output',
                        default='BF.out',
                        help='output file path',
                        metavar='FILE')
    parser.add_argument('-fs', '--finalstate',
                        nargs='*',
                        default=['e+', 'e-', 'mu+', 'mu-'],
                        help='final state particles',
                        metavar='LIST')
    parser.add_argument('-is', '--initialstate',
                        nargs='*',
                        default=['B+', 'B-', 'B0', 'anti-B0'],
                        help='initital state particles',
                        metavar='LIST')
    return parser


if __name__ == '__main__':

    args = arg_parser().parse_args()
    filepath = args.input
    outpath = args.output
    istate = args.initialstate
    fstate = args.finalstate

    B1decs = {istate[0]: [], istate[1]: [], istate[2]: [], istate[3]: []}
    B2decs = {istate[0]: [], istate[1]: [], istate[2]: [], istate[3]: []}
    Bdecs = {istate[0]: [], istate[1]: [], istate[2]: [], istate[3]: []}

    B1BF = {istate[0]: 0, istate[1]: 0, istate[2]: 0, istate[3]: 0}
    B2BF = {istate[0]: 0, istate[1]: 0, istate[2]: 0, istate[3]: 0}
    BtotBF = {istate[0]: 0, istate[1]: 0, istate[2]: 0, istate[3]: 0}
    togglesave = ''

    with open(filepath) as f:
        line = f.readline()
        while line:
            if "Decay "+str(istate[0]) in line and line[0] != '#':
                togglesave = istate[0]
            if "Decay "+str(istate[1]) in line and line[0] != '#':
                togglesave = istate[1]
            if "Decay "+str(istate[2]) in line and line[0] != '#':
                togglesave = istate[2]
            if "Decay "+str(istate[3]) in line and line[0] != '#':
                togglesave = istate[3]

            if "Enddecay" in line:
                togglesave = ''

            if togglesave:
                if(line[0] != '#' and (' '+fstate[0]+' ' in line or ' '+fstate[1]+' ' in line
                                       or ' '+fstate[2]+' ' in line or ' '+fstate[3]+' ' in line)
                   and line[0:5] != 'Decay' and line != '\n'):
                    Bdecs[togglesave].append(line)
                if(line[0] != '#' and (' '+fstate[0]+' ' in line or ' '+fstate[1]+' ' in line)
                   and line[0:5] != 'Decay' and line != '\n'):
                    B1decs[togglesave].append(line)
                if(line[0] != '#' and (' '+fstate[2]+' ' in line or ' '+fstate[3]+' ' in line)
                   and line[0:5] != 'Decay' and line != '\n'):
                    B2decs[togglesave].append(line)
            line = f.readline()

    print("\n")
    basf2.B2INFO("Number of decays for: ")
    basf2.B2INFO(istate[0] + " " + str(len(Bdecs[istate[0]])))
    basf2.B2INFO(istate[1] + " " + str(len(Bdecs[istate[1]])))
    basf2.B2INFO(istate[2] + " " + str(len(Bdecs[istate[2]])))
    basf2.B2INFO(istate[3] + " " + str(len(Bdecs[istate[3]]))+"\n")

    for keys, vals in Bdecs.items():
        for string in vals:
            BtotBF[keys] += float(string.split()[0])
    for keys, vals in B1decs.items():
        for string in vals:
            B1BF[keys] += float(string.split()[0])
    for keys, vals in B2decs.items():
        for string in vals:
            B2BF[keys] += float(string.split()[0])

    basf2.B2INFO("The BFs for "+fstate[0] + " and " + fstate[1]+" are: ")
    basf2.B2INFO(B1BF)
    basf2.B2INFO("The BFs for "+fstate[2] + " and " + fstate[3]+" are: ")
    basf2.B2INFO(B2BF)
    basf2.B2INFO("The total BFs are: ")
    basf2.B2INFO(BtotBF)

    with open(filepath) as f:
        with open(outpath, "w+") as outf:
            for keys, vals in B1decs.items():
                outf.write(str(keys)+'\n')
                for val in vals:
                    outf.write(str(val))
            for keys, vals in B2decs.items():
                outf.write(str(keys)+'\n')
                for val in vals:
                    outf.write(str(val))
        print("\n")
        basf2.B2INFO('Sucessfully written decays to '+outpath)
        outf.close()
