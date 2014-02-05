/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOSEGMENT2D_H_
#define CDCRECOSEGMENT2D_H_

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include <tracking/cdcLocalTracking/eventdata/collections/CDCRecoHit2DVector.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// A segment consisting of two dimensional reconsturcted hits
    class CDCRecoSegment2D : public CDCRecoHit2DVector {
    public:

      /// Default constructor for ROOT compatibility.
      CDCRecoSegment2D() {;}

      /// Empty deconstructor
      ~CDCRecoSegment2D() {;}

      /// Getter for the automaton cell.
      AutomatonCell& getAutomatonCell() const { return m_automatonCell; }

    private:
      mutable AutomatonCell m_automatonCell; ///< Memory for the automaton cell. It is declared mutable because it can vary rather freely despite of the hit content might be required fixed

    private:
      /// ROOT Macro to make CDCRecoSegment2D a ROOT class.
      ClassDefInCDCLocalTracking(CDCRecoSegment2D, 1);



    }; //end class CDCRecoSegment2D

  } // end namespace CDCLocalTracking
} // end namespace Belle2
#endif // CDCRECOSEGMENT2D_H_
