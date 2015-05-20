/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef CDCENTITIESCOMPARE_H
#define CDCENTITIESCOMPARE_H

#include <tracking/trackFindingCDC/eventdata/entities/CDCGenHit.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit2D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCTangent.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>



namespace Belle2 {
  namespace TrackFindingCDC {

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

    /// Defines tangents and facet as coaligned on the start to middle tangent
    /*bool operator<(const CDCFacet& facet, const CDCTangent& tangent) {
      return facet.getStartRLWireHit() < tangent.getFromRLWireHit() or
    ( facet.getStartRLWireHit() == tangent.getFromRLWireHit() and
    facet.getMiddleRLWireHit() < tangent.getToRLWireHit() );
    }

    /// Defines tangents and facet as coaligned on the start to middle tangent
    bool operator<(const CDCWireHit& tangent, const CDCFacet& facet){
      return tangent.getFromRLWireHit() < facet.getStartRLWireHit() or
    ( tangent.getFromRLWireHit() == facet.getStartRLWireHit() and
    tangent.getToRLWireHit() < facet.getMiddleRLWireHit() );
    }
    */


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //CDCENTITIESCOMPARE_H
