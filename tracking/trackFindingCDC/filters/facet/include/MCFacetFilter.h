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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of good facets based on monte carlo information
    class MCFacetFilter : public Filter<CDCFacet> {
    private:
      /// Type of the super class
      typedef Filter<CDCFacet> Super;

    public:
      /** Constructor also setting the switch ,
       *  if the reversed version of a facet (in comparision to MC truth) shall be accepted.
       */
      MCFacetFilter(bool allowReverse = true) : m_param_allowReverse(allowReverse) {;}

    public:
      /** Main filter method returning the weight of the facet.
       *  Returns NOT_A_CELL if the cell shall be rejected.
       */
      virtual CellWeight operator()(const CDCFacet& facet) IF_NOT_CINT(override final);

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the facet if the reverse facet is correct
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

    private:
      /// Indicated if the oriented triple is a correct hypotheses
      bool operator()(const CDCRLWireHitTriple& rlWireHit, int inTrackHitDistanceTolerance = 99999);

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      { m_param_allowReverse = allowReverse; }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      { return m_param_allowReverse; }

    private:
      /** Switch to indicate if the reversed version of the facet
       *  shall also be accepted (default is true).
       */
      bool m_param_allowReverse;

    }; // end class MCFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
