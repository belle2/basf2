/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen, Moritz Nadler                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTRECOHIT_H
#define EXTRECOHIT_H

#include <TObject.h>
#include <TVector3.h>
#include <TMatrixD.h>

#include <RecoHits/GFAbsRecoHit.h>

// for definition of ExtHitStatus
#include <tracking/dataobjects/ExtHit.h>

namespace Belle2 {

  /** This class defines the extrapolation RecoHit.
   *
   *  It contains the phasespace point and covariance matrix of an intersection between
   *  the extrapolated track and a GEANT4 volume's surface.
   */
  class ExtRecoHit : public GFAbsRecoHit {

  public:

    /** Default Constructor for ROOT IO.*/
    ExtRecoHit();

    /** Constructor needed for basf2 extrapolation module
     *
     */
    ExtRecoHit(const TMatrixD&, const TMatrixD&, ExtHitStatus);

    /** Destructor. */
    virtual ~ExtRecoHit() {}

    /** Creating a copy of this hit.
     *
     * This function overwrites a function that GFRecoHitIfc inherits from GFAbsRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     *
     * This function overwrites a function that GFRecoHitIfc inherits from GFAbsRecoHit.
     */
    const TMatrixD& getHMatrix(const GFAbsTrackRep*);

    // getMeasurement:  dummy here, not to be used in ext
    void getMeasurement(const GFAbsTrackRep*,
                        const GFDetPlane&,
                        const TVectorD&,
                        const TMatrixDSym&,
                        TVectorD&,
                        TMatrixDSym&) {}

    // getDetPlane:  always returns fDetPlane in ext, regardless of track representation
    const GFDetPlane& getDetPlane(GFAbsTrackRep*) {
      return fDetPlane;
    }

    ExtHitStatus getStatus() { return m_status; }

  private:

    /** Number of elements in the phasespace point
     */
    enum {HIT_DIMENSIONS = 6};
    const static TMatrixD c_hMatrix;
    /** Status code for the hit
     */
    ExtHitStatus m_status;

    /** ROOT Macro.*/
    ClassDef(ExtRecoHit, 1)
  };

}
#endif
