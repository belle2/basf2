/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseFacetNeighborChooser.h"

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of facets based on simple criterions.
    class SimpleFacetNeighborChooser : public Filter<Relation<CDCRecoFacet>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCRecoFacet>> Super;

    public:
      /// Constructor using default direction of flight deviation cut off.
      SimpleFacetNeighborChooser() : m_param_deviationCosCut(cos(PI / 180.0 * 180.0))
      {;}

      /// Constructor using given direction of flight deviation cut off.
      SimpleFacetNeighborChooser(FloatType deviationCosCut) :
        m_param_deviationCosCut(deviationCosCut)
      {;}

    public:
      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  deviation_cos_cut - Acceptable deviation cosine in the angle of adjacent tangents to the
       *                      drift circles.
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) IF_NOT_CINT(override);

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() IF_NOT_CINT(override);


    public:
      /** Main filter method returning the weight of the neighborhood relation.*/
      virtual NeighborWeight operator()(const CDCRecoFacet& fromFacet,
                                        const CDCRecoFacet& toFacet) override final;

    private:
      /// Memory for the used direction of flight deviation.
      FloatType m_param_deviationCosCut;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
