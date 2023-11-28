#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

# @cond

import basf2
from ROOT import Belle2


class CreateDataMain(basf2.Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('EventMetaData')
        self.array = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.array.registerInDataStore('Particles')
        self.arrayRel = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRel.registerInDataStore('MCParticles')

        self.array.registerRelationTo(self.arrayRel)

        self.objFirst = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.objFirst.registerInDataStore('EventMetaDataFirst')
        self.arrayFirst = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.arrayFirst.registerInDataStore('ParticlesFirst')
        self.arrayRelFirst = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRelFirst.registerInDataStore('MCParticlesFirst')

        self.arrayFirst.registerRelationTo(self.arrayRelFirst)

    def event(self):
        """reimplementation"""

        self.obj.create()
        self.obj.obj().setExperiment(500)

        for i in range(5):
            newptr = self.array.appendNew()
            newptr.setPValue(i)

        for i in range(5):
            newptr = self.arrayRel.appendNew()
            newptr.setPDG(i)

        for i in range(5):
            self.array[i].addRelationTo(self.arrayRel[i])

        self.objFirst.create()
        self.objFirst.obj().setExperiment(600)

        for i in range(6):
            newptr = self.arrayFirst.appendNew()
            newptr.setPValue(i)

        for i in range(6):
            newptr = self.arrayRelFirst.appendNew()
            newptr.setPDG(i)

        for i in range(6):
            self.arrayFirst[i].addRelationTo(self.arrayRelFirst[i])


class CreateDataIndep(basf2.Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('EventMetaData')
        self.array = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.array.registerInDataStore('Particles')
        self.arrayRel = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRel.registerInDataStore('MCParticles')

        self.array.registerRelationTo(self.arrayRel)

        self.objSecond = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.objSecond.registerInDataStore('EventMetaDataSecond')
        self.arraySecond = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.arraySecond.registerInDataStore('ParticlesSecond')
        self.arrayRelSecond = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRelSecond.registerInDataStore('MCParticlesSecond')

        self.arraySecond.registerRelationTo(self.arrayRelSecond)

    def event(self):
        """reimplementation"""

        self.obj.create()
        self.obj.obj().setExperiment(700)

        for i in range(7):
            newptr = self.array.appendNew()
            newptr.setPValue(i)

        for i in range(7):
            newptr = self.arrayRel.appendNew()
            newptr.setPDG(i)

        for i in range(7):
            self.array[i].addRelationTo(self.arrayRel[i])

        self.objSecond.create()
        self.objSecond.obj().setExperiment(800)

        for i in range(8):
            newptr = self.arraySecond.appendNew()
            newptr.setPValue(i)

        for i in range(8):
            newptr = self.arrayRelSecond.appendNew()
            newptr.setPDG(i)

        for i in range(8):
            self.arraySecond[i].addRelationTo(self.arrayRelSecond[i])


class CheckData(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('EventMetaData')
        self.obj_indepPath = Belle2.PyStoreObj('EventMetaData_indepPath')
        self.objFirst = Belle2.PyStoreObj('EventMetaDataFirst')
        self.objSecond = Belle2.PyStoreObj('EventMetaDataSecond')
        self.array = Belle2.PyStoreArray('Particles')
        self.arrayRel = Belle2.PyStoreArray('MCParticles')
        self.arrayFirst = Belle2.PyStoreArray('ParticlesFirst')
        self.arrayRelFirst = Belle2.PyStoreArray('MCParticlesFirst')
        self.arraySecond = Belle2.PyStoreArray('ParticlesSecond')
        self.arrayRelSecond = Belle2.PyStoreArray('MCParticlesSecond')

    def event(self):
        """reimplementation"""

        print(self.name())

        assert self.obj.obj().getExperiment() == 500
        assert self.obj_indepPath.obj().getExperiment() == 700
        assert self.objFirst.obj().getExperiment() == 600
        assert self.objSecond.obj().getExperiment() == 800
        assert self.array.getEntries() == 5 + 7
        assert self.arrayFirst.getEntries() == 6
        assert self.arraySecond.getEntries() == 8

        for arr in self.array:
            print(arr.getPValue())

        for arr in self.arrayRel:
            print(arr.getPDG())

        for arr in self.array:
            print(arr.getPValue(), '->', arr.getRelated('MCParticles').getPDG())


main = basf2.Path()
main.add_module('EventInfoSetter')

main.add_module(CreateDataMain())

indep = basf2.Path()
indep.add_module(CreateDataIndep())

main.add_independent_merge_path(
    indep,
    merge_back_event=['ALL'])

main.add_module(CheckData())

basf2.print_path(main)
basf2.process(main)

print(basf2.statistics)

# @endcond
