/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCGENHIT_H
#define CDCGENHIT_H

#include <TVector2.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/translators/SimpleTDCCountTranslator.h>

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

#include <tracking/cdcLocalTracking/topology/CDCWire.h>

#include <tracking/cdcLocalTracking/eventdata/trajectories/CDCTrajectory2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /** This is a mock class, which defines the interface expected by the CDCGenHitVector.

        It is not the meant as a base class yet but rather as a guidline and for testing
        purposes being a drop in replacement for a concrete hit type.
    */

    class CDCGenHit : public UsedTObject {

    public:
      /// Default constructor for ROOT compatibility.
      CDCGenHit();

      /// Empty deconstructor
      ~CDCGenHit();

    private:
      int m_i;

      /// ROOT Macro to make CDCGenHit a ROOT class.
      ClassDefInCDCLocalTracking(CDCGenHit, 1);

    }; //end class GenHit
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //CDCGENHIT_H

