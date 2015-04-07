#!/usr/bin/env python
# -*- coding: utf-8 -*-

from basf2 import *
from ROOT import Belle2

testObj = Belle2.PyStoreObj("TestObj")
persistentObj = Belle2.PyStoreObj("PersistentObj", 1)
objList = (testObj, persistentObj)

assert len(Belle2.PyStoreObj.list()) == 0
assert len(Belle2.PyStoreObj.list(1)) == 0

for obj in objList:
    assert not obj
    assert not obj.create()

    # class unknown
    assert not obj.registerInDataStore()

    assert obj.registerInDataStore("EventMetaData")

    assert not obj
    assert obj.create()
    assert obj

assert len(Belle2.PyStoreObj.list()) == 1
assert len(Belle2.PyStoreObj.list(1)) == 1

# arrays
arrayList = (Belle2.PyStoreArray("TestArray"),
             Belle2.PyStoreArray("PersistentArray", 1))

assert len(Belle2.PyStoreArray.list()) == 0
assert len(Belle2.PyStoreArray.list(1)) == 0

for array in arrayList:
    assert not array
    assert array.getEntries() == 0
    assert len(array) == 0

    assert not array[1234]
    assert not array.appendNew()
    assert len(array) == 0

B2INFO("Previous errors are expected, please ignore.")
