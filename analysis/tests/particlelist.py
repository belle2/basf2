#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import gSystem
gSystem.Load('libanalysis_dataobjects')
from ROOT import Belle2

bminus = Belle2.PyStoreObj("B-:test")
assert bminus.registerInDataStore("ParticleList")
assert bminus.create()
bminus.obj().initialize(-521, "B-:test")

bplus = Belle2.PyStoreObj("B+:test")
assert bplus.registerInDataStore("ParticleList")
assert bplus.create()
bplus.obj().initialize(521, "B+:test")
bplus.obj().bindAntiParticleList(bminus.obj())

assert 'Particles' == bminus.obj().getParticleCollectionName()
assert 'B-:test' == bminus.obj().getParticleListName()
assert 'B+:test' == bminus.obj().getAntiParticleListName()
assert 'B+:test' == bplus.obj().getParticleListName()
assert 'B-:test' == bplus.obj().getAntiParticleListName()
