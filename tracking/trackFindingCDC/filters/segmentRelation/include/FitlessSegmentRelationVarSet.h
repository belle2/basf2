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


#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/ca/Relation.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration
    class CDCSegmentRelation;

    /// Names of the variables to be generated.
    constexpr
    static char const* const fitlessSegmentRelationVarNames[] = {
      "from_ndf",
      "to_ndf",

      "from_chi2_over_ndf",
      "to_chi2_over_ndf",

      "from_p_value",
      "to_p_value",

      "abs_avg_curv",
      "delta_curv",
      "delta_curv_var",
      "delta_curv_pull",

      "delta_pos_phi",
      "delta_mom_phi",
      "from_delta_mom_phi",
      "to_delta_mom_phi",
      "delta_alpha",

      "arc_length_front_offset",
      "arc_length_back_offset",
      "from_arc_length_total",
      "to_arc_length_total",
      "arc_length_gap",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment
     */
    class FitlessSegmentRelationVarNames : public VarNames<Relation<const CDCRecoSegment2D> > {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(fitlessSegmentRelationVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return fitlessSegmentRelationVarNames[iName];
      }
    };

    /** Class that computes floating point variables from a facet relation.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class FitlessSegmentRelationVarSet : public VarSet<FitlessSegmentRelationVarNames> {

    private:
      /// Type of the base class
      using Super = VarSet<FitlessSegmentRelationVarNames>;

    public:
      /// Construct the varset.
      explicit FitlessSegmentRelationVarSet();

      /// Generate and assign the variables from the segment pair
      virtual bool extract(const Relation<const CDCRecoSegment2D>* ptrSegmentRelation) override;
    };
  }
}
