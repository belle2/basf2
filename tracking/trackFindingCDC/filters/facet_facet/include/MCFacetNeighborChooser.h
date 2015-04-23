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

#include <tracking/trackFindingCDC/filters/facet/MCFacetFilter.h>
#include "BaseFacetNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Class filtering the neighborhood of facets with monte carlo information
    class MCFacetNeighborChooser : public Filter<Relation<CDCRecoFacet>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCRecoFacet>> Super;

    public:
      /** Constructor also setting the switch if the reversed version of a facet
       *  (in comparision to MC truth) shall be accepted.
       */
      MCFacetNeighborChooser(bool allowReverse = true) : m_mcFacetFilter(allowReverse) {;}

    public:
      /// May be used to clear information from former events. Currently unused.
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void terminate() IF_NOT_CINT(override final);


    public:
      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the relation facet if the reverse relation facet is correct
       *               preserving the progagation reversal symmetry on this level of detail.
       *               Allowed values "true", "false". Default is "true".
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) IF_NOT_CINT(override);

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() IF_NOT_CINT(override);

      /// Indicates that the filter requires Monte Carlo information.
      virtual bool needsTruthInformation() IF_NOT_CINT(override final);

    public:
      /** Main filter method returning the weight of the neighborhood relation.
       *  Return NOT_A_NEIGHBOR if relation shall be rejected.
       */
      virtual NeighborWeight operator()(const CDCRecoFacet& fromFacet,
                                        const CDCRecoFacet& toFacet) override final;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      {
        m_mcFacetFilter.setAllowReverse(allowReverse);
      }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      {
        return m_mcFacetFilter.getAllowReverse();
      }

    private:
      /// Monte Carlo cell filter to reject neighborhoods have false cells
      MCFacetFilter m_mcFacetFilter;

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
