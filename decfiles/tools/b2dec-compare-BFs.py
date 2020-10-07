#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Helper script that reads the decay file and prints out any differences between charge conjugated B meson decays
S. Duell
"""

import os
import sys

if(len(sys.argv) < 2):
    filepath = '../../dec/DECAY_BELLE2.DEC'
else:
    filepath = sys.argv[1]

Balldecs = {'B+': [], 'B-': [], 'B0': [], 'anti-B0': []}
Bdecs = {'B+': [], 'B-': [], 'B0': [], 'anti-B0': []}
BallBF = {'B+': [], 'B-': [], 'B0': [], 'anti-B0': []}
BtotBF = {'B+': 0., 'B-': 0., 'B0': 0., 'anti-B0': 0.}
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

        # if "D+" in line:
        #    print("Line {}: {}".format(cnt, line.strip()))
        line = f.readline()
        cnt += 1

print("# of B+ decays: {}".format(len(Bdecs['B+'])))
print("# of B- decays: {}".format(len(Bdecs['B-'])))
print("# of B0 decays: {}".format(len(Bdecs['B0'])))
print("# of anti-B0 decays: {}".format(len(Bdecs['anti-B0'])))

for keys, vals in Bdecs.items():
    #    print('Processing decays of {}'.format(keys))
    for string in vals:
        BtotBF[keys] += float(string.split()[0])
        BallBF[keys].append(float(string.split()[0]))

print('\n\n')

print("B meson total BF")
print(BtotBF)

print('\n\n')


nomatch = [index for index, (e1, e2) in enumerate(zip(BallBF['B+'], BallBF['B-'])) if e1 != e2]
print('charged B not matching decay modes:')
for idx in nomatch:
    print('{} vs. {} at index {}'.format(Bdecs['B+'][idx], Bdecs['B-'][idx], idx))

print('\n\n')

nomatch2 = [index for index, (e3, e4) in enumerate(zip(BallBF['B0'], BallBF['anti-B0'])) if e3 != e4]
print('neutral B not matching decay modes:')
for idx in nomatch2:
    print('{} vs. {} at index {}'.format(Bdecs['B0'][idx], Bdecs['anti-B0'][idx], idx))
