/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * The "old" Belle II eclRecShower within a combined                      *
 * Connected Region Finder and Splitter.                                  *
 *                                                                        *
 *  THIS IS AN INTERMEDIATE STEP TOWARDS THE FULL REFACTORING!            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * Original code: Poyuan Chen, Vishal                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclCRFindingAndSplitting/ECLCRFinderAndSplitterModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h> // shoud be removed asap (TF)

// ECL BELLE CLUSTERING VIA RECLIB
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCF.h>

// OTHER

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLCRFinderAndSplitter)
REG_MODULE(ECLCRFinderAndSplitterPureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLCRFinderAndSplitterModule::ECLCRFinderAndSplitterModule() : Module()
{
  // Set description
  setDescription("ECLCRFinderAndSplitterModule: Searches CRs and finds clusters in one step (reproduce old B2 reco behaviour)");
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLCRFinderAndSplitterModule::~ECLCRFinderAndSplitterModule()
{
}

void ECLCRFinderAndSplitterModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;

  // CPU time start
  m_timeCPU = clock() * Unit::us;

  // Register in datastore
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLHitAssignment> eclHitAssignments(eclHitAssignmentArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());

  eclCalDigits.registerInDataStore();
  eclHitAssignments.registerInDataStore();
  eclShowers.registerInDataStore();
  eclCRs.registerInDataStore();

  eclShowers.registerRelationTo(eclCRs);
  eclShowers.registerRelationTo(eclCalDigits);

}


void ECLCRFinderAndSplitterModule::beginRun()
{
  // Do not use beginRun for Database access

  B2INFO("ECLCRFinderAndSplitterModule: Processing run: " << m_nRun);
}


void ECLCRFinderAndSplitterModule::event()
{
  // we assume/define this is the "all photon" hypothesis N1 from the design proposal
  const int cHypothesis = 5;

  // input array
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());

  // output arrays
  StoreArray<ECLHitAssignment> eclHitAssignments(eclHitAssignmentArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());

  // Cluster finder
  std::cout.unsetf(std::ios::scientific);
  std::cout.precision(6);
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();

  // Loop over all ECLCalDigits and add them to the reclib cluster finder (original comment (TF))
  for (const auto& aECLCalDigit : eclCalDigits) {
    const double energy = (aECLCalDigit.getEnergy());

    const int cId = (aECLCalDigit.getCellId() - 1);

    if (energy < 0.) {
      continue; //skip negative energy values (original comment (TF))
    }

    // Register hit to cluster finder (cf) (original comment (TF))
    cf.Accumulate(m_nEvent, energy, cId);
  }

  // Call the CRF. (original comment (TF))
  cf.SearchCRs();
  cf.Attributes();

  // counter for connected regions
  int nCR = 0;

  // counter for showers (to use old hitassignments)
  // (there is another counter for showers within a CR!)
  int nShower = 0;

  // Loop over Connected Region (CR) - careful, this is a different definition than we use in B2 (TF)
  for (const auto& bellecr : cf.CRs()) {

    // counter for showers within CR(TF)
    int nShowerWithinCR = 0;

    // create connected region dataobject (TF)
    if (!eclCRs) eclCRs.create();
    const auto aECLCR = eclCRs.appendNew();
    aECLCR->setCRId(nCR);

    // Each Shower is pointed from the CR. (original comment (TF))
    for (const auto& shower : bellecr.Showers()) {

      // create ECLShower (dataobject to be modified soon)
      if (!eclShowers) eclShowers.create();
      const auto aECLShower = eclShowers.appendNew();

      // the old shower id (just a counter) must be replaced by the more complex new id
      // you can still use this just as a number - but you can also extract more from it
      const int uniqueId = m_SUtility.getShowerId(nCR, cHypothesis, nShowerWithinCR);

      TEclCFShower iSh = shower.second;

      // In order for finding the highest energy crystal in a shower and (original comment (TF))
      // that crystals' timing. 20150529 K.Miyabayashi (original comment (TF))
      double v_HiEnergy = -9999999;
      double v_TIME = -10;

      std::vector<MEclCFShowerHA> HAs = iSh.HitAssignment();
      for (std::vector<MEclCFShowerHA>::iterator iHA = HAs.begin();
           iHA != HAs.end(); ++iHA) {

        if (!eclHitAssignments) eclHitAssignments.create();
        const auto eclHitAssignment = eclHitAssignments.appendNew();
        eclHitAssignment->setShowerId(nShower);
        eclHitAssignment->setCellId(iHA->Id() + 1);

        // From ECLDigit information, the highest energy crystal is found. (original comment (TF))
        // To speed up, Mapping to be used, by Vishal (original comment (TF))
        for (const auto& aECLCalDigit : eclCalDigits) {
          if (aECLCalDigit.getCellId() != (iHA->Id() + 1)) continue;

          // add relation to the shower
          aECLShower->addRelationTo(&aECLCalDigit);

          // find the highest energy
          const double v_FitEnergy    = aECLCalDigit.getEnergy(); //TF
          const double v_FitTime      = aECLCalDigit.getTime(); //TF

          // If the crystal has highest energy, keep its information. (original comment (TF))
          if (v_HiEnergy < v_FitEnergy) {
            v_HiEnergy = v_FitEnergy;
            v_TIME = v_FitTime;
          }
        } // end digit loop
      } // end MEclCFShowerHA loop

      // no "corrections" anymore in this module! --> eclShowerCorrection (TF)

//      // create ECLShower (dataobject to be modified soon)
//      if (!eclShowers) eclShowers.create();
//      const auto aECLShower = eclShowers.appendNew();

      // fill ECLShower
      aECLShower->setShowerId(nShower); // must be changed to uniqueid as soon as hitassignment is gone
      aECLShower->setEnergy(shower.second.Energy());
      aECLShower->setTheta(shower.second.Theta());
      aECLShower->setPhi(shower.second.Phi());
      aECLShower->setR(shower.second.Distance());
      aECLShower->setMass(shower.second.Mass());
      aECLShower->setWidth(shower.second.Width()); // should be done in eclShowerShape (TF)
      aECLShower->setE9oE25(shower.second.E9oE25()); // should be done in eclShowerShape (TF)
      aECLShower->setE9oE25unf(shower.second.E9oE25unf()); // should be done in eclShowerShape (TF)
      aECLShower->setNHits(shower.second.NHits());
      aECLShower->setStatus(shower.second.Status());
      aECLShower->setGrade(shower.second.Grade());
      aECLShower->setUncEnergy(shower.second.UncEnergy());
      aECLShower->setTime(v_TIME);

      aECLShower->setHighestEnergy(v_HiEnergy); //(TF)
      aECLShower->setUniqueShowerId(uniqueId); //(TF)

      nShower++; // Increment counter for ECLShowers (TF)
      nShowerWithinCR++; // Increment counter for ECLShowers within this ECLCR (TF)

      // set relation to the connected region (TF), there can be multiple showers per CR (but only one CR per shower)
      aECLShower->addRelationTo(aECLCR);

      // no covariance matrix anymore in this module! --> eclCovariance (TF)

      // no cluster mdst object anymore in this module! --> eclFinalize (TF)

    } //end loop shower

    nCR++; // Increment counter for ECLCRs (TF)

  } //end loop connected region

  // increment event counter
  m_nEvent++;

}


void ECLCRFinderAndSplitterModule::endRun()
{
  // increment run counter
  m_nRun++;
}


void ECLCRFinderAndSplitterModule::terminate()
{
  // set the CPU time
  m_timeCPU = clock() * Unit::us - m_timeCPU;
}
