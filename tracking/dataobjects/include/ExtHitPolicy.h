/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: genfit/GFSpacepointHitPolicy                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTHITPOLICY_H
#define EXTHITPOLICY_H

#include <TMatrixD.h>
#include <TObject.h>

#include <GFDetPlane.h>

class GFAbsRecoHit;
class GFAbsTrackRep;

namespace Belle2 {

  /** @brief Policy class implementing a phasespace point hit geometry.
   *
   * For a phasespace point the detector plane is defined by the direction vector.
   */

  class ExtHitPolicy : public TObject {
  public:

    ExtHitPolicy() {}

    // Accessors ----------------------
    /** @brief Get detector plane.  The user's track is ignored.
     *
     * The detector plane is defined by the position and direction of the
     * phasespace hit.
     */
    const GFDetPlane& detPlane(GFAbsRecoHit*, const GFAbsTrackRep*);

    // Modifiers ----------------------
    /** @brief Set detector plane using the phasespace point.
     *
     * The detector plane is defined by the position and direction of the hit.
     * This method must be called in the constructor of the derived RecoHit.
     */
    void setDetPlane(const TMatrixD&);

    // Operations ----------------------
    /** @brief Hit coordinates in detector plane.
     */
    TMatrixD hitCoord(GFAbsRecoHit*, const GFDetPlane&);

    /** @brief Hit covariances in detector plane.
     */
    TMatrixD hitCov(GFAbsRecoHit*, const GFDetPlane&);

    virtual ~ExtHitPolicy() {}

    const std::string& getName() {return fPolicyName;}
  private:
    static const std::string fPolicyName;

    // Private Data Members ------------
    GFDetPlane fPlane;

    // Private Methods -----------------

    ClassDef(ExtHitPolicy, 1)
  };

} // end of namespace Belle2

#endif
