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
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include "MCFacetFilter.h"

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

      CellState isGoodFacet(const CDCRecoFacet& facet) const;

      /// Clears all remember information from the last event
      void clear() const;

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

    private:
      mutable std::ofstream m_output_csv;  ///< Output stream for the csv file


      MCFacetFilter m_mcFacetFilter;
      RealFacetFilter m_realFacetFilter;

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
    CellState EvaluateFacetFilter<RealFacetFilter>::isGoodFacet(const CDCRecoFacet& facet) const
    {

      CellState mcCellWeight = m_mcFacetFilter.isGoodFacet(facet);
      //bool mcDecision = (not isNotACell(mcCellWeight));

      //CDCRecoFacet reversedFacet = facet.reversed();
      //CellState mcReversedCellWeight = m_mcFacetFilter.isGoodFacet(reversedFacet);

      bool mcDecision = not isNotACell(mcCellWeight);

      facet.adjustLines();

      const ParameterLine2D& startToMiddle = facet.getStartToMiddleLine();
      const ParameterLine2D& startToEnd    = facet.getStartToEndLine();
      const ParameterLine2D& middleToEnd   = facet.getMiddleToEndLine();

      m_output_csv << startToMiddle.tangential().cosWith(startToEnd.tangential()) << ","
                   << startToEnd.tangential().cosWith(middleToEnd.tangential()) << ","
                   << startToMiddle.tangential().cosWith(middleToEnd.tangential()) << ","
                   << mcDecision
                   << std::endl;

      //return mcCellWeight;

      CellState realCellWeight = m_realFacetFilter.isGoodFacet(facet);
      return realCellWeight;


    }


    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::clear() const {;}


    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::initialize()
    {
      m_output_csv.open("facet_creator_filter.csv");
      m_output_csv << "SMToSECosine,"
                   << "SEToMECosine,"
                   << "SMToMECosine,"
                   << "MCTruth"
                   << std::endl;
    }


    template<class RealFacetFilter>
    void EvaluateFacetFilter<RealFacetFilter>::terminate()
    {
      m_output_csv.close();
    }



  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //EVALUATEFACETFILTER_H_
