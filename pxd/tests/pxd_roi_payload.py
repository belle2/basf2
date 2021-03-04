#!/usr/bin/env python3
# -*- coding: utf-8 -*-


import basf2 as b2
from ROOT import Belle2

import simulation

import PXDROIUnpackerModule

b2.set_random_seed(42)


class PxdROIPayloadTestModule(b2.Module):

    """
    module which checks if the roy payload from HLT can be created and depacked correctly
    """

    def sortROIs(self, unsortedPyStoreArray):
        """ sort ROI list
            Returns a python-list containing the ROIs
        """

        # first convert to a python-list to be able to sort
        py_list = list(unsortedPyStoreArray)

        # sort via a hierachy of sort keys
        return sorted(py_list,
                      key=lambda x: (
                          x.getSensorID(),
                          x.getMinUid(),
                          x.getMaxUid(),
                          x.getMinVid(),
                          x.getMaxVid()))

    def event(self):
        """ load the PXD Digits of the simulation and the packed/unpacked ones
        and compare them"""

        orgroisuns = Belle2.PyStoreArray('ROIs')
        if not orgroisuns:
            b2.B2FATAL("ROIs not in file")
            return

        unpackedroisuns = Belle2.PyStoreArray('PXDROIsPayHLT')
        if not unpackedroisuns:
            b2.B2FATAL("PXDROIsPayHLT not in file")
            return

        # To  make a 1:1 comparison, we have to sort both arrays.
        # As the order of the payload differs from the original array.
        # (its sorted by DHHID)
        # We have to sort both, because sorting by coordinate is not defined.

        orgrois = self.sortROIs(orgroisuns)
        unpackedrois = self.sortROIs(unpackedroisuns)

        # For some unknown reason, the ROI array contains a lot of
        # doubles. For creating the payload, these have been removed. to make a 1:1
        # comparison, we have to skip the following check and lateron skip ROIs
        # which are identical to the one before (ordered array).

        # if not len(orgrois) == len(unpackedrois):
        #     B2FATAL("Org. ROIs and Unpacked ROIs count not equal after packing and unpacking")

        print("Comparing %i ROIs " % len(orgrois))

        def f(x):
            return (
                x.getSensorID(),
                x.getMinUid(),
                x.getMaxUid(),
                x.getMinVid(),
                x.getMaxVid())

        # check all quantities between the direct and the packed/unpacked pxd digits
        # for i in range(len(orgrois)):
            # org = orgrois[i]
            # if i == 0 or f(org) != f(orgrois[i - 1]):
            # B2INFO(" Org $%X %3d %3d %3d %3d" % (org.getSensorID().getID(), org.getMinUid(),
            # org.getMaxUid(), org.getMinVid(), org.getMaxVid()))

        # for i in range(len(unpackedrois)):
            # unp = unpackedrois[i]
            # B2INFO(" Unp $%X %3d %3d %3d %3d" % (unp.getSensorID().getID(),
            # unp.getMinUid(), unp.getMaxUid(), unp.getMinVid(), unp.getMaxVid()))

        j = 0
        for i in range(len(orgrois)):
            org = orgrois[i]
            if i != 0 and f(org) == f(orgrois[i - 1]):
                b2.B2WARNING("Found the same ROI a second time (Double ROI)!")
                b2.B2WARNING(
                    "Check $%X %3d %3d %3d %3d" %
                    (org.getSensorID().getID(),
                     org.getMinUid(),
                        org.getMaxUid(),
                        org.getMinVid(),
                        org.getMaxVid()))
            if i == 0 or f(org) != f(orgrois[i - 1]):
                if j == len(unpackedrois):
                    b2.B2FATAL("Unpacked ROIs comparison exceeds array limit!")
                    break

                unp = unpackedrois[j]

                b2.B2INFO(
                    "Check Org $%X %3d %3d %3d %3d Unp $%X %3d %3d %3d %3d" %
                    (org.getSensorID().getID(),
                     org.getMinUid(),
                        org.getMaxUid(),
                        org.getMinVid(),
                        org.getMaxVid(),
                        unp.getSensorID().getID(),
                        unp.getMinUid(),
                        unp.getMaxUid(),
                        unp.getMinVid(),
                        unp.getMaxVid()))
                # compare all available quantities
                if unp.getMinUid() == 0 and unp.getMinVid() == 0 and unp.getMaxUid() == 250 - 1 and unp.getMaxVid() == 768 - 1:
                    b2.B2INFO("Full size ROI")
                    if org.getSensorID().getID() != unp.getSensorID().getID():
                        b2.B2INFO("DHHID changed")
                        if j == len(unpackedrois):
                            b2.B2FATAL("Unpacked ROIs comparison exceeds array limit!")
                            break
                        j += 1
                        unp = unpackedrois[j]

                if not(unp.getMinUid() == 0 and unp.getMinVid() == 0 and unp.getMaxUid() == 250 - 1 and unp.getMaxVid() == 768 - 1):
                    assert org.getSensorID().getID() == unp.getSensorID().getID()
                    assert org.getMinUid() == unp.getMinUid()
                    assert org.getMaxUid() == unp.getMaxUid()
                    assert org.getMinVid() == unp.getMinVid()
                    assert org.getMaxVid() == unp.getMaxVid()
                    j += 1


# to run the framework the used modules need to be registered
particlegun = b2.register_module('ParticleGun')
particlegun.param('pdgCodes', [13, -13])
particlegun.param('nTracks', 40)

# Create Event information
eventinfosetter = b2.register_module('EventInfoSetter')
eventinfosetter.param({'evtNumList': [10]})

main = b2.create_path()
# init path
main.add_module(eventinfosetter)
main.add_module(particlegun)
# add simulation for pxd only
# turn off the cleanup as the storearrays are needed
simulation.add_simulation(main, components=['PXD', 'SVD'], forceSetPXDDataReduction=True,
                          usePXDDataReduction=True, cleanupPXDDataReduction=False)
b2.set_module_parameters(main, type="Geometry", useDB=False, components=['PXD', 'SVD', 'MagneticFieldConstant4LimitedRSVD'])

roiPayloadAssembler = b2.register_module('ROIPayloadAssembler')
roiPayloadAssembler.param({"ROIListName": "ROIs", "SendAllDownscaler": 0,
                           "SendROIsDownscaler": 0, "CutNrROIs": 5, "AcceptAll": True})

main.add_module(roiPayloadAssembler)

# Show progress of processing
main.add_module('Progress')

main.add_module(PXDROIUnpackerModule.PXDPayloadROIUnpackerModule())

# run custom test module to check ROI befor and after packing/unpacking
main.add_module(PxdROIPayloadTestModule())

# Process events
b2.process(main)
