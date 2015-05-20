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
#ifndef CDCWIREHITCLUSTER_H
#define CDCWIREHITCLUSTER_H


#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCGenHitVector.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// An aggregation of CDCWireHits.
    class CDCWireHitCluster : public CDCGenHitVector<const Belle2::TrackFindingCDC::CDCWireHit*> {
    public:

      /// Default constructor for ROOT compatibility.
      CDCWireHitCluster() {;}

      /// Empty deconstructor
      ~CDCWireHitCluster() {;}

    private:
      /// ROOT Macro to make CDCWireHitCluster a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCWireHitCluster, 1);



    }; //end class CDCWireHitCluster

  }
}


#endif //CDCWIREHITCLUSTER_H



