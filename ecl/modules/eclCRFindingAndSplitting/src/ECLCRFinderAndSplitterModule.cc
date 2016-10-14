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

// ECL BELLE CLUSTERING VIA RECLIB
#include <ecl/rec_lib/TEclCFCR.h>
#include <ecl/rec_lib/TRecEclCF.h>
#include <ecl/rec_lib/TRecEclCFParameters.h>
#include <ecl/rec_lib/TEclCFShower.h>
#include <ecl/rec_lib/TRecEclCF.h>

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
  m_nEvent  = 0;

  // Register in datastore
  // input array
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  // output arrays
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclCRs.registerInDataStore(eclConnectedRegionArrayName());

  m_eclShowers.registerRelationTo(m_eclCRs);
  m_eclShowers.registerRelationTo(m_eclCalDigits);
}


void ECLCRFinderAndSplitterModule::beginRun()
{
}


void ECLCRFinderAndSplitterModule::event()
{
  // we assume/define this is the "all photon" hypothesis N1 from the design proposal
  const int cHypothesis = 5;

  /** Utility unpacker of the shower id that contains CR, seed and hypothesis */
//  ECLShowerId SUtility;

  // Cluster finder
  TRecEclCF& cf = TRecEclCF::Instance();
  cf.Clear();

  // Loop over all ECLCalDigits and add them to the reclib cluster finder (original comment (TF))
  for (const auto& cd : m_eclCalDigits) {
    const double energy = cd.getEnergy();
    if (energy < 0.) continue; //skip negative energy values (original comment (TF))
    const int cId = cd.getCellId() - 1;
    // Register hit to cluster finder (cf) (original comment (TF))
    cf.Accumulate(m_nEvent, energy, cId);
  }

  // Call the CRF. (original comment (TF))
  cf.SearchCRs();
  cf.Attributes();

  // counter for connected regions
  int nCR = 1;

  // counter for showers (to use old hitassignments)
  // (there is another counter for showers within a CR!)
  int nShower = 0;

  // Loop over Connected Region (CR) - careful, this is a different definition than we use in B2 (TF)
  for (const TEclCFCR& bellecr : cf.CRs()) {
    // counter for showers within CR(TF)
    int nShowerWithinCR = 1;

    // create connected region dataobject (TF)
    ECLConnectedRegion* aECLCR = m_eclCRs.appendNew();
    aECLCR->setCRId(nCR);

    // Each Shower is pointed from the CR. (original comment (TF))
    const EclCFShowerMap& showers = bellecr.Showers();
    for (const std::pair<EclIdentifier, TEclCFShower>& it : showers) {
      const TEclCFShower& shower = it.second;

      // create ECLShower (dataobject to be modified soon)
      ECLShower* aECLShower = m_eclShowers.appendNew();

      // In order for finding the highest energy crystal in a shower and (original comment (TF))
      // that crystals' timing. 20150529 K.Miyabayashi (original comment (TF))
      double v_HiEnergy = -9999999;
      double v_TIME = -10;

      const std::vector<MEclCFShowerHA>& HAs = shower.HitAssignment();
      for (std::vector<MEclCFShowerHA>::const_iterator iHA = HAs.begin(); iHA != HAs.end(); ++iHA) {

        // From ECLDigit information, the highest energy crystal is found. (original comment (TF))
        // To speed up, Mapping to be used, by Vishal (original comment (TF))
        for (const auto& cd : m_eclCalDigits) {
          if (cd.getCellId() - 1 != iHA->Id()) continue;

          // add relation to the shower
          aECLShower->addRelationTo(&cd);

          // find the highest energy
          const double v_FitEnergy = cd.getEnergy(); //TF
          // If the crystal has highest energy, keep its information. (original comment (TF))
          if (v_HiEnergy < v_FitEnergy) {
            v_HiEnergy = v_FitEnergy;
            v_TIME = cd.getTime();
          }
        }
      }

      // fill ECLShower
      aECLShower->setShowerId(nShowerWithinCR);
      aECLShower->setEnergy(shower.Energy());
      aECLShower->setEnergyRaw(shower.UncEnergy());
      aECLShower->setTheta(shower.Theta());
      aECLShower->setPhi(shower.Phi());
      aECLShower->setR(shower.Distance());
//      aECLShower->setMass(shower.Mass());
//      aECLShower->setWidth(shower.Width()); // should be done in eclShowerShape (TF)
      aECLShower->setE9oE21(shower.E9oE25()); // should be done in eclShowerShape (TF)
//      aECLShower->setE9oE25unf(shower.E9oE25unf()); // should be done in eclShowerShape (TF)
      aECLShower->setNumberOfCrystals((double) shower.NHits());
      aECLShower->setStatus(0); // status is not used in this old clustering
//      aECLShower->setStatus(shower.Status());
//      aECLShower->setGrade(shower.Grade());
//      aECLShower->setUncEnergy(shower.UncEnergy());
      aECLShower->setTime(v_TIME);

      aECLShower->setEnergyHighestCrystal(v_HiEnergy); //(TF)
      // the old shower id (just a counter) must be replaced by the more complex new id
      // you can still use this just as a number - but you can also extract more from it
//      const int uniqueId = SUtility.getShowerId(nCR, cHypothesis, nShowerWithinCR);
//      aECLShower->setUniqueShowerId(uniqueId); //(TF)

      aECLShower->setConnectedRegionId(nCR);
      aECLShower->setHypothesisId(cHypothesis);

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
}


void ECLCRFinderAndSplitterModule::terminate()
{
}
