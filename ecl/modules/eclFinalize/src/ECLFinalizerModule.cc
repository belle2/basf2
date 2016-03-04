/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module converts the ecl dataobject(s) in the mdst dataobect(s)    *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
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
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLFinalizerModule::~ECLFinalizerModule()
{
}

void ECLFinalizerModule::initialize()
{
  // Register in datastore
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  eclShowers.registerInDataStore();

  StoreArray<ECLCluster> eclClusters(eclClusterArrayName());
  eclClusters.registerInDataStore();
  eclClusters.registerRelationTo(eclShowers);

}

void ECLFinalizerModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
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

    // get shower time
    const float ShowerTime = eclShower.getTime();

    // Loose timing cut is applied. 20150529 K.Miyabayashi (original comment)
    if (-300.0 < ShowerTime && ShowerTime < 200.0) {
      const auto eclCluster = eclClusters.appendNew();

      eclCluster->setTiming(eclShower.getTime());
      eclCluster->setEnergy(eclShower.getEnergy());
      eclCluster->setTheta(eclShower.getTheta());
      eclCluster->setPhi(eclShower.getPhi());
      eclCluster->setR(eclShower.getR());

      eclCluster->setE9oE25(eclShower.getE9oE25());

      eclCluster->setEnedepSum(eclShower.getUncEnergy()); // abuses UncEnergy from the Belle CF in the old code (TF)
      eclCluster->setNofCrystals(eclShower.getNHits());
      eclCluster->setHighestE(
        eclShower.getHighestEnergy()); // not available from original ECLShower, need to update ECLShower (or work around)

      double Mdst_Error[6] = {
        eclShower.getEnergyError(),
        0,
        eclShower.getPhiError(),
        0,
        0,
        eclShower.getThetaError()
      };
      eclCluster->setError(Mdst_Error);

      // ECLCluster has no "ID"-like structure, abuse the unused "CrystHealth"
      eclCluster->setCrystHealth(eclShower.getUniqueShowerId());

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
