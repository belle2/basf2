/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
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
}

void ECLShowerShapeModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLShowerShapeModule::event()
{
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  for (auto& eclShower : eclShowers)
    eclShower.setLateralEnergy(computeLateralEnergy(eclShower));
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
  if (shower.getNofCrystals() < 3.0) return 0;

  // Find the two digits with the maximum energy
  double maxEnergy(0), secondMaxEnergy(0);
  unsigned int iMax(0), iSecondMax(0);
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    if (energy > maxEnergy) {
      maxEnergy = energy;
      iMax = iRel;
      secondMaxEnergy = maxEnergy;
      iSecondMax = iMax;
    } else if (energy > secondMaxEnergy) {
      secondMaxEnergy = energy;
      iSecondMax = iRel;
    }
  }
  double sumE = 0;
  TVector3 cryCenter;
  cryCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    if (iRel != iMax && iRel != iSecondMax) {
      const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
      const auto weight = relatedDigitsPairs.weight(iRel);
      int cId = aECLCalDigit->getCellId();
      ECLGeometryPar* geometry = ECLGeometryPar::Instance();
      TVector3 pos = geometry->GetCrystalPos(cId - 1);
      TVector3 deltaPos = pos - cryCenter;
      double r = deltaPos.Mag();
      double r2 = r * r;
      sumE += weight * aECLCalDigit->getEnergy() * r2;
    }
  }
  const double r0sq = 5.*5. ; // average crystal dimension
  return sumE / (sumE + r0sq * (maxEnergy + secondMaxEnergy));
}

