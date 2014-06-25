/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FACETFILTERTREE_H_
#define FACETFILTERTREE_H_

#include <tracking/cdcLocalTracking/tempro/StaticTypedTree.h>
#include <tracking/cdcLocalTracking/tempro/BranchOf.h>
#include <tracking/cdcLocalTracking/tempro/Named.h>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Tree to store relevant variables to filter out wrong facets and the true information form Monte Carlo.
    typedef StaticTypedTree <
    //BranchOf < Float_t, NAMED("mcWeight") > ,
    //BranchOf < Float_t, NAMED("prWeight") > ,

    BranchOf < Bool_t, NAMED("mcDecision") > ,
             BranchOf < Bool_t, NAMED("prDecision") > ,

             BranchOf < Float_t, NAMED("smToSECosine") > ,
             BranchOf < Float_t, NAMED("seToMECosine") > ,
             BranchOf < Float_t, NAMED("smToMECosine") >

             > FacetFilterTreeBase;

    /// Tree wrapper to help extraction of the relevant variables from the facets and filling of the underlying tree.
    class FacetFilterTree : public FacetFilterTreeBase {

    public:
      /// Empty constructor.
      FacetFilterTree();

      /// Empty deconstructor.
      ~FacetFilterTree();

    public:
      /// Fills the object with the information gather from the given segment to segment pair instance.
      bool setValues(const CellWeight& mcWeight, const CellWeight& prWeight, const CDCRecoFacet& facet);

    }; // end class

  } // end namespace CDCLocalTracking
} // end namespace Belle2


#endif // FACETFILTERTREE_H_
