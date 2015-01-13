/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCENTITIESCOMPARE_H
#define CDCENTITIESCOMPARE_H

#include "CDCGenHit.h"

#include "CDCWireHit.h"
#include "CDCRLWireHit.h"
#include "CDCRecoHit2D.h"
#include "CDCRecoTangent.h"
#include "CDCRecoFacet.h"

#include "CDCRecoHit3D.h"



namespace Belle2 {
  namespace CDCLocalTracking {

    /** @name Definition of coalignment with wires
     *  The following methods define wires and several entities that support the getWire method as coaligned.
     *  Which is true for
     *    - CDCGenHit
     *    - CDCWireHit
     *    - CDCRLWireHit
     */
    /**@{*/
    /* template<class HitEntity> */
    /* bool operator<(const HitEntity& hitEntity, const CDCWire& wire) { return hitEntity.getWire() < wire; } */

    /* template<class HitEntity> */
    /* bool operator<(const CDCWire& wire, const HitEntity& hitEntity) { return wire < hitEntity.getWire(); } */
    /* /\**@}*\/ */

    /* /\** @name Definition of coalignment with wire hits */
    /*  *  The following methods define wires and several entities that support the getWireHit method as coaligned. */
    /*  *  Which is true for */
    /*  *    - CDCRLWireHit */
    /*  *\/ */
    /* /\**@{*\/ */
    /* template<class HitEntity> */
    /* bool operator<(const HitEntity& hitEntity, const CDCWireHit& wireHit) { return hitEntity.getWireHit() < wireHit; } */

    /* template<class HitEntity> */
    /* bool operator<(const CDCWireHit& wireHit, const HitEntity& hitEntity) { return wireHit < hitEntity.getWireHit(); } */
    /**@}*/

    /// Defines reconstructed tangents and facet as coaligned on the start to middle tangent
    /*bool operator<(const CDCRecoFacet& recoFacet, const CDCRecoTangent& recoTangent) {
      return recoFacet.getStartRLWireHit() < recoTangent.getFromRLWireHit() or
    ( recoFacet.getStartRLWireHit() == recoTangent.getFromRLWireHit() and
    recoFacet.getMiddleRLWireHit() < recoTangent.getToRLWireHit() );
    }

    /// Defines reconstructed tangents and facet as coaligned on the start to middle tangent
    bool operator<(const CDCWireHit& recoTangent, const CDCRecoFacet& recoFacet){
      return recoTangent.getFromRLWireHit() < recoFacet.getStartRLWireHit() or
    ( recoTangent.getFromRLWireHit() == recoFacet.getStartRLWireHit() and
    recoTangent.getToRLWireHit() < recoFacet.getMiddleRLWireHit() );
    }
    */


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CDCENTITIESCOMPARE_H
