#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Compare length of decay description measured in number of lines
# (including comments).

import re
import basf2
from ROOT.Belle2 import EvtGenDatabasePDG

database = EvtGenDatabasePDG.Instance()

f = open(basf2.find_file('decfiles/dec/DECAY_BELLE2.DEC'))
decfile_lines = f.readlines()
f.close()

re_decay = re.compile('Decay *([^ ]+).*\n')
re_enddecay = re.compile('Enddecay *')

in_decay = False
decays = []
for i in range(len(decfile_lines)):
    if in_decay:
        match = re_enddecay.match(decfile_lines[i])
        if match is not None:
            decays.append([particle, i - decay_line])
            in_decay = False
    else:
        match = re_decay.match(decfile_lines[i])
        if match is not None:
            particle = match.group(1)
            decay_line = i
            in_decay = True


def get_decay_length(particle_name):
    for decay in decays:
        if (decay[0] == particle_name):
            return decay[1]
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
