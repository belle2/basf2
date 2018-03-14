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

#include <tracking/trackFindingCDC/filters/base/Filter.fwd.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter adapter to make a filter work on a set of variables
    template<class AFilter>
    class OnVarSet : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Type of pbject to be filtered
      using Object = typename AFilter::Object;

    private:
      /// Type of the variable set usedi in this filter.
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor from the variable set the filter should use
      explicit OnVarSet(std::unique_ptr<AVarSet> varSet);

      /// Default destructor
      ~OnVarSet();

      // No reassignment of variable set possible for now
      void initialize() override;

      /// Checks if any variables need Monte Carlo information.
      bool needsTruthInformation() override;

    public:
      /// Function extracting the variables of the object into the variable set.
      Weight operator()(const Object& obj) override;

    public:
      /// Steal the set of variables form this filter - filter becomes disfunctional afterwards.
      std::unique_ptr<AVarSet> releaseVarSet()&& ;

    protected:
      /// Getter for the set of variables
      AVarSet& getVarSet() const;

      /// Setter for the set of variables
      void setVarSet(std::unique_ptr<AVarSet> varSet);

    private:
      /// Instance of the variable set to be used in the filter.
      std::unique_ptr<AVarSet> m_varSet;
    };
  }
}

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Convience template to create a filter operating on a specific set of variables.
    template<class AVarSet>
    class FilterOnVarSet: public OnVarSet<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      using Super = OnVarSet<Filter<typename AVarSet::Object> >;

    public:
      /// Type of the object to be analysed.
      using Object = typename AVarSet::Object;

    public:
      /// Constructor of the filter.
      FilterOnVarSet();

      /// Default destructor
      ~FilterOnVarSet();
    };
  }
}
