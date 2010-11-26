/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCRECOHIT_H
#define CDCRECOHIT_H

#include <framework/datastore/Units.h>
#include <cdc/geocdc/CDCGeometryPar.h>

#include <TVector3.h>
#include <TMatrixD.h>

//base class headers
#include <cdc/dataobjects/CDCHit.h>

#include <genfit/GFRecoHitIfc.h>
#include <genfit/GFWireHitPolicy.h>

// other GenFit headers
#include <genfit/RKTrackRep.h>
#include <genfit/GFDetPlane.h>


namespace Belle2 {

  /** This class is an enriched form of the CDCHit class.
   *
   *  It includes all information of CDCHit, plus it includes additional information
   *  from geometry, calibration, GENFIT, etc.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=DataStore">Martin Heck</a>
   *
   *  @todo So far this class is mostly a stumb...
   */
  class CDCRecoHit : public GFRecoHitIfc<GFWireHitPolicy> {
  public:

    /** Default Constructor for ROOT IO.*/
    CDCRecoHit();

    /** Constructor, that does the enrichment.
     *
     *  @param cdcHit original CDCHit from the enriched hit will be constructed.
     */
    CDCRecoHit(const CDCHit& cdcHit);

    /** Destructor. */
    ~CDCRecoHit() {}

    /** Creating a copy of this hit.
     *
     * This function overwrites a funtion that GFRecoHitIfc inherits from GFRecoHit.
     */
    GFAbsRecoHit* clone();

    /** Projection for the hit ...
     *
     * This function overwrites a funtion that GFRecoHitIfc inherits from GFRecoHit.
     */
    TMatrixD getHMatrix();


  private:

    static const int m_nParHitRep = 7;

    /** ROOT Macro.*/
    ClassDef(CDCRecoHit, 1);
  };

}
#endif
