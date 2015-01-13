/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FACETNEIGHBORCHOOSERTREE_H_
#define FACETNEIGHBORCHOOSERTREE_H_

#include <tracking/trackFindingCDC/tempro/StaticTypedTree.h>
#include <tracking/trackFindingCDC/tempro/BranchOf.h>
#include <tracking/trackFindingCDC/tempro/Named.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Tree to store relevant variables to filter out wrong facets and the true information form Monte Carlo.
    typedef StaticTypedTree <
    //BranchOf < Float_t, NAMED("mcWeight") > ,
    //BranchOf < Float_t, NAMED("prWeight") > ,

    BranchOf < Bool_t, NAMED("mcDecision") > ,
             BranchOf < Bool_t, NAMED("prDecision") > ,

             BranchOf < Float_t, NAMED("smToNeighborSECosine") > ,
             BranchOf < Float_t, NAMED("seToNeighborMECosine") >
             > FacetNeighborChooserTreeBase;


    /// Tree wrapper to help extraction of the relevant variables from the facets and filling of the underlying tree.
    class FacetNeighborChooserTree : public FacetNeighborChooserTreeBase {

    public:
      /// Empty constructor.
      FacetNeighborChooserTree();

      /// Empty deconstructor.
      ~FacetNeighborChooserTree();

    public:
      /// Fills the object with the information gather from the given segment to segment pair instance.
      bool setValues(const CellWeight& mcWeight, const CellWeight& prWeight, const CDCRecoFacet& facet, const CDCRecoFacet& neighborFacet);

    }; // end class

  } // end namespace TrackFindingCDC
} // end namespace Belle2


#endif // FACETNEIGHBORCHOOSERTREE_H_
