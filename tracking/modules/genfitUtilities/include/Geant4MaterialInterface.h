/* Copyright 2014, Ludwig-Maximilians-Universität München,
   Authors: Tobias Schlüter, Thomas Hauth

   Provided as part of the Belle II software framework basf2.  Its
   licenses apply.
*/

#pragma once

#include <memory>

#include "genfit/AbsMaterialInterface.h"

class G4VPhysicalVolume;

namespace Belle2 {

  /**
   * @brief AbsMaterialInterface implementation for use with Geant4's navigator.
   *
   * This allows to look up the material properties from the Geant4 geometry also used for
   * simulation purposes.
   */
  class Geant4MaterialInterface : public genfit::AbsMaterialInterface {

  public:

    Geant4MaterialInterface();
    ~Geant4MaterialInterface();

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

    /** get the material parameters as the method above, but fill directly
     * into a genfit::MaterialProperties class
     */
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

  private:

    /** holds a object of G4SafeNavigator, which is located in Geant4MaterialInterface.cc */
    std::unique_ptr<class G4SafeNavigator> nav_;

    /** the volume the extraoplation is currenly located in*/
    const class G4VPhysicalVolume* currentVolume_;

    /** stores whether to call SetGeometricallyLimitedStep() because the full step
     * length was taken.
     */
    bool m_takingFullStep = false;
  };

}
