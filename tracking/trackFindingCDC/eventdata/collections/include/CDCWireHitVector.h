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
#ifndef CDCWIREHITVECTOR_H
#define CDCWIREHITVECTOR_H

#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A vector of wire hits. Template instance of CDCGenHitVector<CDCWireHit>
    /** See CDCGenHitVector for all methods and details */
    class CDCWireHitVector : public CDCGenHitVector<Belle2::TrackFindingCDC::CDCWireHit> {

    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitVector() {;}

      /// Empty deconstructor
      ~CDCWireHitVector() {;}

    private:
      /// ROOT Macro to make CDCWireHitVector a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCWireHitVector, 1);

    }; //class


  } // namespace TrackFindingCDC
} // namespace Belle2

#endif // CDCWIREHITVECTOR_H
