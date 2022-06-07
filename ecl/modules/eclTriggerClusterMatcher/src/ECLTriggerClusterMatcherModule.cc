/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
//This module
#include <ecl/modules/eclTriggerClusterMatcher/ECLTriggerClusterMatcherModule.h>

//Framework
#include <framework/geometry/B2Vector3.h>

//MDST
#include <mdst/dataobjects/ECLCluster.h>

//Trg
#include <trg/ecl/dataobjects/TRGECLCluster.h>


using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLTriggerClusterMatcher);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLTriggerClusterMatcherModule::ECLTriggerClusterMatcherModule() : Module()
{
  // Set module properties
  setDescription("Match ECLTRGClusters to ECLClusters");
  addParam("minClusterEnergy", m_minClusterEnergy, "Minimum energy of the ECLCluster to be checked [GeV].", 0.1);
  addParam("maxAngle", m_maxAngle, "Maximum angle between an ECLCLuster and an ECLTRGCluster [rad]", 0.15);
  addParam("minFracEnergy", m_minFracEnergy, "Minimum energy fraction trg/cluster.", 0.);
  addParam("maxFracEnergy", m_maxFracEnergy, "Maximal energy fraction trg/cluster.", 2.);
  setPropertyFlags(c_ParallelProcessingCertified);
}

void ECLTriggerClusterMatcherModule::initialize()
{
  m_eclClusters.isRequired();
  m_eclTriggers.isRequired();
  m_eclClusters.registerRelationTo(m_eclTriggers);
}

void ECLTriggerClusterMatcherModule::event()
{
  for (auto& eclcluster : m_eclClusters) {
    const double eclclusterTheta = eclcluster.getTheta();
    const double eclclusterPhi = eclcluster.getPhi();

    double eclclusterE = 0.0;
    if (eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)) {
      eclclusterE = eclcluster.getEnergy(ECLCluster::EHypothesisBit::c_nPhotons);
    } else if (eclcluster.hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron)) {
      eclclusterE = eclcluster.getEnergy(ECLCluster::EHypothesisBit::c_neutralHadron);
    } else continue;

    // Users can re-run this with different settings, remove this bit.
    eclcluster.removeStatus(ECLCluster::EStatusBit::c_TriggerCluster);

    // Add status bit that shows that the matcher has run
    eclcluster.addStatus(ECLCluster::EStatusBit::c_TriggerClusterMatching);

    if (eclclusterE < m_minClusterEnergy) continue;  // skip low energy cluster

    // Lets ignore any problems from particle from non-IP locations.
    B2Vector3D cluster(1., 1., 1);
    cluster.SetTheta(eclclusterTheta);
    cluster.SetPhi(eclclusterPhi);

    for (const auto& ecltrigger : m_eclTriggers) {
      const double ecltriggerX = ecltrigger.getPositionX();
      const double ecltriggerY = ecltrigger.getPositionY();
      const double ecltriggerZ = ecltrigger.getPositionZ();
      const double ecltriggerE = ecltrigger.getEnergyDep();

      B2Vector3D trigger(ecltriggerX, ecltriggerY, ecltriggerZ);

      const double angle = cluster.Angle(trigger);

      double fracEnergy = -1.;
      if (eclclusterE > 0.0) fracEnergy = ecltriggerE / eclclusterE;

      // set relation(s) and status bit
      if (angle < m_maxAngle and fracEnergy > m_minFracEnergy and fracEnergy < m_maxFracEnergy) {
        eclcluster.addStatus(ECLCluster::EStatusBit::c_TriggerCluster);
        eclcluster.addRelationTo(&ecltrigger);
      }

    }
  }
}
