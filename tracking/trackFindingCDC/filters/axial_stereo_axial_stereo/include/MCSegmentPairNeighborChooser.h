/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseSegmentPairNeighborChooser.h"
#include <tracking/trackFindingCDC/filters/axial_stereo/MCSegmentPairFilter.h>
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    ///Class filtering the neighborhood of axial stereo segment pairs with monte carlo information
    class MCSegmentPairNeighborChooser :
      public Filter<Relation<CDCSegmentPair>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCSegmentPair>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor setting to default reversal symmetry. */
      MCSegmentPairNeighborChooser(bool allowReverse = true);

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
       *  symmetric -  Accept the relation of segment pairs also if the reverse relation is correct
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


      /// Main filter method returning the weight of the neighborhood relation. Return NOT_A_NEIGHBOR if relation shall be rejected.
      virtual NeighborWeight operator()(const CDCSegmentPair& fromSegmentPair,
                                        const CDCSegmentPair& toSegmentPair) IF_NOT_CINT(override final);

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      {
        m_mcSegmentPairFilter.setAllowReverse(allowReverse);
      }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      {
        return m_mcSegmentPairFilter.getAllowReverse();
      }

    private:
      /// Instance of the Monte Carlo axial stereo segment filter for rejection of false cells.
      MCSegmentPairFilter m_mcSegmentPairFilter;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
