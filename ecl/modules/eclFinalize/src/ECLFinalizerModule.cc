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
  setDescription("ECLFinalizerModule: Converts ecl shower dataobjects to mdst eclcluster dataobjects");

  addParam("clusterEnergyCutMin", m_clusterEnergyCutMin, "Min value for the cluster energy cut.",
           20.0 * Belle2::Unit::MeV);
  addParam("clusterTimeCutMaxEnergy", m_clusterTimeCutMaxEnergy, "All clusters above this energy are kept.",
           50.0 * Belle2::Unit::MeV);

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

    // get shower time, energy and highest energy for cuts
    const double showerTime = eclShower.getTime();
    const double showerdt99 = eclShower.getDeltaTime99();
    const double showerEnergy = eclShower.getEnergy();

    // only keep showers above an energy threshold (~20MeV) and if their time is within
    // the 99% coverage of the time resolution (failed fits pass as well)
    // high energetic clusters (~50MeV+) pass as well in order to keep all out of time
    // background events that may damage real clusters
    if (showerEnergy > m_clusterEnergyCutMin
        and ((fabs(showerTime) < showerdt99) or (showerEnergy > m_clusterTimeCutMaxEnergy))) {

      // create an mdst cluster for each ecl shower
      const auto eclCluster = eclClusters.appendNew();

      // set all variables
      eclCluster->setStatus(eclShower.getStatus());
      eclCluster->setConnectedRegionId(eclShower.getConnectedRegionId());
      eclCluster->setHypothesisId(eclShower.getHypothesisId());
      eclCluster->setClusterId(eclShower.getShowerId());

      eclCluster->setEnergy(eclShower.getEnergy());
      eclCluster->setEnergyRaw(eclShower.getEnergyRaw());
      eclCluster->setEnergyHighestCrystal(eclShower.getEnergyHighestCrystal());

      double covmat[6] = {
        eclShower.getUncertaintyEnergy()* eclShower.getUncertaintyEnergy(),
        0.0,
        eclShower.getUncertaintyPhi()* eclShower.getUncertaintyPhi(),
        0.0,
        0.0,
        eclShower.getUncertaintyTheta()* eclShower.getUncertaintyTheta()
      };
      eclCluster->setCovarianceMatrix(covmat);

      //TMatrixDSym covmatecls = eclShower.getCovarianceMatrix3x3();
      //covmatecls.Print();
      //TMatrixDSym covmatecl = eclCluster->getCovarianceMatrix3x3();
      //covmatecl.Print();

      // m_deltaL is set in track-cluster matching
      // m_minTrkDistance is set in track-cluster matching

      eclCluster->setAbsZernike40(eclShower.getAbsZernike40());
      eclCluster->setAbsZernike51(eclShower.getAbsZernike51());
      eclCluster->setZernikeMVA(eclShower.getZernikeMVA());
      eclCluster->setE1oE9(eclShower.getE1oE9());
      eclCluster->setE9oE21(eclShower.getE9oE21());
      eclCluster->setSecondMoment(eclShower.getSecondMoment());
      eclCluster->setLAT(eclShower.getLateralEnergy());
      eclCluster->setNumberOfCrystals(eclShower.getNumberOfCrystals());
      eclCluster->setTime(eclShower.getTime());
      eclCluster->setDeltaTime99(eclShower.getDeltaTime99());
      eclCluster->setTheta(eclShower.getTheta());
      eclCluster->setPhi(eclShower.getPhi());
      eclCluster->setR(eclShower.getR());

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
