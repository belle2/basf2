#!/usr/bin/env python3

##########################################################################
# basf2 (Belle II Analysis Software Framework)                           #
# Author: The Belle II Collaboration                                     #
#                                                                        #
# See git log for contributors and copyright holders.                    #
# This file is licensed under LGPL-3.0, see LICENSE.md.                  #
##########################################################################

"""
Check that the cached beam parameters of PCmsLabTransform follow the payloads:
two runs with different collision payloads are processed (with and without
multiprocessing), and in every event the cached values are compared to a direct
computation from the payload values of that run.
"""

import basf2 as b2
import b2test_utils as b2tu
import ROOT
from ROOT import Belle2

# Compares the (cached) rotations of PCmsLabTransform with a direct computation from the given values
ROOT.gInterpreter.Declare("""
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/utilities/LabToCms.h>
#include <algorithm>
#include <cmath>

namespace CmsTransformCacheTest {
  double maxRotationDifference(const ROOT::Math::XYZVector& boost, double angleXZ, double angleYZ)
  {
    const Belle2::PCmsLabTransform transform;
    const ROOT::Math::LorentzRotation labToCms = Belle2::LabToCms::rotateLabToCms(boost, angleXZ, angleYZ);
    const ROOT::Math::LorentzRotation cmsToLab = labToCms.Inverse();
    double expected[16], actual[16], maxDifference = 0;
    labToCms.GetComponents(expected);
    transform.rotateLabToCms().GetComponents(actual);
    for (int i = 0; i < 16; ++i) maxDifference = std::max(maxDifference, std::abs(expected[i] - actual[i]));
    cmsToLab.GetComponents(expected);
    transform.rotateCmsToLab().GetComponents(actual);
    for (int i = 0; i < 16; ++i) maxDifference = std::max(maxDifference, std::abs(expected[i] - actual[i]));
    return maxDifference;
  }
}
""")

#: Two sets of collision parameters: CMS energy, boost vector, angle XZ, angle YZ.
#: The CMS energy is stored as Double32_t (i.e. float) in the payload file, so values exactly representable as float are used.
VALUES_A = (10.578125, (0.041488088603337358, 0.0, 0.27252546681036521), 0.0057699660560828307, 0.0)
#: Different collision parameters
VALUES_B = (10.359375, (0.030, 0.001, 0.29), 0.0041, 0.0003)
#: Collision parameters for each (experiment, run): they alternate, so they change at every run boundary
BEAM_VALUES = {(0, 1): VALUES_A, (0, 2): VALUES_B, (0, 3): VALUES_A, (0, 4): VALUES_B}
#: Number of events per run
N_EVENTS_PER_RUN = 5
#: Total number of events
N_EVENTS = N_EVENTS_PER_RUN * len(BEAM_VALUES)


def create_payloads(database_path):
    """Store the collision payloads of each run in a local database."""
    b2.conditions.expert_settings(save_payloads=database_path)
    database = Belle2.Database.Instance()
    for (exp, run), (mass, boost, angle_xz, angle_yz) in BEAM_VALUES.items():
        iov = Belle2.IntervalOfValidity(exp, run, exp, run)

        invariant_mass = Belle2.CollisionInvariantMass()
        invariant_mass.setMass(mass, 0, 0)
        assert database.storeData("CollisionInvariantMass", invariant_mass, iov)

        boost_vector = Belle2.CollisionBoostVector()
        boost_vector.setBoost(ROOT.TVector3(*boost), ROOT.TMatrixDSym(3))
        assert database.storeData("CollisionBoostVector", boost_vector, iov)

        axis = Belle2.CollisionAxisCMS()
        axis.setAngles(angle_xz, angle_yz, ROOT.TMatrixDSym(2))
        axis.setSpread(ROOT.TMatrixDSym(2), 0, 0, 0)
        assert database.storeData("CollisionAxisCMS", axis, iov)


class CheckCmsTransform(b2.Module):
    """Compare the (cached) PCmsLabTransform values to the payload values of the current run."""

    def __init__(self):
        """Constructor"""
        super().__init__()
        self.set_property_flags(b2.ModulePropFlags.PARALLELPROCESSINGCERTIFIED)
        #: Event metadata, to know the current run
        self.event_metadata = None
        #: Number of checked events
        self.n_checked = 0

    def initialize(self):
        """Access the event metadata (created here, as the DataStore is reset between two calls of process())."""
        self.event_metadata = Belle2.PyStoreObj("EventMetaData")

    def event(self):
        """Check the transformation in each event."""
        exp, run = self.event_metadata.getExperiment(), self.event_metadata.getRun()
        mass, boost, angle_xz, angle_yz = BEAM_VALUES[(exp, run)]

        transform = Belle2.PCmsLabTransform()
        errors = []
        if abs(transform.getCMSEnergy() - mass) > 1e-12:
            errors.append(f"CMS energy {transform.getCMSEnergy()} instead of {mass}")
        actual_boost = transform.getBoostVector()
        actual_boost = (actual_boost.X(), actual_boost.Y(), actual_boost.Z())
        if any(abs(a - b) > 1e-12 for a, b in zip(actual_boost, boost)):
            errors.append(f"boost vector {actual_boost} instead of {boost}")
        if ROOT.CmsTransformCacheTest.maxRotationDifference(ROOT.Math.XYZVector(*boost), angle_xz, angle_yz) > 1e-12:
            errors.append("Lab <-> CMS rotations do not match the payloads")

        if errors:
            b2.B2FATAL(f"Wrong beam parameters in exp {exp} run {run}: " + "; ".join(errors))
        self.n_checked += 1

    def terminate(self):
        """Make sure all events were checked (only possible without multiprocessing, where one process sees all events)."""
        if b2.get_nprocesses() == 0 and self.n_checked != N_EVENTS:
            b2.B2FATAL(f"{self.n_checked} events checked instead of {N_EVENTS}")


def run(n_processes):
    """Process two runs with different payloads, with the given number of processes."""
    b2.set_nprocesses(n_processes)
    path = b2.Path()
    path.add_module("EventInfoSetter",
                    expList=[exp for exp, _ in BEAM_VALUES],
                    runList=[run for _, run in BEAM_VALUES],
                    evtNumList=[N_EVENTS_PER_RUN] * len(BEAM_VALUES))
    path.add_module(CheckCmsTransform())
    b2.process(path)


if __name__ == "__main__":
    b2tu.configure_logging_for_tests()
    with b2tu.clean_working_directory():
        database = "localdb/database.txt"
        # create the payloads in a subprocess, so that the database configuration of this process is not fixed yet
        assert b2tu.run_in_subprocess(target=create_payloads, database_path=database) == 0
        b2.conditions.override_globaltags()
        b2.conditions.prepend_testing_payloads(database)
        # single process, then multiprocessing: the DBStore is reset between the two calls of process()
        run(0)
        run(2)
