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


class CreateData(basf2.Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('EMD')
        self.array = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.array.registerInDataStore('Parts')
        self.arrayRel = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRel.registerInDataStore('MCParts')

        self.array.registerRelationTo(self.arrayRel)

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


class CreateData2(basf2.Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('EMD2')
        self.array = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.array.registerInDataStore('Parts2')
        self.arrayRel = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRel.registerInDataStore('MCParts2')

        self.array.registerRelationTo(self.arrayRel)

    def event(self):
        """reimplementation"""

        self.obj.create()
        self.obj.obj().setExperiment(1000)

        for i in range(10):
            newptr = self.array.appendNew()
            newptr.setPValue(i)

        for i in range(10):
            newptr = self.arrayRel.appendNew()
            newptr.setPDG(i)

        for i in range(10):
            self.array[i].addRelationTo(self.arrayRel[i])


class CreateData22(basf2.Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('EMD2')
        self.array = Belle2.PyStoreArray(Belle2.Particle.Class())
        self.array.registerInDataStore('Parts2')
        self.arrayRel = Belle2.PyStoreArray(Belle2.MCParticle.Class())
        self.arrayRel.registerInDataStore('MCParts2')

        self.array.registerRelationTo(self.arrayRel)

    def event(self):
        """reimplementation"""

        self.obj.create()
        self.obj.obj().setExperiment(1000)

        for i in range(11):
            newptr = self.array.appendNew()
            newptr.setPValue(i + 100)

        for i in range(11):
            newptr = self.arrayRel.appendNew()
            newptr.setPDG(i + 100)

        for i in range(11):
            self.array[i].addRelationTo(self.arrayRel[i])


class CheckData(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('EMD')
        self.array = Belle2.PyStoreArray('Parts')
        self.arrayRel = Belle2.PyStoreArray('MCParts')

    def event(self):
        """reimplementation"""

        print(self.name())

        assert self.obj.obj().getExperiment() == 500
        assert self.array.getEntries() == 5

        for arr in self.array:
            print(arr.getPValue())

        for arr in self.arrayRel:
            print(arr.getPDG())

        for arr in self.array:
            print(arr.getPValue(), '->', arr.getRelated('MCParts').getPDG())


class CheckData2(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('EMD2')
        self.array = Belle2.PyStoreArray('Parts2')
        self.arrayRel = Belle2.PyStoreArray('MCParts2')

        self.array.requireRelationTo(self.arrayRel)

    def event(self):
        """reimplementation"""

        print(self.name())

        assert self.obj.obj().getExperiment() == 1000
        assert self.array.getEntries() == 10
        assert self.arrayRel.getEntries() == 10

        for arr in self.array:
            print(arr.getPValue(), '->', arr.getRelated('MCParts2').getPDG())


class CheckData22(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('EMD2')
        self.array = Belle2.PyStoreArray('Parts2')
        self.arrayRel = Belle2.PyStoreArray('MCParts2')

    def event(self):
        """reimplementation"""

        print(self.name())

        assert self.obj.obj().getExperiment() == 1000

        assert self.array.getEntries() == 11
        assert self.arrayRel.getEntries() == 11

        for arr in self.array:
            print(arr.getPValue(), '->', arr.getRelated('MCParts2').getPDG())


class CheckData222(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('EMD2')
        self.array = Belle2.PyStoreArray('Parts2')
        self.arrayRel = Belle2.PyStoreArray('MCParts2')

    def event(self):
        """reimplementation"""

        print(self.name())

        # TODO: check what happens if obj with different values is merged!
        assert self.obj.obj().getExperiment() == 1000

        assert self.array.getEntries() == 21
        assert self.arrayRel.getEntries() == 21

        for arr in self.array:
            print(arr.getPValue())

        for arr in self.arrayRel:
            print(arr.getPDG())

        for arr in self.array:
            print(arr.getPValue(), '->', arr.getRelated('MCParts2').getPDG())


class CheckAbsence(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('EMD').isOptional()
        assert not Belle2.PyStoreArray('Parts').isOptional()

    def event(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('EMD').obj()


class CheckAbsence2(basf2.Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('EMD2').isOptional()
        assert not Belle2.PyStoreArray('Parts2').isOptional()

    def event(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('EMD2').obj()


main = basf2.Path()
main.add_module('EventInfoSetter')
# main.add_module(CheckAbsence()).set_name("checkabsence 1")

main.add_module(CreateData2())
main.add_module(CheckData2()).set_name("checkdata2 1")

indep = basf2.Path()
# indep.add_module(CheckAbsence()).set_name("checkabsence 3")
indep.add_module(CreateData())
indep.add_module(CheckData()).set_name("checkdata 2")
# indep.add_module(CheckAbsence2()).set_name("checkdata2 3")
indep.add_module(CreateData22())
indep.add_module(CheckData22()).set_name("checkdata22 1")
main.add_independent_merge_path(
    indep,
    merge_back_event=[
        'EMD',
        'Parts',
        'MCParts',
        'EMD2',
        'Parts2',
        'MCParts2',
        'PartsToMCParts',
        'Parts2ToMCParts2'])
main.add_module(CheckData()).set_name("checkdata 3")
main.add_module(CheckData222()).set_name("checkdata222 1")

basf2.print_path(main)
basf2.process(main)

print(basf2.statistics)

# @endcond
