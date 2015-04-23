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

#include <tracking/trackFindingCDC/filters/axial_axial/MCAxialAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include "BaseSegmentTripleFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of segment triples based on monte carlo information
    class MCSegmentTripleFilter  : public Filter<CDCSegmentTriple> {

    private:
      /// Type of the super class
      typedef Filter<CDCSegmentTriple> Super;

    public:
      /// Constructor initializing the symmetry flag.
      MCSegmentTripleFilter(bool allowReverse = true);

    public:
      /// May be used to clear information from former events. Currently unused.
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the modules initialize to the filter
      virtual void terminate() IF_NOT_CINT(override final);

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the axial segment triple if the reverse segment triple
       *               is correct preserving the progagation reversal symmetry
       *               on this level of detail.
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

      /// Check if the segment triple is aligned in the Monte Carlo track. Signals NOT_A_CELL if not.
      virtual CellWeight operator()(const CDCSegmentTriple& triple) IF_NOT_CINT(override final);

    private:
      /// Sets the trajectories of the segment triple from Monte Carlo information. IS executed for good segment triples.
      void setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      {
        m_param_allowReverse = allowReverse;
        m_mcAxialAxialSegmentPairFilter.setAllowReverse(allowReverse);
      }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      {
        return m_param_allowReverse;
      }

    private:
      /// Switch to indicate if the reversed version of the segment triple shall also be accepted (default is true).
      bool m_param_allowReverse;

      /// Instance of the cell filter to reject neighborhoods of false cells.
      MCAxialAxialSegmentPairFilter m_mcAxialAxialSegmentPairFilter;


    }; // end class MCSegmentTripleFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
