/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef OPTIMIZINGFACETFILTER_H_
#define OPTIMIZINGFACETFILTER_H_

#include <iostream>
#include <fstream>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include "MCFacetFilter.h"
#include "SimpleFacetFilter.h"

namespace Belle2 {
  namespace CDCLocalTracking {
    /// Filter for the constuction of good facets based on simple criterions.
    class OptimizingFacetFilter {

    public:

      /** Constructor. */
      OptimizingFacetFilter();

      /** Destructor.*/
      ~OptimizingFacetFilter();

      CellState isGoodFacet(const CDCRecoFacet& facet) const;

      /// Clears all remember information from the last event
      void clear() const;

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();

    private:
      mutable ofstream m_output_csv;  ///< Output stream for the csv file


      MCFacetFilter m_mcFacetFilter;
      SimpleFacetFilter m_simpleFacetFilter;

    }; // end class OptimizingFacetFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //OPTIMIZINGFACETFILTER_H_
