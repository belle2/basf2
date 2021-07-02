#!/usr/bin/env python3
# -*- coding: utf-8 -*-

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

from ROOT import gSystem
from ROOT import Belle2
gSystem.Load('libanalysis_dataobjects')

bminus = Belle2.PyStoreObj(Belle2.ParticleList.Class(), "B-:test")
assert bminus.registerInDataStore()
assert bminus.create()
bminus.obj().initialize(-521, "B-:test")

bplus = Belle2.PyStoreObj(Belle2.ParticleList.Class(), "B+:test")
assert bplus.registerInDataStore()
assert bplus.create()
bplus.obj().initialize(521, "B+:test")
bplus.obj().bindAntiParticleList(bminus.obj())

assert 'Particles' == bminus.obj().getParticleCollectionName()
assert 'B-:test' == bminus.obj().getParticleListName()
assert 'B+:test' == bminus.obj().getAntiParticleListName()
assert 'B+:test' == bplus.obj().getParticleListName()
assert 'B-:test' == bplus.obj().getAntiParticleListName()
