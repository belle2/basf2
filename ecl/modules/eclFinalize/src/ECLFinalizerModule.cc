/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module converts the ecl dataobject(s) in the mdst dataobect(s)    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclFinalize/ECLFinalizerModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLShower.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>

// OTHER
#include <ecl/utility/ECLShowerId.h>

// ROOT
#include <TVector3.h>
#include <TMatrixFSym.h>
#include <TMath.h>

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLFinalizer)
REG_MODULE(ECLFinalizerPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
ECLFinalizerModule::ECLFinalizerModule() : Module()
{
  // Set description
  setDescription("ECLFinalizerModule: Converts ecl dataobjects to mdst dataobjects");

  addParam("clusterEnergyCutMin", m_clusterEnergyCutMin, "Min value for the cluster energy cut.",
           0.0 * Belle2::Unit::GeV);
  addParam("clusterEnergyCutMax", m_clusterEnergyCutMax, "Max value for the cluster energy cut.",
           999.9 * Belle2::Unit::GeV);
  addParam("highestEnergyCutMin", m_highestEnergyCutMin, "Min value for the highest energy cut.",
           0.0 * Belle2::Unit::GeV);
  addParam("highestEnergyCutMax", m_highestEnergyCutMax, "Max value for the highest energy cut.",
           999.9 * Belle2::Unit::GeV);
  addParam("clusterTimeCutMin", m_clusterTimeCutMin, "Min value for the cluster time cut.",
           -125.0 * Belle2::Unit::ns);
  addParam("clusterTimeCutMax", m_clusterTimeCutMax, "Max value for the cluster time cut.",
           125.0 * Belle2::Unit::ns);

  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLFinalizerModule::~ECLFinalizerModule()
{
}

void ECLFinalizerModule::initialize()
{
  // Register in datastore.
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  eclShowers.registerInDataStore();
  StoreArray<ECLCluster> eclClusters(eclClusterArrayName());
  eclClusters.registerInDataStore();

  // Register relations.
  eclClusters.registerRelationTo(eclShowers);

}

void ECLFinalizerModule::beginRun()
{
  // Do not use this blindly for database updates, they will probably not follow the concept of a "run"
  ;
}

void ECLFinalizerModule::event()
{
  // Input array
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // Output array
  StoreArray<ECLCluster> eclClusters(eclClusterArrayName());

  // loop over all ECLShowers
  for (const auto& eclShower : eclShowers) {

    // create an mdst cluster for each ecl shower
    if (!eclClusters) eclClusters.create();

    // get shower time, energy and highest energy for cuts
    const double showerTime = eclShower.getTime();
    const double showerEnergy = eclShower.getEnergy();
    const double highestEnergy = eclShower.getHighestEnergy();

    // Loose timing cut is applied. 20150529 K.Miyabayashi (original comment)
    // replaced -300..200 clocktick cut by +/- 125ns cut (Torbenm 24-March-2016) - THIS WILL BE REPLACED BY AN ENERGY DEPENDENT CUT USING THE RESOLUTION SOON
    if ((m_clusterTimeCutMin < showerTime and showerTime < m_clusterTimeCutMax)
        and (m_clusterEnergyCutMin < showerEnergy and showerEnergy < m_clusterEnergyCutMax)
        and (m_highestEnergyCutMin < highestEnergy and highestEnergy < m_highestEnergyCutMax)) {

      const auto eclCluster = eclClusters.appendNew();

      eclCluster->setTiming(eclShower.getTime());
      eclCluster->setEnergy(eclShower.getEnergy());
      eclCluster->setTheta(eclShower.getTheta());
      eclCluster->setPhi(eclShower.getPhi());
      eclCluster->setR(eclShower.getR());

      eclCluster->setEnedepSum(eclShower.getEnedepSum());
      eclCluster->setNofCrystals((int) eclShower.getNofCrystals());
      eclCluster->setHighestE(eclShower.getHighestEnergy());
      double Mdst_Error[6] = {
        eclShower.getEnergyError(),
        0,
        eclShower.getPhiError(),
        0,
        0,
        eclShower.getThetaError()
      };
      eclCluster->setError(Mdst_Error);

      // ECLCluster has no "ID"-like structures yet, use the unused "CrystHealth" until we fix the mdst object
      /** Utility unpacker of the shower id that contains CR, seed and hypothesis */
      ECLShowerId SUtility;
      int uniqueid = SUtility.getShowerId(eclShower.getConnectedRegionId(), eclShower.getHypothesisId(), eclShower.getShowerId());
      eclCluster->setCrystHealth(uniqueid);

      // set shower shapes variables
      eclCluster->setLAT(eclShower.getLateralEnergy());
      eclCluster->setE9oE25(eclShower.getE9oE25());

      // set relation to ECLShower
      eclCluster->addRelationTo(&eclShower);

    }

  }

}

void ECLFinalizerModule::endRun()
{
  ;
}

void ECLFinalizerModule::terminate()
{
  ;
}
