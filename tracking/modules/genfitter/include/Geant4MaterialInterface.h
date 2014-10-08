/* Copyright 2014, Ludwig-Maximilians-Universität München,
   Authors: Tobias Schlüter

   Provided as part of the Belle II software framework basf2.  Its
   licenses apply.
*/

#pragma once

#include <memory>

#include "G4VPhysicalVolume.hh"
#include "G4Navigator.hh"

#include "genfit/AbsMaterialInterface.h"
#include "genfit/RKTrackRep.h"

namespace Belle2 {


  class G4SafeNavigator : public G4Navigator {
    // Guards against leaving the physical volume.
  public:
    G4SafeNavigator() : dX_(0), dY_(0), dZ_(0) {}
    virtual ~G4SafeNavigator() {}

    void SetWorldVolume(G4VPhysicalVolume* pWorld);

    virtual
    G4VPhysicalVolume* LocateGlobalPointAndSetup(const G4ThreeVector& point,
                                                 const G4ThreeVector* direction = 0,
                                                 const G4bool pRelativeSearch = true,
                                                 const G4bool ignoreDirection = true);

    virtual G4double ComputeStep(const G4ThreeVector& pGlobalPoint,
                                 const G4ThreeVector& pDirection,
                                 const G4double pCurrentProposedStepLength,
                                 G4double&  pNewSafety);

    G4double CheckNextStep(const G4ThreeVector& pGlobalPoint,
                           const G4ThreeVector& pDirection,
                           const G4double pCurrentProposedStepLength,
                           G4double& pNewSafety);
  private:
    // Half-sizes of the world volume, assumed to be a G4Box;
    double dX_, dY_, dZ_;
  };



  /**
   * @brief AbsMaterialInterface implementation for use with ROOT's TGeoManager.
   */
  class Geant4MaterialInterface : public genfit::AbsMaterialInterface {

  public:

    Geant4MaterialInterface();
    ~Geant4MaterialInterface() {};

    /** @brief Initialize the navigator at given position and with given
        direction.  Returns true if the volume changed.
     */
    bool initTrack(double posX, double posY, double posZ,
                   double dirX, double dirY, double dirZ);

    /** @brief Get material parameters in current material
     */
    void getMaterialParameters(double& density,
                               double& Z,
                               double& A,
                               double& radiationLength,
                               double& mEE);

    void getMaterialParameters(genfit::MaterialProperties& parameters);

    /** @brief Make a step (following the curvature) until step length
     * sMax or the next boundary is reached.  After making a step to a
     * boundary, the position has to be beyond the boundary, i.e. the
     * current material has to be that beyond the boundary.  The actual
     * step made is returned.
     */
    double findNextBoundary(const genfit::RKTrackRep* rep,
                            const genfit::M1x7& state7,
                            double sMax,
                            bool varField = true);

    double findNextBoundaryAndStepStraight(double sMax);

  private:

    std::unique_ptr<G4SafeNavigator> nav_;
    const G4VPhysicalVolume* currentVolume_;
  };

}
