#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

testObj = Belle2.PyStoreObj("TestObj")
persistentObj = Belle2.PyStoreObj("PersistentObj", 1)

objList = [testObj, persistentObj]
for obj in objList:
    assert not obj
    assert not obj.create()

    #class unknown
    assert not obj.registerInDataStore()
    B2INFO("Previous error is expected, please ignore.")

    assert obj.registerInDataStore("EventMetaData")

    assert not obj
    assert obj.create()
    assert obj
