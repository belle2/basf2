/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/track_relation/BaseTrackRelationFilter.h>
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of track relations based on MC information.
    class MCTrackRelationFilter : public Filter<Relation<CDCTrack> > {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCTrack> > Super;

    public:
      /// Constructor
      MCTrackRelationFilter(bool allowReverse = true) :
        m_param_allowReverse(allowReverse) {;}

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the track relation if the reverse track relation
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

      /// Checks if a track relation is a good combination.
      virtual NeighborWeight operator()(const CDCTrack& fromTrack,
                                        const CDCTrack& toTrack) override final;

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      { m_param_allowReverse = allowReverse; }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      { return m_param_allowReverse; }

    private:
      /// Switch to indicate if the reversed version of the track pair shall also be accepted (default is true).
      bool m_param_allowReverse;

    }; // end class MCTrackRelationFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
