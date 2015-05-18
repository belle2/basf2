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

#ifndef MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
#define MCSEGMENTTRIPLENEIGHBORCHOOSER_H_

#include "BaseSegmentTripleRelationFilter.h"
#include <tracking/trackFindingCDC/filters/segment_triple/MCSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    ///Class filtering the neighborhood of segment triples with monte carlo information
    class MCSegmentTripleRelationFilter: public Filter<Relation<CDCSegmentTriple>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCSegmentTriple>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /** Constructor. */
      MCSegmentTripleRelationFilter(bool allowReverse = true);

      /// Clears stored information for a former event
      virtual void clear() IF_NOT_CINT(override final);

      /// Forwards the initialize method from the module
      virtual void initialize() IF_NOT_CINT(override final);

      /// Forwards the terminate method from the module
      virtual void terminate() IF_NOT_CINT(override final);

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the relation of segment triples also if the reverse relation is correct
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


      /** Main filter method returning the weight of the neighborhood relation.
       *  Return NOT_A_NEIGHBOR if relation shall be rejected.*/
      virtual NeighborWeight operator()(const CDCSegmentTriple& triple,
                                        const CDCSegmentTriple& neighborTriple) IF_NOT_CINT(override final);

      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      {
        m_mcSegmentTripleFilter.setAllowReverse(allowReverse);
      }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      {
        return m_mcSegmentTripleFilter.getAllowReverse();
      }

    private:
      /// Instance of the Monte Carlo segment triple filter for rejection of false cells.
      MCSegmentTripleFilter m_mcSegmentTripleFilter;

    }; // end class


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //MCSEGMENTTRIPLENEIGHBORCHOOSER_H_
