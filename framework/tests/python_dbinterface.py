##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################
from basf2 import _constwrapper
from b2test_utils import configure_logging_for_tests
import ROOT
from ROOT import Belle2
import multiprocessing
import unittest

# @cond internal_test


class DBInterface(unittest.TestCase):
    def assertDeath(self, function, *args, **kwargs):
        """Run function in child process and check if it died. Only way to check for B2FATAL"""
        ctx = multiprocessing.get_context("fork")
        p = ctx.Process(target=function, args=args, kwargs=kwargs)
        p.start()
        p.join()
        self.assertNotEqual(p.exitcode, 0)

    def setUp(self):
        configure_logging_for_tests()

    def tearDown(self):
        Belle2.DBStore.Instance().reset()

    def test_obj(self):
        """Test object interface"""
        bp = Belle2.PyDBObj("BeamParameters")
        # no such thing, should be invalid
        self.assertFalse(bp.isValid())

        # create a mock dbstore entry
        override_object = Belle2.BeamParameters()
        vertex = ROOT.TVector3(1, 2, 3)
        # sadly root does not handle the overloads correctly so we don't have an
        # overload without the covariance matrix
        override_object.setVertex(vertex, ROOT.std.vector("double")())
        Belle2.DBStore.Instance().addConstantOverride("BeamParameters", override_object, False)

        # must be valid now
        self.assertTrue(bp.isValid())

        # make sure type checking works: no access array<->obj or with wrong
        # type or both
        self.assertDeath(lambda: Belle2.PyDBObj("BeamParameters", Belle2.PXDSimHit.Class()))
        self.assertDeath(lambda: Belle2.PyDBArray("BeamParameters"))
        self.assertDeath(lambda: Belle2.PyDBArray("BeamParameters", Belle2.PXDSimHit.Class()))

        # check for correct vertex
        self.assertEqual(bp.obj().getVertex(), vertex)
        # and also "indirection" should work
        self.assertEqual(bp.getVertex(), vertex)
        # and it should be read only
        with self.assertRaises(AttributeError):
            # aka no setters
            bp.obj().setVertex()

        # but static members like Class should work
        bp.obj().Class()

        # make a copy
        copy = Belle2.BeamParameters(bp.obj())
        # and compare
        self.assertEqual(copy, bp.obj())

    def test_array(self):
        bplist = Belle2.PyDBArray("BeamParameterList")
        # no such thing, should be invalid
        self.assertFalse(bplist.isValid())
        # create a mock dbstore entry
        override_array = ROOT.TClonesArray("Belle2::BeamParameters")
        override_array.ExpandCreate(3)
        for i in range(override_array.GetEntriesFast()):
            override_array[i].setVertex(ROOT.TVector3(i, i, i), ROOT.std.vector("double")())

        # must be valid now
        Belle2.DBStore.Instance().addConstantOverride("BeamParameterList", override_array, False)

        self.assertTrue(bplist.isValid())
        # make sure type checking works: no access array<->obj or with wrong
        # type or both
        self.assertDeath(lambda: Belle2.PyDBArray("BeamParameterList", Belle2.PXDSimHit.Class()))
        self.assertDeath(lambda: Belle2.PyDBObj("BeamParameterList"))
        self.assertDeath(lambda: Belle2.PyDBObj("BeamParameterList", Belle2.PXDSimHit.Class()))
        # make sure the number of entries is consistent
        self.assertEqual(len(bplist), override_array.GetEntriesFast())

        # try looping over this and keep track about the number of elements
        for i, e in enumerate(bplist):
            # make sure the vertex is what we put in
            self.assertEqual(int(e.getVertex()[0]), i)
            # and that it's in general equal to the original one
            self.assertEqual(e, override_array[i])
            # and itself :D
            self.assertEqual(e, bplist[i])
            # and to a copy
            copy = Belle2.BeamParameters(e)
            self.assertEqual(e, copy)
            # unless we change the copy
            copy.setVertex(ROOT.TVector3(1, 2, 3), ROOT.std.vector("double")())
            self.assertNotEqual(e, copy)
            # and check that modification is blocked
            with self.assertRaises(AttributeError):
                e.setVertex(ROOT.TVector3(1, 2, 3), ROOT.std.vector("double")())

        self.assertEqual(i + 1, len(bplist))

        # make sure item assignment is off
        with self.assertRaises(TypeError):
            bplist[0] = Belle2.BeamParameters()


if __name__ == "__main__":
    unittest.main()

# @endcond
