#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Helper script that reads the decay file and saves a new decay file where the automatically
created hadronic B meson decay modes (pythia/jetset) are rescaled such that the total B meson
BF is compatible with 1 (up to a certain precision, at the moment of this script creation O~1e-7)

S. Duell
"""


import os
import sys

if(len(sys.argv) < 3):
    if(len(sys.argv) < 2):
        filepath = '../../dec/DECAY_BELLE2.DEC'
    else:
        filepath = sys.argv[1]
    outfile = filepath+'_COR'
else:
    filepath = sys.argv[1]
    outfile = sys.argv[2]

Balldecs = {'B+': [], 'B-': [], 'B0': [], 'anti-B0': []}
Bdecs = {'B+': [], 'B-': [], 'B0': [], 'anti-B0': []}

BtotBF = {'B+': 0., 'B-': 0., 'B0': 0., 'anti-B0': 0.}
Btohadtotdecs = {'B+': 0., 'B-':  0., 'B0':  0., 'anti-B0': 0.}
jetsetscale = {'B+': 0., 'B-': 0., 'B0': 0., 'anti-B0': 0.}
togglesave = ''

with open(filepath) as f:
    line = f.readline()
    cnt = 1
    while line:
        if "Decay B+" in line:
            togglesave = 'B+'
        if "Decay B-" in line:
            togglesave = 'B-'
        if "Decay B0" in line:
            togglesave = 'B0'
        if "Decay anti-B0" in line:
            togglesave = 'anti-B0'

        if "Enddecay" in line:
            togglesave = ''

        if togglesave:
            if(line[0] != '#' and line[0:5] != 'Decay' and line != '\n'):
                Bdecs[togglesave].append(line)

        line = f.readline()
        cnt += 1

print(len(Bdecs['B+']))
print(len(Bdecs['B-']))
print(len(Bdecs['B0']))
print(len(Bdecs['anti-B0']))

for keys, vals in Bdecs.items():
    print('Processing decays of {}'.format(keys))
    for string in vals:
        BtotBF[keys] += float(string.split()[0])
        if('u' == string.split()[1] or 'anti-u' == string.split()[1]
                or 'd' == string.split()[1] or 'anti-d' == string.split()[1]
                or 's' == string.split()[1] or 'anti-s' == string.split()[1]
                or 'c' == string.split()[1] or 'anti-c' == string.split()[1]
                or 'cd' in string.split()[1] or 'anti-cd' in string.split()[1]
                or 'cs' in string.split()[1] or 'anti-cs' in string.split()[1]):
            Btohadtotdecs[keys] += float(string.split()[0])

for keys, vals in Btohadtotdecs.items():
    jetsetscale[keys] = (1-((BtotBF[keys]-1)/Btohadtotdecs[keys]))

print("the jetset scaling factors are: ")
print(jetsetscale)

with open(filepath) as f:
    with open(outfile, "w+") as outf:
        line = f.readline()
        cnt = 1
        while line:
            if "Decay B+" in line:
                togglesave = 'B+'
            if "Decay B-" in line:
                togglesave = 'B-'
            if "Decay B0" in line:
                togglesave = 'B0'
            if "Decay anti-B0" in line:
                togglesave = 'anti-B0'

            if "Enddecay" in line:
                togglesave = ''

            if togglesave:
                if(line[0] != '#' and line[0:5] != 'Decay' and line != '\n'):
                    if('u' == line.split()[1] or 'anti-u' == line.split()[1]
                            or 'd' == line.split()[1] or 'anti-d' == line.split()[1]
                            or 's' == line.split()[1] or 'anti-s' == line.split()[1]
                            or 'c' == line.split()[1] or 'anti-c' == line.split()[1]
                            or 'cd' in line.split()[1] or 'anti-cd' in line.split()[1]
                            or 'cs' in line.split()[1] or 'anti-cs' in line.split()[1]):
                        bfstr = '{:.8f}    '.format(float(line.split()[0])*jetsetscale[togglesave])
                        bfstr += (line.split(None, 1)[-1])
                        outf.write(bfstr)
                    else:
                        outf.write(line)
                else:
                    outf.write(line)
            else:
                outf.write(line)

            line = f.readline()
            cnt += 1
    print('sucessfully rescaled decay file to '+outfile)
    outf.close()
