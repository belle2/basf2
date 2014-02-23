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

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>
#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>

#include "BaseFacetNeighborChooser.h"
#include "MCFacetNeighborChooser.h"

namespace Belle2 {
  namespace CDCLocalTracking {

    ///Class filtering the neighborhood of facets based on simple criterions.
    template<class RealFacetNeighborChooser>
    class EvaluateFacetNeighborChooser : public BaseFacetNeighborChooser {

    public:
      /// Empty constructor
      EvaluateFacetNeighborChooser() {
      }

      /// Empty deconstructor
      ~EvaluateFacetNeighborChooser() {
      }

      /// Forwards the modules initialize to the filter
      void initialize() {
        m_output_csv.open("facet_neighbor_chooser.csv");

        //Headline
        m_output_csv << "SMToNeighborSECosing,"
                     << "SEToNeighborMECosing,"
                     << "MCTruth"
                     << std::endl;
      }


      /// Forwards the modules initialize to the filter
      void terminate() {
        m_output_csv.close();
      }

      inline NeighborWeight isGoodNeighbor(
        const CDCRecoFacet& facet,
        const CDCRecoFacet& neighborFacet
      ) const {

        if (facet.getStartWire() == neighborFacet.getEndWire()) return NOT_A_NEIGHBOR;

        NeighborWeight mcWeight = m_mcNeighborChooser.isGoodNeighbor(facet, neighborFacet);

        bool mcDecision = not isNotANeighbor(mcWeight);


        //the compatibility of the short legs or all?
        //start end to continuation middle end
        //start middle to continuation start end

        const ParameterLine2D& firstStartToMiddle = facet.getStartToMiddleLine();
        const ParameterLine2D& firstStartToEnd    = facet.getStartToEndLine();

        const ParameterLine2D& secondStartToEnd   = neighborFacet.getStartToEndLine();
        const ParameterLine2D& secondMiddleToEnd   = neighborFacet.getMiddleToEndLine();


        m_output_csv << firstStartToMiddle.tangential().cosWith(secondStartToEnd.tangential())  << ","
                     << firstStartToEnd.tangential().cosWith(secondMiddleToEnd.tangential()) << ","
                     << mcDecision
                     << std::endl;

        NeighborWeight realWeight = m_realNeighborChooser.isGoodNeighbor(facet, neighborFacet);
        return realWeight;

      }


    private:

      mutable ofstream m_output_csv;  ///< Output stream for the csv file

      MCFacetNeighborChooser m_mcNeighborChooser;
      RealFacetNeighborChooser m_realNeighborChooser;


    }; // end class


  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //EVALUATEFACETNEIGHBORCHOOSER_H_
