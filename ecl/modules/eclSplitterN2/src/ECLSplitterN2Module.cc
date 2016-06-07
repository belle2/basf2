/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Main reconstruction splitter code for the N2 hypothesis                *
 * (neutral hadron).                                                      *
 * Include all digits of the CR.                                          *
 *                                                                        *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca) (TF)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclSplitterN2/ECLSplitterN2Module.h>

// FRAMEWORK
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/utility/Position.h>

// OTHER
#include <string>
#include <utility>      // std::pair
#include <algorithm>    // std::find

// NAMESPACES
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module(s)
//-----------------------------------------------------------------
REG_MODULE(ECLSplitterN2)
REG_MODULE(ECLSplitterN2PureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLSplitterN2Module::ECLSplitterN2Module() : Module(), m_eclCalDigits(eclCalDigitArrayName()),
  m_eclConnectedRegions(eclConnectedRegionArrayName()),
  m_eclShowers(eclShowerArrayName()), m_eclEventInformation(eclEventInformationName())
{
  // Set description.
  setDescription("ECLSplitterN2Module: Baseline reconstruction splitter code for the neutral hadron hypothesis (N2).");

  // Set module parameters.
  addParam("positionMethod", m_positionMethod, "Position determination method.", std::string("lilo"));
  addParam("liloParameterA", m_liloParameterA, "Position determination linear-log. parameter A.", 4.0);
  addParam("liloParameterB", m_liloParameterB, "Position determination linear-log. parameter B.", 0.0);
  addParam("liloParameterC", m_liloParameterC, "Position determination linear-log. parameter C.", 0.0);

  // Set parallel processing flag.
  setPropertyFlags(c_ParallelProcessingCertified);
}

ECLSplitterN2Module::~ECLSplitterN2Module()
{
  // do not delete objects here, do it in terminate()!
}

void ECLSplitterN2Module::initialize()
{
  // Check user input.
  m_liloParameters.resize(3);
  m_liloParameters.at(0) = m_liloParameterA;
  m_liloParameters.at(1) = m_liloParameterB;
  m_liloParameters.at(2) = m_liloParameterC;

  // ECL dataobjects.
  m_eclCalDigits.registerInDataStore(eclCalDigitArrayName());
  m_eclConnectedRegions.registerInDataStore(eclConnectedRegionArrayName());
  m_eclShowers.registerInDataStore(eclShowerArrayName());
  m_eclEventInformation.registerInDataStore(eclEventInformationName());

  // Register relations (we probably dont need all, but keep them for now for debugging).
  m_eclConnectedRegions.registerRelationTo(m_eclCalDigits);
  m_eclShowers.registerRelationTo(m_eclConnectedRegions);
  m_eclShowers.registerRelationTo(m_eclCalDigits);

}

void ECLSplitterN2Module::beginRun()
{
  ;
}

void ECLSplitterN2Module::event()
{
  B2DEBUG(175, "ECLCRSplitterN2Module::event()");

  // Loop over all connected regions and create one shower per connected region.
  for (auto& aCR : m_eclConnectedRegions) {

    if (!m_eclShowers) m_eclShowers.create();
    const auto aECLShower = m_eclShowers.appendNew();

    // Add relation to the CR.
    aECLShower->addRelationTo(&aCR);

    // Fill shower variables.
    double highestEnergy = 0.0;
    double highestEnergyTime = 0.;
    double highestEnergyTimeResolution = 0.;
    double weightSum = 0.0;
    double energySum = 0.0;
    unsigned int highestEnergyID = 0;

    // Prepare vectors for position calculation.
    const double weight = 1.0;
    std::vector< ECLCalDigit > digits;
    std::vector< double > weights;

    for (auto& aECLCalDigit : aCR.getRelationsTo<ECLCalDigit>()) {

      // Add Relation to ECLCalDigits.
      aECLShower->addRelationTo(&aECLCalDigit, weight);

      // Find highest energetic crystal, its time, and its time resolution.
      const double energyDigit = aECLCalDigit.getEnergy();
      if (energyDigit > highestEnergy) {
        highestEnergy               = energyDigit * weight;
        highestEnergyID             = aECLCalDigit.getCellId();
        highestEnergyTime           = aECLCalDigit.getTime();
        highestEnergyTimeResolution = aECLCalDigit.getTimeResolution();
      }

      // Fill vectors for position calculation.
      digits.push_back(aECLCalDigit);
      weights.push_back(weight);

      weightSum += weight;
      energySum += energyDigit * weight;
    }

    const TVector3& showerposition = Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters);
    aECLShower->setTheta(showerposition.Theta());
    aECLShower->setPhi(showerposition.Phi());
    aECLShower->setR(showerposition.Mag());

    aECLShower->setEnergy(energySum);
    aECLShower->setEnedepsum(energySum);
    aECLShower->setHighestEnergy(highestEnergy);
    aECLShower->setCentralCellId(highestEnergyID);
    aECLShower->setTime(highestEnergyTime);
    aECLShower->setTimeResolution(highestEnergyTimeResolution);
    aECLShower->setNofCrystals(weightSum);

    aECLShower->setShowerId(1); // always one (only this single shower in the CR)
    aECLShower->setHypothesisId(Belle2::ECLConnectedRegion::c_N2);
    aECLShower->setConnectedRegionId(aCR.getCRId());

  } // end auto& aCR

}


void ECLSplitterN2Module::endRun()
{
  ;
}


void ECLSplitterN2Module::terminate()
{
  ;
}
