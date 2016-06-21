#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from basf2 import *
from ROOT import Belle2


class CreateData(Module):

    """create some data"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.obj.registerInDataStore('obj')
        self.array = Belle2.PyStoreArray(Belle2.EventMetaData.Class())
        self.array.registerInDataStore('array')

    def event(self):
        """reimplementation"""

        self.obj.create()
        self.obj.obj().setExperiment(19)

        for i in range(13):
            self.array.appendNew()


class CheckData(Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        self.obj = Belle2.PyStoreObj('obj')
        self.array = Belle2.PyStoreArray('array')

    def event(self):
        """reimplementation"""

        assert self.obj.obj().getExperiment() == 19
        assert self.array.getEntries() == 13


class CheckAbsence(Module):

    """check output of CreateData"""

    def initialize(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('obj').isOptional()
        assert not Belle2.PyStoreArray('array').isOptional()

    def event(self):
        """reimplementation"""

        assert not Belle2.PyStoreObj('obj').obj()


main = create_path()
main.add_module('EventInfoSetter')
main.add_module(CheckAbsence())

indep = create_path()
indep.add_module(CreateData())
indep.add_module(CheckData()).set_name("checkdata 1")
main.add_independent_path(indep)
main.add_module(CheckAbsence())

indep2 = create_path()
indep2.add_module(CheckAbsence())
indep2.add_module(CreateData())
indep2.add_module(CheckData()).set_name("checkdata 2")
main.add_independent_path(indep2, merge_back_event=['obj', 'array'])
main.add_module(CheckData()).set_name("checkdata 3")

process(main)

print(statistics)
