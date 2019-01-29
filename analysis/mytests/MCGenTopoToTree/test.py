#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import basf2
from modularAnalysis import inputMdst
from modularAnalysis import fillParticleList
from modularAnalysis import MCGenTopoToTree

# create path
my_path = basf2.create_path()

# load input ROOT file
inputMdst('default', basf2.find_file('JPsi2ee_e2egamma.root', 'examples', False), path=my_path)

# create and fill ParticleList
fillParticleList('e+', 'nTracks>=2 and abs(d0)<2 and abs(z0)<4 and clusterE>2', path=my_path)

# output topology information of MC generated particles to tree

# 1) In the first instance, a paticle list is provided. In the second
# instance, no paticle list is provided. The two trees are different and
# ouputed to two files.
MCGenTopoToTree('e+', 'MCGenTopoE', 'testE.root', path=my_path)
MCGenTopoToTree('', 'MCGenTopoEvt', 'testEvt.root', path=my_path)

# 2) In the first instance, a paticle list is provided. In the second
# instance, no paticle list is provided. The two trees are different but
# ouputed to one file.
MCGenTopoToTree('e+', 'MCGenTopoE', 'test.root', path=my_path)
MCGenTopoToTree('', 'MCGenTopoEvt', 'test.root', path=my_path)

# Process the events
basf2.process(my_path)

# print out the summary
print(basf2.statistics)
