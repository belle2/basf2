#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Compare the length of decay description measured in number of lines
# (excluding comments and empty lines) as well as the sum of the decay
# mode branching fractions.
# Authors: Kirill Chilikin, Stephan Duell

import re

from ROOT import Belle2
from ROOT.Belle2 import EvtGenDatabasePDG

database = EvtGenDatabasePDG.Instance()

f = open(Belle2.FileSystem.findFile('decfiles/dec/DECAY_BELLE2.DEC'))
decfile_lines = f.readlines()
f.close()

re_decay = re.compile('Decay *([^ ]+).*\n')
re_enddecay = re.compile('Enddecay *')

in_decay = False
decays = {}  # save the decay mode lines belonging to a certain particle as dictionary to do some checks on these.
for i in range(len(decfile_lines)):
    if in_decay:
        match = re_enddecay.match(decfile_lines[i])
        if match is not None:
            in_decay = False
        else:
            if(decfile_lines[i] != '\n'):
                if(decfile_lines[i].lstrip()[0] != '#'):
                    decays[particle].append(decfile_lines[i])
    else:
        match = re_decay.match(decfile_lines[i])
        if match is not None:
            particle = match.group(1)
            decays[particle] = []
            in_decay = True

# determine the number of defined decay modes for each defined particle and compare them to the corresponding anti-particle


def get_decay_length(particle_name):
    if particle_name in decays:
        return len(decays[particle_name])
    return -1

for particle in database.ParticleList():
    code = particle.PdgCode()
    name = particle.GetName()
    if (code > 0):
        antiparticle = database.GetParticle(-code)
        if antiparticle:
            antiname = antiparticle.GetName()
            length = get_decay_length(name)
            antilength = get_decay_length(antiname)
            if (length > 0 and antilength > 0 and length != antilength):
                print(f'Inconsistent length of decay description '
                      f'for {name} ({length}) and {antiname} ({antilength}).')
                exit(1)


# Now get the sum of branching fractions for each particle
def get_branching_fraction(particle_name):
    if particle_name in decays:
        bfsum = 0.
        for decmode in decays[particle_name]:
            bfsum += float(decmode.split()[0])
        return bfsum
    return -1

for particle in database.ParticleList():
    code = particle.PdgCode()
    name = particle.GetName()
    if (code > 0):
        antiparticle = database.GetParticle(-code)
        if antiparticle:
            antiname = antiparticle.GetName()
            bfsum = get_branching_fraction(name)
            antibfsum = get_branching_fraction(antiname)
            if (bfsum > 0 and antibfsum > 0 and bfsum != antibfsum):
                print(f'Inconsistent sum of decay branching fractions '
                      f'for {name} ({bfsum}) and {antiname} ({antibfsum}).')
                exit(1)
            # This should be done for each particle, not only B mesons, but the other particle's decays have not yet been fixed
            if((abs(code) == 511 or abs(code) == 521) and bfsum > 0 and abs(1.0-bfsum) > 1e-7):
                print(f'Sum of decay mode branching fractions '
                      f'for {name} is not compatible with 1 ({bfsum}).')
                exit(1)
            # This should be done for each particle, not only B mesons, but the other particle's decays have not yet been fixed
            if((abs(code) == 511 or abs(code) == 521) and antibfsum > 0 and abs(1.0-antibfsum) > 1e-7):
                print(f'Sum of decay mode branching fractions '
                      f'for {antiname} is not compatible with 1 ({antibfsum}).')
                exit(1)