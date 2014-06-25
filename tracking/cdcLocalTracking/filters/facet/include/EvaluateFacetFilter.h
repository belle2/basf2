/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVALUATEFACETFILTER_H_
#define EVALUATEFACETFILTER_H_

#include <iostream>
#include <fstream>

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>

#include "MCFacetFilter.h"
#include "FacetFilterTree.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for facets using a real facet filter and compares it to the Monte Carlo decision and writing the events to file for analysis.
    template<class RealFacetFilter>
    class EvaluateFacetFilter {

    public:

      /** Constructor. */
      EvaluateFacetFilter();

      /** Destructor.*/
      ~EvaluateFacetFilter();

      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL if the cell shall be rejected.
      CellState isGoodFacet(const CDCRecoFacet& facet);

      /// Clears all remember information from the last event
      void clear();

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

      /// Getter for the file name the tree shall be written to
      std::string getFileName() const;

      /// Getter for the filter to be evaluted
      RealFacetFilter& getRealFacetFilter()
      { return m_realFacetFilter; }

      /// Getter for the Monte Carlo filter
      MCFacetFilter& getMCFacetFilter()
      { return m_mcFacetFilter; }

    private:
      /// ROOT output file
      TFile* m_ptrTFileForOutput;

      /// ROOT tree wrapper helping to file the output tree.
      FacetFilterTree m_facetFilterTree;

      MCFacetFilter m_mcFacetFilter; ///< Instance of Monte Carlo facet filter.
      RealFacetFilter m_realFacetFilter; ///< Instance of evaluated facet filter.

    }; // end class EvaluateFacetFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2





namespace Belle2 {
  namespace CDCLocalTracking {

    template<class RealFacetFilter>
    EvaluateFacetFilter<RealFacetFilter>::EvaluateFacetFilter()
    {
    }


    template<class RealFacetFilter>
    EvaluateFacetFilter<RealFacetFilter>::~EvaluateFacetFilter()
    {
    }


    template<class RealFacetFilter>
    CellWeight EvaluateFacetFilter<RealFacetFilter>::isGoodFacet(const CDCRecoFacet& facet)
    {
      CellWeight mcWeight = getMCFacetFilter().isGoodFacet(facet);
      CellWeight prWeight = getRealFacetFilter().isGoodFacet(facet);

      //do fits
      facet.adjustLines();

      m_facetFilterTree.setValues(mcWeight, prWeight, facet);
      m_facetFilterTree.fill();

      return prWeight;
    }



    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::clear()
    {
    }



    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::initialize()
    {
      getMCFacetFilter().initialize();
      getRealFacetFilter().initialize();

      m_ptrTFileForOutput = new TFile(getFileName().c_str(), "RECREATE");
      if (m_ptrTFileForOutput) {
        m_facetFilterTree.create(*m_ptrTFileForOutput);
      }
    }



    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::terminate()
    {
      m_facetFilterTree.save();

      if (m_ptrTFileForOutput != nullptr) {
        m_ptrTFileForOutput->Close();
        delete m_ptrTFileForOutput;
        m_ptrTFileForOutput = nullptr;
      }

      getRealFacetFilter().terminate();
      getMCFacetFilter().terminate();

    }



    template<class RealFacetFilter>
    std::string EvaluateFacetFilter<RealFacetFilter>::getFileName() const
    {
      return "EvaluateFacetFilter.root";
    }

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //EVALUATEFACETFILTER_H_
