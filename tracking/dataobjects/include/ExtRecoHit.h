/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EXTRECOHIT_H
#define EXTRECOHIT_H

#include <TObject.h>
#include <TVector3.h>
#include <TMatrixD.h>

#include <GFRecoHitIfc.h>
#include <tracking/dataobjects/ExtHitPolicy.h>

class GFAbsRecoHit;

namespace Belle2 {

  enum ExtHitStatus { ENTER, EXIT, STOP, ESCAPE };

  /** This class defines the extrapolation RecoHit.
   *
   *  It contains the phasespace point and covariance matrix of an intersection between
   *  the extrapolated track and a GEANT4 volume's surface.
   */
  class ExtRecoHit : public GFRecoHitIfc<ExtHitPolicy> {

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
    TMatrixD getHMatrix(const GFAbsTrackRep*);

    ExtHitStatus getStatus() { return m_status; }

  private:

    /** Number of elements in the phasespace point
     */
    enum {HIT_DIMENSIONS = 6};

    /** Status code for the hit
     */
    ExtHitStatus m_status;

    /** ROOT Macro.*/
    ClassDef(ExtRecoHit, 1)
  };

}
#endif
