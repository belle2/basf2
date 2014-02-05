/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

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
    template<class HitEntity>
    bool operator<(const HitEntity& hitEntity, const CDCWire& wire) { return hitEntity.getWire() < wire; }

    template<class HitEntity>
    bool operator<(const CDCWire& wire, const HitEntity& hitEntity) { return wire < hitEntity.getWire(); }
    /**@}*/

    /** @name Definition of coalignment with wire hits
     *  The following methods define wires and several entities that support the getWireHit method as coaligned.
     *  Which is true for
     *    - CDCRLWireHit
     */
    /**@{*/
    template<class HitEntity>
    bool operator<(const HitEntity& hitEntity, const CDCWireHit& wireHit) { return hitEntity.getWireHit() < wireHit; }

    template<class HitEntity>
    bool operator<(const CDCWireHit& wireHit, const HitEntity& hitEntity) { return wireHit < hitEntity.getWireHit(); }
    /**@}*/






    /// Defines wires and the two dimensional reconstructed hits as coaligned
    bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWire& wire) { return *(recoHit2D.getWire()) < wire; }

    /// Defines wires and the two dimensional reconstructed hits as coaligned
    bool operator<(const CDCWire& wire, const CDCRecoHit2D& recoHit2D) { return wire < *(recoHit2D.getWire()); }

    /// Defines wire hits and the two dimensional reconstructed hits as coaligned
    bool operator<(const CDCRecoHit2D& recoHit2D, const CDCWireHit& wireHit) { return *(recoHit2D.getWireHit()) < wireHit; }

    /// Defines wire hits and the two dimensional reconstructed hits as coaligned
    bool operator<(const CDCWireHit& wireHit, const CDCRecoHit2D& recoHit2D) { return wireHit < *(recoHit2D.getWireHit()); }





    /// Defines wires and the three dimensional reconstructed hits as coaligned
    bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWire& wire) { return *(recoHit3D.getWire()) < wire; }

    /// Defines wires and the three dimensional reconstructed hits as coaligned
    bool operator<(const CDCWire& wire, const CDCRecoHit3D& recoHit3D) { return wire < *(recoHit3D.getWire()); }

    /// Defines wire hits and the three dimensional reconstructed hits as coaligned
    bool operator<(const CDCRecoHit3D& recoHit3D, const CDCWireHit& wireHit) { return *(recoHit3D.getWireHit()) < wireHit; }

    /// Defines wire hits and the three dimensional reconstructed hits as coaligned
    bool operator<(const CDCWireHit& wireHit, const CDCRecoHit3D& recoHit3D) { return wireHit < *(recoHit3D.getWireHit()); }





    /// Defines wire hits and reconstructed tangents as coaligned on the first wire
    bool operator<(const CDCRecoTangent& recoTangent, const CDCWireHit& wireHit) { return *(recoTangent.getFromWireHit()) < wireHit; }

    /// Defines wire hits and reconstructed tangents as coaligned on the first wire
    bool operator<(const CDCWireHit& wireHit, const CDCRecoTangent& recoTangent) { return  wireHit <  *(recoTangent.getFromWireHit()); }





    /// Defines wire hits and facet as coaligned on the first wire hit
    bool operator<(const CDCRecoFacet& recoFacet, const CDCWireHit& wireHit) { return *(recoFacet.getStartWireHit()) < wireHit; }

    /// Defines wire hits and facet as coaligned on the first wire hit
    bool operator<(const CDCWireHit& wireHit, const CDCRecoFacet& recoFacet) { return  wireHit <  *(recoFacet.getStartWireHit()); }

    /// Defines reconstructed tangents and facet as coaligned on the start to middle tangent
    //bool operator<(const CDCRecoFacet& recoFacet, const CDCRecoTangent& recoTangent)
    //{ return *(recoFacet.getStartWireHit()) < recoTangent; }

    /// Defines reconstructed tangents and facet as coaligned on the start to middle tangent
    //bool operator<(const CDCWireHit& recoTangent, const CDCRecoFacet& recoFacet) { return  recoTangent < *(recoFacet.getStartWireHit()); }



  } //end namespace CDCLocalTracking
} //end namespace Belle2
