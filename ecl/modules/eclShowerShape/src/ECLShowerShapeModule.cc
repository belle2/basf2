/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/geometry/ECLGeometryPar.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerShape)
REG_MODULE(ECLShowerShapePureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerShapeModule::ECLShowerShapeModule() : Module()
{
  // Set description
  setDescription("ECLShowerShapeModule: Calculate ECL shower shape variable (e.g. E9E25)");
  setPropertyFlags(c_ParallelProcessingCertified);

}

ECLShowerShapeModule::~ECLShowerShapeModule()
{
}

void ECLShowerShapeModule::initialize()
{
  // Register in datastore
  /*
  StoreArray<ECLCalDigit> eclCalDigits(eclCalDigitArrayName());
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  StoreArray<ECLConnectedRegion> eclCRs(eclConnectedRegionArrayName());

  eclCalDigits.registerInDataStore();
  eclShowers.registerInDataStore();
  eclCRs.registerInDataStore();
  */
}

void ECLShowerShapeModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLShowerShapeModule::event()
{
  // input array
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());

  // loop over all ECLShowers
  for (auto& eclShower : eclShowers) {
    B2DEBUG(150, "Shower Id: " << eclShower.getShowerId());
    double lat = computeLateralEnergy(eclShower);
    eclShower.setLateralEnergy(float(lat));
    B2DEBUG(150, "lat: " << lat);

  }
}

void ECLShowerShapeModule::endRun()
{
}

void ECLShowerShapeModule::terminate()
{
}

double ECLShowerShapeModule::computeLateralEnergy(const ECLShower& shower) const
{

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();

  // loop over all <digit, weight> pairs that are related to this shower
  // EclNbr ecl;
  if (shower.getNHits() < 3) return 0;

  // Find the digis with two digits with the maximum energy
  double maxEnergy(0), secondMaxEnergy(0);
  unsigned int iMax(0), iSecondMax(0);
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    if (energy > maxEnergy) {
      maxEnergy = energy;
      iMax = iRel;
    }
  }
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    if (energy > secondMaxEnergy && iRel != iMax) {
      secondMaxEnergy = energy;
      iSecondMax = iRel;
    }
  }
  double sumE = 0;
  TVector3 cryFaceCenter;
  cryFaceCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    if (iRel != iMax && iRel != iSecondMax) {
      const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
      const auto weight = relatedDigitsPairs.weight(iRel);
      int cId = aECLCalDigit->getCellId();
      ECLGeometryPar* geometry = ECLGeometryPar::Instance();
      TVector3 pos = geometry->GetCrystalPos(cId);
      TVector3 deltaPos = pos - cryFaceCenter;
      double r = deltaPos.Mag();
      double r2 = r * r;
      sumE += weight * aECLCalDigit->getEnergy() * r2;
    }
  }
  const double r0sq = 5.*5. ; // average crystal dimension
  return sumE / (sumE + r0sq * (maxEnergy + secondMaxEnergy));
}
