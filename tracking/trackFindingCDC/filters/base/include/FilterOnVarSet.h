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

#include <tracking/trackFindingCDC/filters/base/Filter.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Filter adapter to make a filter work on a set of variables
     *  Used as base for ChooseableVarialbe, TMVA and RecordingFilters
     */
    template<class AFilter>
    class OnVarSet : public AFilter {

    private:
      /// Type of the base class
      using Super = AFilter;

    public:
      /// Type of pbject to be filtered
      using Object = typename AFilter::Object;

    private:
      /// Type of the var set interface to be used
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor from the variable set the filter should use
      OnVarSet(std::unique_ptr<AVarSet> varSet)
        : m_varSet(std::move(varSet))
      {
        B2ASSERT("Varset initialised as nullptr", m_varSet);
      }

      /// Initialize the filter before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        if (not m_varSet) B2ERROR("Variable set ot setup");
        m_varSet->initialize();
      }

      /// Allow setup work to take place at beginning of new run
      virtual void beginRun() override
      {
        Super::beginRun();
        m_varSet->beginRun();
      }

      /// Allow setup work to take place at beginning of new event
      virtual void beginEvent() override
      {
        Super::beginEvent();
        m_varSet->beginEvent();
      }

      /// Allow clean up to take place at end of run
      virtual void endRun() override
      {
        m_varSet->endRun();
        Super::endRun();
      }

      /// Initialize the filter after event processing.
      virtual void terminate() override
      {
        m_varSet->terminate();
        Super::terminate();
      }

      /// Checks if any variables need Monte Carlo information.
      virtual bool needsTruthInformation() override
      {
        bool result = Super::needsTruthInformation();
        if (result) return true;

        const std::vector<Named<Float_t*> >& namedVariables = m_varSet->getNamedVariables();
        for (const Named<Float_t*>& namedVariable : namedVariables) {
          std::string name = namedVariable.getName();
          // If the name contains the word truth it is considered to have Monte carlo information.
          if (name.find("truth") != std::string::npos) {
            return true;
          }
        }
        return false;
      }

    public:
      /// Function extracting the variables of the object into the variable set.
      virtual Weight operator()(const Object& obj) override
      {
        bool extracted = m_varSet->extract(&obj);
        return extracted ? 1 : NAN;
      }

    public:
      /// Steal the set of variables form this filter - filter becomes disfunctional afterwards
      std::unique_ptr<AVarSet> releaseVarSet()&&
      { return std::move(m_varSet); }

    protected:
      /// Getter for the set of variables
      AVarSet& getVarSet()
      { return *m_varSet; }

      /// Setter for the set of variables
      void setVarSet(std::unique_ptr<AVarSet> varSet)
      { m_varSet = std::move(varSet); }

    private:
      /// Instance of the variable set to be used in the filter.
      std::unique_ptr<AVarSet> m_varSet;

    };

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
      FilterOnVarSet()
        : Super(std::unique_ptr<AVarSet>(new AVarSet()))
      {}
    };
  }
}
