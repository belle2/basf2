#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

# shift in x by 20 um
beam_misalignment = [0.002, 0, 0]

main = create_path()
main.add_module('EventInfoSetter')
main.add_module('BeamParameters', vertex=beam_misalignment, createPayload=True, payloadIov=[0, 0, -1, -1])
process(main)
