/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCGENHITVECTOR_H
#define CDCGENHITVECTOR_H

#include <vector>
#include <TObject.h>

//#include <algorithm>

//#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
//#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

//#include <tracking/cdcLocalTracking/algorithms/AutomatonCell.h>

//#include <tracking/cdcLocalTracking/topology/CDCWire.h>

#ifdef __CINT__
// Because ROOTCINT does not like namespaces inside template parameters
// we have to make each object, which we want the instantiate the template
// available outside any namespace. Therefore we also have to include each of
// them as well.

#include <tracking/cdcLocalTracking/eventdata/entities/CDCGenHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCWireHit.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit2D.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoTangent.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoHit3D.h>

typedef Belle2::CDCLocalTracking::CDCGenHit CDCGenHit;
typedef Belle2::CDCLocalTracking::CDCWireHit CDCWireHit;
typedef Belle2::CDCLocalTracking::CDCRecoHit2D CDCRecoHit2D;
typedef Belle2::CDCLocalTracking::CDCRecoTangent CDCRecoTangent;
typedef Belle2::CDCLocalTracking::CDCRecoFacet CDCRecoFacet;
typedef Belle2::CDCLocalTracking::CDCRecoHit3D CDCRecoHit3D;

#endif // __CINT__


namespace Belle2 {
  namespace CDCLocalTracking {

    /// A generic vector used for different hit aggregates.
    /** details */

    template<class T>
    class CDCGenHitVector : public TObject {
    public:

      /// Default constructor for ROOT compatibility.
      CDCGenHitVector() {;}

      /// Empty deconstructor
      ~CDCGenHitVector() {;}











    private:
      std::vector<T> m_items;

    private:
      /// ROOT Macro to make CDCGenHitVector a ROOT class.
      ClassDef(CDCGenHitVector, 1);

    }; //class

  } // namespace CDCLocalTracking
} // namespace Belle2

#endif // CDCGENHITVECTOR_H
