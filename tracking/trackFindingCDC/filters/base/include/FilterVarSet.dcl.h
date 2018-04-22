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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <RtypesCore.h>

#include <string>
#include <memory>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {

    /// Vehicle class to transport the variable names
    template<class AFilter>
    struct FilterVarNames : public VarNames<typename AFilter::Object> {

      /// Number of variables to be generated
      static const size_t nVars;

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName);
    };

    /**
     *  Class to compute floating point variables from a filter response
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     *
     *  The variables that are extracted from the filter response are the weight
     *  and a boolean whether the response was NaN.
     */
    template <class AFilter>
    class FilterVarSet : public VarSet<FilterVarNames<AFilter>> {

    private:
      /// Type of the base class
      using Super = VarSet<FilterVarNames<AFilter> >;

    public:
      /// Type of the filter
      using Filter = AFilter;

      /// Type of the object from which the filter judgement should be extracted.
      using Object = typename Filter::Object;

      /// Construct the varset making an instance of the template filter.
      FilterVarSet(const std::string& filterName = "");

      /// Construct the varset.
      FilterVarSet(const std::string& filterName, std::unique_ptr<Filter> ptrFilter);

      using Super::named;

      /// Generate filter weight variable from the object
      bool extract(const Object* obj) final;

      /// Initialize the filter before event processing
      void initialize() final;

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      std::vector<Named<Float_t*>> getNamedVariables(const std::string& prefix) override;

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      MayBePtr<Float_t> find(const std::string& varName) override;

    public:
      /// The cut on the filter output.
      double m_cut = NAN;

      /// Name of the filter
      std::string m_filterName;

      /// Prefix to be put in front of the filter variables
      std::string m_filterNamePrefix;

      /// Filter from which to generate weight as a variable set;
      std::unique_ptr<Filter> m_ptrFilter;
    };
  }
}
