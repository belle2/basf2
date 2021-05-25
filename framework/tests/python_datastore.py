#!/usr/bin/env python3

import basf2
from ROOT import Belle2
import unittest

# @cond internal_test


class PythonDataStoreTests(unittest.TestCase):

    def test_PyStoreObj(self):
        testObj = Belle2.PyStoreObj("TestObj")
        persistentObj = Belle2.PyStoreObj("PersistentObj", Belle2.DataStore.c_Persistent)
        objList = (testObj, persistentObj)

        nEventObj = len(Belle2.PyStoreObj.list())
        nPersistentObj = len(Belle2.PyStoreObj.list())

        for obj in objList:
            self.assertTrue(not obj)
            self.assertTrue(not obj.create())

            # class unknown
            self.assertTrue(not obj.registerInDataStore())

            # Object not yet created
            self.assertTrue(not obj)

        self.assertEqual(nEventObj, len(Belle2.PyStoreObj.list()))
        self.assertEqual(nPersistentObj, len(Belle2.PyStoreObj.list(Belle2.DataStore.c_Persistent)))

    # arrays with explicit class
    def test_PyStoreObj_with_known_class(self):
        nEventObj = len(Belle2.PyStoreObj.list())
        nPersistentObj = len(Belle2.PyStoreObj.list())

        eventMetaData = Belle2.PyStoreObj("EventMetaData")
        self.assertTrue(eventMetaData.registerInDataStore())

        self.assertFalse(eventMetaData.isValid())
        eventMetaData.create()
        self.assertTrue(eventMetaData.isValid())

        # Test direct access to contained object
        eventMetaData.setExperiment(17)

        # Retrieve from other module
        eventMetaData2 = Belle2.PyStoreObj(Belle2.EventMetaData.Class())
        self.assertTrue(eventMetaData2.isValid())
        self.assertEqual(17, eventMetaData.getExperiment())

        # One persistent object was created in the process
        self.assertEqual(nEventObj + 1, len(Belle2.PyStoreObj.list()))
        self.assertEqual(nPersistentObj, len(Belle2.PyStoreObj.list(Belle2.DataStore.c_Persistent)))

    def test_PyStoreArray(self):
        # arrays
        arrayList = (Belle2.PyStoreArray("TestArray"),
                     Belle2.PyStoreArray("PersistentArray", Belle2.DataStore.c_Persistent))

        self.assertEqual(0, len(Belle2.PyStoreArray.list()))
        self.assertEqual(0, len(Belle2.PyStoreArray.list(Belle2.DataStore.c_Persistent)))

        for array in arrayList:
            self.assertTrue(not array)
            self.assertEqual(0, array.getEntries())
            self.assertEqual(0, len(array))

            self.assertTrue(not array[1234])
            self.assertTrue(not array.appendNew())
            self.assertEqual(0, len(array))

    def test_EnumBackwardsCompatibility(self):
        self.assertEqual(0, Belle2.DataStore.c_Event)
        self.assertEqual(1, Belle2.DataStore.c_Persistent)

    # arrays with explicit class
    def test_PyStoreArray_with_known_class(self):
        # First module creates and registers two store arrays and a relation
        array = Belle2.PyStoreArray(Belle2.RelationsObject.Class())

        self.assertTrue(array.registerInDataStore("RelationsObjects"))
        self.assertEqual("RelationsObjects", array.getName())

        otherArray = Belle2.PyStoreArray(Belle2.RelationsObject.Class(), "OtherRelationsObjects")

        self.assertEqual("OtherRelationsObjects", otherArray.getName())
        self.assertTrue(otherArray.registerInDataStore())

        array.registerRelationTo(otherArray)

        self.assertTrue(array.hasRelationTo(otherArray))
        self.assertTrue(otherArray.hasRelationFrom(array))
        self.assertFalse(array.hasRelationFrom(otherArray))
        self.assertFalse(otherArray.hasRelationTo(array))

        obj = array.appendNew()
        self.assertTrue(obj)
        self.assertEqual(1, len(array))
        self.assertTrue(array[0])

        for obj in array:
            # Valid iterator
            break
        else:
            # Invalid iterator
            self.assertTrue(False)

        otherObject = otherArray.appendNew()
        obj.addRelationTo(otherObject)

        # Second module requires and reads the store arrays

        # Arrays can be retrieved like above or by their names only
        array2 = Belle2.PyStoreArray("RelationsObjects")
        array2.isRequired()
        otherArray2 = Belle2.PyStoreArray("OtherRelationsObjects")
        otherArray2.isRequired()

        self.assertEqual(1, len(array2))
        self.assertEqual(1, len(otherArray2))

        array2.requireRelationTo(otherArray2)
        obj2 = array2[0]
        self.assertEqual(obj, obj2)

        otherObject2 = obj2.getRelated("OtherRelationsObjects")
        self.assertEqual(otherObject, otherObject2)


if __name__ == "__main__":
    basf2.B2INFO("Following error messages are expected, please ignore.")
    unittest.main()
    basf2.B2INFO("Previous errors are expected, please ignore.")

# @endcond
