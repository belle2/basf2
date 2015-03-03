#include <framework/logging/Logger.h>
#include <tracking/modules/genfitter/Geant4MaterialInterface.h>
#include <geometry/GeometryManager.h>

#include "genfit/Exception.h"

#include <TGeoMedium.h>
#include <TGeoMaterial.h>
#include <TGeoManager.h>
#include <assert.h>
#include <math.h>

#include "G4ThreeVector.hh"
#include "G4Navigator.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Material.hh"

static const bool debug = false;
//static const bool debug = true;

// Debug flag, use for comparison if differences in behavior occur.
//#define LOOK_IN_TGEO

using namespace Belle2;

namespace {
#ifdef LOOK_IN_TGEO

  /*
  Reference for elemental mean excitation energies at:
  http://physics.nist.gov/PhysRefData/XrayMassCoef/tab1.html

  Code ported from GEANT 3
  */

  const int MeanExcEnergy_NELEMENTS = 93; // 0 = vacuum, 1 = hydrogen, 92 = uranium
  const double MeanExcEnergy_vals[] = {1.e15, 19.2, 41.8, 40.0, 63.7, 76.0, 78., 82.0, 95.0, 115.0, 137.0, 149.0, 156.0, 166.0, 173.0, 173.0, 180.0, 174.0, 188.0, 190.0, 191.0, 216.0, 233.0, 245.0, 257.0, 272.0, 286.0, 297.0, 311.0, 322.0, 330.0, 334.0, 350.0, 347.0, 348.0, 343.0, 352.0, 363.0, 366.0, 379.0, 393.0, 417.0, 424.0, 428.0, 441.0, 449.0, 470.0, 470.0, 469.0, 488.0, 488.0, 487.0, 485.0, 491.0, 482.0, 488.0, 491.0, 501.0, 523.0, 535.0, 546.0, 560.0, 574.0, 580.0, 591.0, 614.0, 628.0, 650.0, 658.0, 674.0, 684.0, 694.0, 705.0, 718.0, 727.0, 736.0, 746.0, 757.0, 790.0, 790.0, 800.0, 810.0, 823.0, 823.0, 830.0, 825.0, 794.0, 827.0, 826.0, 841.0, 847.0, 878.0, 890.0};

  double
  MeanExcEnergy_get(int Z)
  {
    assert(Z >= 0 && Z < MeanExcEnergy_NELEMENTS);
    return MeanExcEnergy_vals[Z];
  }


  double
  MeanExcEnergy_get(TGeoMaterial* mat)
  {
    if (mat->IsMixture()) {
      double logMEE = 0.;
      double denom  = 0.;
      TGeoMixture* mix = (TGeoMixture*)mat;
      for (int i = 0; i < mix->GetNelements(); ++i) {
        int index = int(floor((mix->GetZmixt())[i]));
        logMEE += 1. / (mix->GetAmixt())[i] * (mix->GetWmixt())[i] * (mix->GetZmixt())[i] * log(MeanExcEnergy_get(index));
        denom  += (mix->GetWmixt())[i] * (mix->GetZmixt())[i] * 1. / (mix->GetAmixt())[i];
      }
      logMEE /= denom;
      return exp(logMEE);
    } else { // not a mixture
      int index = int(floor(mat->GetZ()));
      return MeanExcEnergy_get(index);
    }
  }

#endif

  G4ThreeVector where;

}


void G4SafeNavigator::SetWorldVolume(G4VPhysicalVolume* pWorld)
{
  G4Box* box = dynamic_cast<G4Box*>(pWorld->GetLogicalVolume()->GetSolid());
  assert(box && "World volume a G4Box?");
  dX_ = box->GetXHalfLength();
  dY_ = box->GetYHalfLength();
  dZ_ = box->GetZHalfLength();
  nav_.SetWorldVolume(pWorld);

#ifdef LOOK_IN_TGEO
  if (!gGeoManager) {
    B2INFO("Building TGeo representation.");
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
  }
#endif
}


G4VPhysicalVolume* G4SafeNavigator::LocateGlobalPointAndSetup(const G4ThreeVector& point,
    const G4ThreeVector* direction,
    const G4bool pRelativeSearch,
    const G4bool ignoreDirection)
{
  G4ThreeVector p(point);
  if (fabs(p.x()) >= dX_)
    p.setX(0);
  if (fabs(p.y()) >= dY_)
    p.setY(0);
  if (fabs(p.z()) >= dZ_)
    p.setZ(0);
  where = p;

  G4ThreeVector dir(*direction);
  if (std::isnan(dir.x()) || std::isnan(dir.y()) || std::isnan(dir.z())) {
    B2WARNING("nan in direction in G4SafeNavigator::LocateGlobalPointAndSetup");
    dir.set(1., 0., 0.);
  }
  dir.setMag(1.);
  return nav_.LocateGlobalPointAndSetup(p, &dir, pRelativeSearch, ignoreDirection);
}

G4double G4SafeNavigator::ComputeStep(const G4ThreeVector& pGlobalPoint,
                                      const G4ThreeVector& pDirection,
                                      const G4double pCurrentProposedStepLength,
                                      G4double&  pNewSafety)
{
  G4ThreeVector p(pGlobalPoint);
  if (fabs(p.x()) >= dX_)
    p.setX(0);
  if (fabs(p.y()) >= dY_)
    p.setY(0);
  if (fabs(p.z()) >= dZ_)
    p.setZ(0);
  where = p;

  G4ThreeVector dir(pDirection);
  if (std::isnan(dir.x()) || std::isnan(dir.y()) || std::isnan(dir.z())) {
    B2WARNING("nan in direction in G4SafeNavigator::ComputeStep");
    dir.set(1., 0., 0.);
  }
  dir.setMag(1.);
  return nav_.ComputeStep(p, dir, pCurrentProposedStepLength, pNewSafety);
}

G4double G4SafeNavigator::CheckNextStep(const G4ThreeVector& pGlobalPoint,
                                        const G4ThreeVector& pDirection,
                                        const G4double pCurrentProposedStepLength,
                                        G4double& pNewSafety)
{
  G4ThreeVector p(pGlobalPoint);
  if (fabs(p.x()) >= dX_)
    p.setX(0);
  if (fabs(p.y()) >= dY_)
    p.setY(0);
  if (fabs(p.z()) >= dZ_)
    p.setZ(0);
  where = p;

  G4ThreeVector dir(pDirection);
  if (std::isnan(dir.x()) || std::isnan(dir.y()) || std::isnan(dir.z())) {
    B2WARNING("nan in direction in G4SafeNavigator::CheckNextStep");
    dir.set(1., 0., 0.);
  }
  dir.setMag(1.);
  return nav_.CheckNextStep(p, dir, pCurrentProposedStepLength, pNewSafety);
}


Geant4MaterialInterface::Geant4MaterialInterface()
  : nav_(new G4SafeNavigator()), currentVolume_(0)
{
  G4VPhysicalVolume* world = geometry::GeometryManager::getInstance().getTopVolume();
  nav_->SetWorldVolume(world);
}


bool
Geant4MaterialInterface::initTrack(double posX, double posY, double posZ,
                                   double dirX, double dirY, double dirZ)
{
  G4ThreeVector pos(posX * CLHEP::cm, posY * CLHEP::cm, posZ * CLHEP::cm);
  G4ThreeVector dir(dirX, dirY, dirZ);
  const G4VPhysicalVolume* newVolume = nav_->LocateGlobalPointAndSetup(pos, &dir, false, false);
  if (!newVolume)
    std::cout << "hier " << pos << std::endl;
#ifdef LOOK_IN_TGEO
  gGeoManager->IsSameLocation(posX, posY, posZ, kTRUE);
  // Set the intended direction.
  gGeoManager->SetCurrentDirection(dirX, dirY, dirZ);
#endif

  bool volChanged = newVolume != currentVolume_;
  currentVolume_ = newVolume;

  return volChanged;
}


void
Geant4MaterialInterface::getMaterialParameters(double& density,
                                               double& Z,
                                               double& A,
                                               double& radiationLength,
                                               double& mEE)
{
  if (!currentVolume_)
    std::cout << " oder hier " << where << std::endl;
  assert(currentVolume_);

  const G4Material* mat = currentVolume_->GetLogicalVolume()->GetMaterial();

  if (mat->GetNumberOfElements() == 1) {
    Z = mat->GetZ();
    A = mat->GetA();
  } else {
    // Calculate weight-averaged A, Z
    A = Z = 0;
    for (unsigned i = 0; i < mat->GetNumberOfElements(); ++i) {
      const G4Element* element = (*mat->GetElementVector())[i];
      Z += element->GetZ() * mat->GetFractionVector()[i];
      A += element->GetA() * mat->GetFractionVector()[i];
    }
  }

  density = mat->GetDensity() / CLHEP::g * CLHEP::cm3;
  // Z is fine
  A *= CLHEP::mole / CLHEP::g;
  radiationLength = mat->GetRadlen() / CLHEP::cm;
  mEE = mat->GetIonisation()->GetMeanExcitationEnergy() / CLHEP::eV;


#ifdef LOOK_IN_TGEO
  TGeoMaterial* matTG = gGeoManager->GetCurrentVolume()->GetMedium()->GetMaterial();

  double densityTG         = matTG->GetDensity();
  double ZTG               = matTG->GetZ();
  double ATG               = matTG->GetA();
  double radiationLengthTG = matTG->GetRadLen();
  double mEETG             = MeanExcEnergy_get(matTG);

  std::cout << mat->GetName() << " " << matTG->GetName()
            << " rho " << density << " " << densityTG
            << " Z " << Z << " " << ZTG
            << " A " << A << " " << ATG
            << " X0 " << radiationLength << " " << radiationLengthTG
            << " mEE " << mEE << " " << mEETG
            << std::endl;
#endif
}


void
Geant4MaterialInterface::getMaterialParameters(genfit::MaterialProperties& parameters)
{
  double density, Z, A, radLen, mEE;
  this->getMaterialParameters(density, Z, A, radLen, mEE);
  parameters.setMaterialProperties(density, Z, A, radLen, mEE);
}


double
Geant4MaterialInterface::findNextBoundary(const genfit::RKTrackRep* rep,
                                          const genfit::M1x7& stateOrig,
                                          double sMax, // signed
                                          bool varField)
{
  const double delta(1.E-2); // cm, distance limit beneath which straight-line steps are taken.
  const double epsilon(1.E-1); // cm, allowed upper bound on arch deviation from straight line

  genfit::M1x3 SA;
  genfit::M1x7 state7, oldState7;
  memcpy(oldState7, stateOrig, sizeof(state7));

  int stepSign(sMax < 0 ? -1 : 1);

  G4ThreeVector pointOrig(stateOrig[0] * CLHEP::cm,
                          stateOrig[1] * CLHEP::cm,
                          stateOrig[2] * CLHEP::cm);
  G4ThreeVector dirOrig(stepSign * stateOrig[3],
                        stepSign * stateOrig[4],
                        stepSign * stateOrig[5]);

  double s = 0;  // trajectory length to boundary

  const unsigned maxIt = 300;
  unsigned it = 0;

  // Initialize the geometry to the current location (set by caller).
  double safety;
  double slDist = nav_->ComputeStep(pointOrig, dirOrig, fabs(sMax) * CLHEP::cm, safety);
  if (slDist == kInfinity)
    slDist = fabs(sMax);
  else
    slDist /= CLHEP::cm;
  safety /= CLHEP::cm;
  double step = slDist;

#ifdef LOOK_IN_TGEO
  gGeoManager->IsSameLocation(stateOrig[0], stateOrig[1], stateOrig[2], kTRUE);
  gGeoManager->SetCurrentDirection(stepSign * stateOrig[3], stepSign * stateOrig[4], stepSign * stateOrig[5]);
  gGeoManager->FindNextBoundary(fabs(sMax) - s);
  double safetyTG = gGeoManager->GetSafeDistance();
  double slDistTG = gGeoManager->GetStep();
  //double stepTG = slDist;

  if (1 || fabs(slDist - slDistTG) > 1e-3) {
    if (fabs(slDist - slDistTG) < 1e-3)
      std::cout << "not";
    std::cout << " \033[0;01mdifferent\033[00m : "
              << stateOrig[0] << " " << stateOrig[1] << " " << stateOrig[2] << " "
              << stepSign* stateOrig[3] << " " << stepSign* stateOrig[4] << " " << stepSign* stateOrig[5] << "\n"
              << " sMax " << sMax
              << " slDist " << slDist << " " << slDistTG
              << " safety " << safety << " " << safetyTG << std::endl
              << " volume = " << currentVolume_->GetLogicalVolume()->GetName() << " "
              << gGeoManager->FindNode(stateOrig[0], stateOrig[1], stateOrig[2])->GetName()
              << std::endl;
  }
#endif
  while (1) {
    if (++it > maxIt) {
      genfit::Exception exc("Geant4MaterialInterface::findNextBoundary ==> maximum number of iterations exceeded", __LINE__, __FILE__);
      exc.setFatal();
      throw exc;
    }

    // No boundary in sight?
    if (s + safety > fabs(sMax)) {
      if (debug)
        std::cout << "   next boundary is farther away than sMax \n";
      nav_->SetGeometricallyLimitedStep();
      return stepSign * (s + safety); // sMax
    }

    // Are we at the boundary?
    if (slDist < delta) {
      if (debug)
        std::cout << "   very close to the boundary -> return"
                  << " stepSign*(s + slDist) = "
                  << stepSign << "*(" << s + slDist << ")\n";
      nav_->SetGeometricallyLimitedStep();
      return stepSign * (s + slDist);
    }

    // We have to find whether there's any boundary on our path.

    // Follow curved arch, then see if we may have missed a boundary.
    // Always propagate complete way from original start to avoid
    // inconsistent extrapolations.
    memcpy(state7, stateOrig, sizeof(state7));
    rep->RKPropagate(state7, NULL, SA, stepSign * (s + step), varField);

    // Straight line distance² between extrapolation finish and
    // the end of the previously determined safe segment.
    double dist2 = (pow(state7[0] - oldState7[0], 2)
                    + pow(state7[1] - oldState7[1], 2)
                    + pow(state7[2] - oldState7[2], 2));
    // Maximal lateral deviation².
    double maxDeviation2 = 0.25 * (step * step - dist2);

    // Do we need to try again with a shorter step?
    //
    // 1. Too much curvature?
    if (step > safety
        && maxDeviation2 > epsilon * epsilon) {
      // Need to take a shorter step to reliably estimate material,
      // but only if we didn't move by safety.  In order to avoid
      // issues with roundoff we check 'step > safety' instead of
      // 'step != safety'.  If we moved by safety, there couldn't have
      // been a boundary that we missed along the path, but we could
      // be on the boundary.

      // Take a shorter step, but never shorter than safety.
      step = std::max(step / 2, safety);

      continue;
    }

    // 2. Volume changed?
    //
    // Where are we after the step?
    nav_->SetGeometricallyLimitedStep();
    bool volChanged = initTrack(state7[0], state7[1], state7[2],
                                stepSign * state7[3], stepSign * state7[4],
                                stepSign * state7[5]);
    if (volChanged) {

      // Extrapolation may not take the exact step length we asked
      // for, so it can happen that a requested step < safety takes
      // us across the boundary.  This is then the best estimate we
      // can get of the distance to the boundary with the stepper.
      if (step <= safety)
        return stepSign * (s + step);

      // Move back to start.
      initTrack(oldState7[0], oldState7[1], oldState7[2],
                stepSign * oldState7[3], stepSign * oldState7[4],
                stepSign * oldState7[5]);

      // Volume changed during the extrapolation.  Take a shorter
      // step, but never shorter than safety.
      step = std::max(0.5 * step, safety);

      continue;
    }

    // We're in the new place, the step was safe, advance.
    s += step;

    memcpy(oldState7, state7, sizeof(state7));
    slDist = nav_->CheckNextStep(G4ThreeVector(state7[0] * CLHEP::cm,
                                               state7[1] * CLHEP::cm,
                                               state7[2] * CLHEP::cm),
                                 G4ThreeVector(stepSign * state7[3],
                                               stepSign * state7[4],
                                               stepSign * state7[5]),
                                 (fabs(sMax) - s) * CLHEP::cm, safety);
    if (slDist == kInfinity)
      slDist = fabs(sMax) - s;
    else
      slDist /= CLHEP::cm;
    safety /= CLHEP::cm;
    step = slDist;

#ifdef LOOK_IN_TGEO
    gGeoManager->IsSameLocation(state7[0], state7[1], state7[2], kTRUE);
    gGeoManager->SetCurrentDirection(stepSign * state7[3], stepSign * state7[4], stepSign * state7[5]);
    gGeoManager->FindNextBoundary(fabs(sMax) - s);
    safetyTG = gGeoManager->GetSafeDistance();
    slDistTG = gGeoManager->GetStep();
    //double stepTG = slDist;

    if (1 || fabs(slDist - slDistTG) > 1e-3) {
      if (fabs(slDist - slDistTG) < 1e-3)
        std::cout << "not";
      std::cout << " \033[0;35mdifferent\033[00m (following): "
                << state7[0] << " " << state7[1] << " " << state7[2] << " "
                << stepSign* state7[3] << " " << stepSign* state7[4] << " " << stepSign* state7[5] << "\n"
                << " sMax-s " << fabs(sMax) - s
                << " slDist " << slDist << " " << slDistTG
                << " safety " << safety << " " << safetyTG << std::endl
                << " volume = " << currentVolume_->GetLogicalVolume()->GetName() << " "
                << gGeoManager->FindNode(state7[0], state7[1], state7[2])->GetName()
                << std::endl;
    }
#endif

  }
}
