/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * See .h file for a description.                                         *
 *                                                                        *
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
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/dataobjects/ECLLocalMaximum.h>
#include <ecl/dataobjects/ECLShower.h>

// MDST
#include <mdst/dataobjects/ECLCluster.h>

// OTHER
#include <string>

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
  m_eclLocalMaximums(eclLocalMaximumArrayName()),
  m_eclShowers(eclShowerArrayName())
{
  // Set description.
  setDescription("ECLSplitterN2Module: Baseline reconstruction splitter code for the neutral hadron hypothesis.");

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

  // Register relations (we probably dont need all, but keep them for now for debugging).
  m_eclShowers.registerRelationTo(m_eclConnectedRegions);
  m_eclShowers.registerRelationTo(m_eclCalDigits);
  m_eclShowers.registerRelationTo(m_eclLocalMaximums);

}

void ECLSplitterN2Module::beginRun()
{
  ;
}

void ECLSplitterN2Module::event()
{
  B2DEBUG(175, "ECLCRSplitterN2Module::event()");

  // Loop over all connected regions (CR_.
  for (auto& aCR : m_eclConnectedRegions) {
    unsigned int iShower = 1;

    const auto aECLShower = m_eclShowers.appendNew();

    // Add relation to the CR.
    aECLShower->addRelationTo(&aCR);

    // Loop over all local maximums (LM).
    for (auto& aLM : aCR.getRelationsWith<ECLLocalMaximum>(eclLocalMaximumArrayName())) {
      // Add relation to the CR.
      aECLShower->addRelationTo(&aLM);
    }

    // Prepare shower variables.
    double highestEnergy = 0.0;
    double highestEnergyTime = 0.;
    double highestEnergyTimeResolution = 0.;
    double weightSum = 0.0;
    double energySum = 0.0;
    unsigned int highestEnergyID = 0;
    std::vector< ECLCalDigit > digits;
    std::vector< double > weights;

    // Loop over all digits that are related to the CR, they can be weighted (in the future?).
    auto relatedDigitsPairs = aCR.getRelationsTo<ECLCalDigit>(eclCalDigitArrayName());
    for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
      const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
      const auto weight = relatedDigitsPairs.weight(iRel);

      // Add Relation to ECLCalDigits.
      aECLShower->addRelationTo(aECLCalDigit, weight);

      // Find highest energetic crystal, its time, and its time resolution. This is not neceessarily the LM!
      const double energyDigit = aECLCalDigit->getEnergy();
      if (energyDigit > highestEnergy) {
        highestEnergy               = energyDigit * weight;
        highestEnergyID             = aECLCalDigit->getCellId();
        highestEnergyTime           = aECLCalDigit->getTime();
        highestEnergyTimeResolution = aECLCalDigit->getTimeResolution();
      }

      digits.push_back(*aECLCalDigit);
      weights.push_back(weight);

      weightSum += weight;
      energySum += energyDigit * weight;

    }

    const TVector3& showerposition = Belle2::ECL::computePositionLiLo(digits, weights, m_liloParameters);
    aECLShower->setTheta(showerposition.Theta());
    aECLShower->setPhi(showerposition.Phi());
    aECLShower->setR(showerposition.Mag());

    aECLShower->setEnergy(energySum);
    aECLShower->setEnergyRaw(energySum);
    aECLShower->setEnergyHighestCrystal(highestEnergy);
    aECLShower->setCentralCellId(highestEnergyID);
    aECLShower->setTime(highestEnergyTime);
    aECLShower->setDeltaTime99(highestEnergyTimeResolution);
    aECLShower->setNumberOfCrystals(weightSum);

    aECLShower->setShowerId(iShower);
    aECLShower->setHypothesisId(Belle2::ECLCluster::c_neutralHadron);
    aECLShower->setConnectedRegionId(aCR.getCRId());

    B2DEBUG(175, "N2 shower " << iShower);
    B2DEBUG(175, "  theta           = " << aECLShower->getTheta());
    B2DEBUG(175, "  phi             = " << aECLShower->getPhi());
    B2DEBUG(175, "  R               = " << aECLShower->getR());
    B2DEBUG(175, "  energy          = " << aECLShower->getEnergy());
    B2DEBUG(175, "  time            = " << aECLShower->getTime());
    B2DEBUG(175, "  time resolution = " << aECLShower->getDeltaTime99());

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
