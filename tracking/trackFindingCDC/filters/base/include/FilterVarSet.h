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

#include <framework/core/ModuleParamList.h>

#include <boost/algorithm/string/predicate.hpp>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    constexpr
    static char const* const filterVarNames[] = {
      "accept",
      "weight",
      "positive",
    };

    /// Name of the variables that should be extracted from the response of a filter.
    template<class AFilter>
    class FilterVarNames : public VarNames<typename AFilter::Object> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(filterVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return filterVarNames[iName];
      }
    };

    /**
     *  A variable set based that represents the response of a filter.
     *
     *  The variables that are extracted from the filter response are the weight
     *  and a boolean whether the response was NaN.
     *  Class that computes floating point variables from segment pair.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    template<class AFilter>
    class  FilterVarSet : public VarSet<FilterVarNames<AFilter> > {

    private:
      /// Type of the base class
      using Super = VarSet<FilterVarNames<AFilter> >;

    public:
      /// Type of the filter
      using Filter = AFilter;

      /// Type of the object from which the filter judgement should be extracted.
      using Object = typename Filter::Object;

      /// Construct the varset.
      FilterVarSet(const std::string& filterName = "",
                   std::unique_ptr<Filter> ptrFilter = makeUnique<Filter>()) :
        Super(),
        m_filterName(filterName),
        m_filterNamePrefix(filterName + '_'),
        m_ptrFilter(std::move(ptrFilter))
      {
      }

      using Super::named;

      /// Generate filter weight variable from the object
      virtual bool extract(const Object* obj) override final
      {
        bool extracted = Super::extract(obj);
        if (m_ptrFilter and obj) {
          Weight weight = (*m_ptrFilter)(*obj);
          this->template var<named("weight")>() = weight;
          this->template var<named("accept")>() = not std::isnan(weight) and not(weight < m_cut);
          this->template var<named("positive")>() = weight > 0 and not(weight < m_cut);

          // Forward the nested result.
          return extracted;
        } else {
          return false;
        }

      }

      /// Initialize the filter before event processing
      virtual void initialize() override final
      {
        Super::initialize();

        ModuleParamList moduleParamList;
        const std::string prefix = "";
        m_ptrFilter->exposeParameters(&moduleParamList, prefix);

        // try to find the TMVAFilter cut parameter and reset it such that we can set it ourself
        try {
          ModuleParam<double> cutParam = moduleParamList.getParameter<double>("cut");
          m_cut = cutParam.getValue();
          cutParam.setValue(NAN);
        } catch (ModuleParamList::ModuleParameterNotFoundError) {
          // Not found continue
        }

        if (m_ptrFilter) {
          m_ptrFilter->initialize();
        }
      }

      /// Allow setup work to take place at beginning of new run
      virtual void beginRun() override final
      {
        Super::beginRun();
        if (m_ptrFilter) {
          m_ptrFilter->beginRun();
        }
      }

      /// Allow setup work to take place at beginning of new event
      virtual void beginEvent() override final
      {
        Super::beginEvent();
        if (m_ptrFilter) {
          m_ptrFilter->beginEvent();
        }
      }

      /// Allow clean up to take place at end of run
      virtual void endRun() override final
      {
        if (m_ptrFilter) {
          m_ptrFilter->endRun();
        }
        Super::endRun();
      }

      /// Terminate the filter after event processing
      virtual void terminate() override final
      {
        if (m_ptrFilter) {
          m_ptrFilter->terminate();
        }
        Super::terminate();
      }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(std::string prefix = "") override
      {
        return Super::getNamedVariables(prefix + m_filterNamePrefix);
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string varName) override
      {
        if (boost::starts_with(varName, m_filterNamePrefix)) {
          std::string varNameWithoutPrefix = varName.substr(m_filterNamePrefix.size());
          MayBePtr<Float_t> found = Super::find(varNameWithoutPrefix);
          if (found) return found;
        }
        return nullptr;
      }

    public:
      /// The cut on the TMVA output.
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
