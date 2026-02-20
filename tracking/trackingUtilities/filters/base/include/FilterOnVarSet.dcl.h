/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackingUtilities/filters/base/Filter.fwd.h>

#include <tracking/trackingUtilities/varsets/BaseVarSet.h>

#include <tracking/trackingUtilities/numerics/Weight.h>

#include <memory>

namespace Belle2 {
  namespace TrackingUtilities {

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

      /// No reassignment of variable set possible for now
      void initialize() override;

      /// Checks if any variables need Monte Carlo information.
      bool needsTruthInformation() override;

    public:
      /// Function extracting the variables of the object into the variable set.
      Weight operator()(const Object& obj) override;

    public:
      /// Steal the set of variables form this filter - filter becomes dysfunctional afterwards.
      std::unique_ptr<AVarSet> releaseVarSet()&& ;

    protected:
      /// Getter for the set of variables
      AVarSet& getVarSet() const;

      /// Setter for the set of variables
      void setVarSet(std::unique_ptr<AVarSet> varSet);

      /// Forward prefixed parameters of this findlet to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    private:
      /// Instance of the variable set to be used in the filter.
      std::unique_ptr<AVarSet> m_varSet;
    };
  }
}

#include <tracking/trackingUtilities/filters/base/Filter.icc.h>

namespace Belle2 {
  namespace TrackingUtilities {

    /// Convenience template to create a filter operating on a specific set of variables.
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
