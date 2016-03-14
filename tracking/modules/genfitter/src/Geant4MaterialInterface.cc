#include <framework/logging/Logger.h>
#include <tracking/modules/genfitter/Geant4MaterialInterface.h>
#include <geometry/GeometryManager.h>

#include "genfit/Exception.h"
#include "genfit/RKTrackRep.h"

#include <assert.h>
#include <math.h>

#include "G4ThreeVector.hh"
#include "G4Navigator.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Material.hh"
#include "G4TouchableHistory.hh"

static const bool debug = false;
//static const bool debug = true;

using namespace Belle2;

namespace Belle2 {
  class G4SafeNavigator {
    // Guards against leaving the physical volume.
    //
    // Not inheriting from G4Navigator because CheckNextStep is not
    // virtual.
  public:
    G4SafeNavigator() : dX_(0), dY_(0), dZ_(0) {}
    ~G4SafeNavigator() {}

    void SetWorldVolume(G4VPhysicalVolume* pWorld);
    G4VPhysicalVolume* LocateGlobalPointAndSetup(const G4ThreeVector& point,
                                                 const G4ThreeVector* direction = 0,
                                                 const G4bool pRelativeSearch = true,
                                                 const G4bool ignoreDirection = true);

    G4double CheckNextStep(const G4ThreeVector& pGlobalPoint,
                           const G4ThreeVector& pDirection,
                           const G4double pCurrentProposedStepLength,
                           G4double& pNewSafety);

    G4VPhysicalVolume* ResetHierarchyAndLocate(const G4ThreeVector& point,
                                               const G4ThreeVector& direction,
                                               const G4TouchableHistory& h);
    G4TouchableHistory* CreateTouchableHistory() const
    {
      return nav_.CreateTouchableHistory();
    }
  private:
    void limitToWorld(G4ThreeVector& p, G4ThreeVector& d) const;

    // Half-sizes of the world volume, assumed to be a G4Box;
    double dX_, dY_, dZ_;
    G4Navigator nav_;
  };
}

void
G4SafeNavigator::limitToWorld(G4ThreeVector& p, G4ThreeVector& d) const
{
  if (fabs(p.x()) >= dX_)
    p.setX(0);
  if (fabs(p.y()) >= dY_)
    p.setY(0);
  if (fabs(p.z()) >= dZ_)
    p.setZ(0);

  if (std::isnan(d.x()) || std::isnan(d.y()) || std::isnan(d.z())) {
    B2WARNING("nan in direction in G4SafeNavigator::limitToWorld");
    d.set(1., 0., 0.);
  } else {
    d.setMag(1.);
  }
}

void G4SafeNavigator::SetWorldVolume(G4VPhysicalVolume* pWorld)
{
  G4Box* box = dynamic_cast<G4Box*>(pWorld->GetLogicalVolume()->GetSolid());
  assert(box && "World volume a G4Box?");
  dX_ = box->GetXHalfLength();
  dY_ = box->GetYHalfLength();
  dZ_ = box->GetZHalfLength();
  nav_.SetWorldVolume(pWorld);
}


G4VPhysicalVolume* G4SafeNavigator::LocateGlobalPointAndSetup(const G4ThreeVector& point,
    const G4ThreeVector* direction,
    const G4bool pRelativeSearch,
    const G4bool ignoreDirection)
{
  G4ThreeVector p(point);
  assert(direction);
  G4ThreeVector dir(*direction);
  limitToWorld(p, dir);
  return nav_.LocateGlobalPointAndSetup(p, &dir, pRelativeSearch, ignoreDirection);
}

G4VPhysicalVolume* G4SafeNavigator::ResetHierarchyAndLocate(const G4ThreeVector& point,
                                                            const G4ThreeVector& direction,
                                                            const G4TouchableHistory& h)
{
  G4ThreeVector p(point);
  G4ThreeVector dir(direction);
  limitToWorld(p, dir);
  return nav_.ResetHierarchyAndLocate(p, dir, h);
}

G4double G4SafeNavigator::CheckNextStep(const G4ThreeVector& pGlobalPoint,
                                        const G4ThreeVector& pDirection,
                                        const G4double pCurrentProposedStepLength,
                                        G4double& pNewSafety)
{
  G4ThreeVector p(pGlobalPoint);
  G4ThreeVector dir(pDirection);
  limitToWorld(p, dir);
  return nav_.CheckNextStep(p, dir, pCurrentProposedStepLength, pNewSafety);
}


Geant4MaterialInterface::Geant4MaterialInterface()
  : nav_(new G4SafeNavigator()), currentVolume_(0)
{
  G4VPhysicalVolume* world = geometry::GeometryManager::getInstance().getTopVolume();
  nav_->SetWorldVolume(world);
}

Geant4MaterialInterface::~Geant4MaterialInterface()
{
}


bool
Geant4MaterialInterface::initTrack(double posX, double posY, double posZ,
                                   double dirX, double dirY, double dirZ)
{
  G4ThreeVector pos(posX * CLHEP::cm, posY * CLHEP::cm, posZ * CLHEP::cm);
  G4ThreeVector dir(dirX, dirY, dirZ);
  const G4VPhysicalVolume* newVolume = nav_->LocateGlobalPointAndSetup(pos, &dir, false, false);
  if (!newVolume) {
    genfit::Exception exc("Geant4MaterialInterface: newVolume == 0", __LINE__, __FILE__);
    exc.setFatal();
    throw exc;
  }

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
  // Z has correct units
  A *= CLHEP::mole / CLHEP::g;
  radiationLength = mat->GetRadlen() / CLHEP::cm;
  mEE = mat->GetIonisation()->GetMeanExcitationEnergy() / CLHEP::eV;
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
  // This assumes that sMax is small enough to take only a single RK
  // step.  This restriction comes about because RKPropagate only
  // takes one step.
  const double delta(1.E-2); // cm, distance limit beneath which straight-line steps are taken.
  const double epsilon(1.E-1); // cm, allowed upper bound on arch deviation from straight line

  genfit::M1x3 SA;
  genfit::M1x7 state7, oldState7;
  oldState7 = stateOrig;

  int stepSign(sMax < 0 ? -1 : 1);

  G4ThreeVector pointOld(stateOrig[0] * CLHEP::cm,
                         stateOrig[1] * CLHEP::cm,
                         stateOrig[2] * CLHEP::cm);
  G4ThreeVector dirOld(stepSign * stateOrig[3],
                       stepSign * stateOrig[4],
                       stepSign * stateOrig[5]);

  double s = 0;  // trajectory length to boundary

  const unsigned maxIt = 300;
  unsigned it = 0;

  // Initialize the geometry to the current location (set by caller).
  double safety;
  double slDist = nav_->CheckNextStep(pointOld, dirOld, fabs(sMax) * CLHEP::cm, safety);
  if (slDist == kInfinity)
    slDist = fabs(sMax);
  else
    slDist /= CLHEP::cm;
  safety /= CLHEP::cm;

  // No boundary in sight?
  if (safety > fabs(sMax)) {
    if (debug)
      std::cout << "   next boundary is farther away than sMax \n";
    return stepSign * safety; // sMax
  }

  // Are we at the boundary?
  if (slDist < delta) {
    if (debug)
      std::cout << "   very close to the boundary -> return @ it " << it
                << " stepSign*slDist = "
                << stepSign << "*" << slDist << "\n";
    return stepSign * slDist;
  }
  double step = slDist;

  while (1) {
    if (++it > maxIt) {
      genfit::Exception exc("Geant4MaterialInterface::findNextBoundary ==> maximum number of iterations exceeded", __LINE__, __FILE__);
      exc.setFatal();
      throw exc;
    }

    if (step < delta)
      return stepSign * (s + step);

    // We have to find whether there's any boundary on our path.

    // Follow curved arch, then see if we may have missed a boundary.
    // Always propagate complete way from original start to avoid
    // inconsistent extrapolations.  This is always a single RK step.
    state7 = stateOrig;
    rep->RKPropagate(state7, NULL, SA, stepSign * (s + step), varField);

    G4ThreeVector pos(state7[0] * CLHEP::cm, state7[1] * CLHEP::cm, state7[2] * CLHEP::cm);
    G4ThreeVector dir(stepSign * state7[3], stepSign * state7[4], stepSign * state7[5]);

    // Straight line distance between extrapolation finish and
    // the end of the previously determined safe segment.
    double dist2 = (pow(state7[0] - oldState7[0], 2)
                    + pow(state7[1] - oldState7[1], 2)
                    + pow(state7[2] - oldState7[2], 2));

    // If we moved less than safety, the volume cannot possibly have
    // changed, so we skip further checks.
    if (dist2 > safety * safety) {

      // Do we need to try again with a shorter step?  There are two
      // possible reasons:
      //
      // 1. Too much curvature?

      // Maximal lateral deviation² of the curved path from the
      // straight line connecting beginning and end.
      double maxDeviation2 = 0.25 * (step * step - dist2);
      if (maxDeviation2 > epsilon * epsilon) {
        // Need to take a shorter step to reliably estimate material,
        // but only if we didn't move by safety.

        // Take a shorter step, but never shorter than safety.
        step = safety + 0.5 * (step - safety);

        continue;
      }

      // 2. Volume changed?
      //
      // Where are we after the step?
      std::unique_ptr<G4TouchableHistory> hist(nav_->CreateTouchableHistory());
      G4VPhysicalVolume* newVolume = nav_->LocateGlobalPointAndSetup(pos, &dir);

      if (newVolume != currentVolume_) {

        // Volume changed during the extrapolation.

        // Extrapolation may not take the exact step length we asked
        // for, so it can happen that a requested step < safety takes
        // us across the boundary.  This is then the best estimate we
        // can get of the distance to the boundary with the stepper.
        if (step <= safety)
          return stepSign * (s + step);

        // Move back to last good point, but looking in the actual
        // direction of the step.
        G4ThreeVector dirCloser(pos - pointOld);
        dirCloser.setMag(1.);
        nav_->ResetHierarchyAndLocate(pointOld, dirCloser, *hist);

        // Look along the secant of the actual trajectory instead of
        // the original direction.  There should be a crossing within
        // distance step.
        double secantDist = nav_->CheckNextStep(pointOld, dirCloser,
                                                step * CLHEP::cm, safety) / CLHEP::cm;
        safety /= CLHEP::cm;
        if (secantDist >= step) {
          // Cannot be.  Just take a shorter step, and hope that this
          // works.
          slDist = secantDist;
          step = std::max(0.9 * step, safety);
        } else {
          slDist = step = std::max(secantDist, safety);
        }

        // Are we at the boundary?
        if (slDist < delta) {
          if (debug)
            std::cout << "   very close to the boundary -> return @ it " << it
                      << " stepSign*(s + slDist) = "
                      << stepSign << "*(" << s + slDist << ")\n";
          return stepSign * (s + slDist);
        }

        continue;
      }
    }

    // We're in the new place, the step was safe, advance.
    s += step;

    oldState7 = state7;
    pointOld = pos;
    slDist = nav_->CheckNextStep(pos, dir,
                                 (fabs(sMax) - s) * CLHEP::cm, safety);
    if (slDist == kInfinity)
      slDist = fabs(sMax) - s;
    else
      slDist /= CLHEP::cm;
    safety /= CLHEP::cm;
    step = slDist;

    // No boundary in sight?
    if (s + safety > fabs(sMax)) {
      if (debug)
        std::cout << "   next boundary is farther away than sMax \n";
      return stepSign * (s + safety); // sMax
    }

    // Are we at the boundary?
    if (slDist < delta) {
      if (debug)
        std::cout << "   very close to the boundary -> return @ it " << it
                  << " stepSign*(s + slDist) = "
                  << stepSign << "*(" << s + slDist << ")\n";
      return stepSign * (s + slDist);
    }
  }
}
