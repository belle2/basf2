/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVALUATEFACETNEIGHBORCHOOSER_H_
#define EVALUATEFACETNEIGHBORCHOOSER_H_

#include <iostream>
#include <fstream>

#include "BaseFacetNeighborChooser.h"
#include "MCFacetNeighborChooser.h"

#include "FacetNeighborChooserTree.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class filtering the neighborhood of facets based on simple criterions.
    template<class RealFacetNeighborChooser>
    class EvaluateFacetNeighborChooser : public BaseFacetNeighborChooser {

    public:
      /// Empty constructor
      EvaluateFacetNeighborChooser();

      /// Empty deconstructor
      ~EvaluateFacetNeighborChooser();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      NeighborWeight isGoodNeighbor(const CDCRecoFacet& facet, const CDCRecoFacet& neighborFacet);

      /// Forwards the modules initialize to the filter.
      void terminate();

      /// Getter for the filter to be evaluted.
      RealFacetNeighborChooser& getRealFacetNeighborChooser()
      { return m_realFacetNeighborChooser; }

      /// Getter for the Monte Carlo filter.
      MCFacetNeighborChooser& getMCFacetNeighborChooser()
      { return m_mcFacetNeighborChooser; }

      /// Getter for the file name the tree shall be written to.
      std::string getFileName() const;

    private:
      /// ROOT output file
      TFile* m_ptrTFileForOutput;

      /// ROOT tree wrapper helping to file the output tree.
      FacetNeighborChooserTree m_facetNeighborChooserTree;

      MCFacetNeighborChooser m_mcFacetNeighborChooser; ///< Instance of the Monte Carlo filter as reference
      RealFacetNeighborChooser m_realFacetNeighborChooser; ///< Instance of the tested neighbor filter.

    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2


// Implementations

namespace Belle2 {
  namespace CDCLocalTracking {

    template<class RealFacetNeighborChooser>
    EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::EvaluateFacetNeighborChooser()
    {
    }



    template<class RealFacetNeighborChooser>
    EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::~EvaluateFacetNeighborChooser()
    {
    }



    template<class RealFacetNeighborChooser>
    void EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::initialize()
    {

      getMCFacetNeighborChooser().initialize();
      getRealFacetNeighborChooser().initialize();

      m_ptrTFileForOutput = new TFile(getFileName().c_str(), "RECREATE");
      if (m_ptrTFileForOutput) {
        m_facetNeighborChooserTree.create(*m_ptrTFileForOutput);
      }

    }

    template<class RealFacetNeighborChooser>
    NeighborWeight EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::isGoodNeighbor(
      const CDCRecoFacet& facet,
      const CDCRecoFacet& neighborFacet
    )
    {

      if (facet.getStartWire() == neighborFacet.getEndWire()) return NOT_A_NEIGHBOR;

      NeighborWeight mcWeight = getMCFacetNeighborChooser().isGoodNeighbor(facet, neighborFacet);
      NeighborWeight prWeight = getRealFacetNeighborChooser().isGoodNeighbor(facet, neighborFacet);

      m_facetNeighborChooserTree.setValues(mcWeight, prWeight, facet, neighborFacet);
      m_facetNeighborChooserTree.fill();

      return prWeight;
    }



    template<class RealFacetNeighborChooser>
    void EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::terminate()
    {

      m_facetNeighborChooserTree.save();

      if (m_ptrTFileForOutput != nullptr) {
        m_ptrTFileForOutput->Close();
        delete m_ptrTFileForOutput;
        m_ptrTFileForOutput = nullptr;
      }

      getRealFacetNeighborChooser().terminate();
      getMCFacetNeighborChooser().terminate();

    }



    template<class RealFacetNeighborChooser>
    std::string EvaluateFacetNeighborChooser<RealFacetNeighborChooser>::getFileName() const
    {
      return "EvaluateFacetNeighborChooser.root";
    }



  } //end namespace CDCLocalTracking
} //end namespace Belle




#endif //EVALUATEFACETNEIGHBORCHOOSER_H_
