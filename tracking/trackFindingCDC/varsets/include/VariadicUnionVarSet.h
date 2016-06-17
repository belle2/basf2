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

#include <tracking/trackFindingCDC/varsets/UnionVarSet.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>

#include <vector>
#include <string>
#include <memory>
#include <assert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class that accomodates many variable sets and presents them as on set of variables.
     *  In contrast to the UnionVarSet the individual VarSets are given as variadic template parameters.
     *
     *  Dummy implementation based on UnionVarSet. The UnionVarSet can be optimized and leverage that
     *  the types of all nested variable sets are known at compile time.
     */
    template<class... AVarSets>
    class VariadicUnionVarSet : public BaseVarSet<typename FirstType<AVarSets...>::Object> {

    private:
      /// Type of the super class
      typedef BaseVarSet<typename FirstType<AVarSets...>::Object> Super;

    public:
      /// Object type from which variables shall be extracted.
      typedef typename Super::Object Object;

    private:
      /// Type of the contained variable sets
      typedef BaseVarSet<Object> ContainedVarSet;

    public:
      /// Create the union variable set.
      explicit VariadicUnionVarSet()
      {
        EvalVariadic{
          (m_multiVarSet.push_back(std::unique_ptr<ContainedVarSet>(new AVarSets())) , true)...
        };

        assert(m_multiVarSet.size() == sizeof...(AVarSets));
      }

    public:
      using Super::extract;

      /**
       *  Initialize all contained variable set before event processing.
       */
      virtual void initialize() override final
      { m_multiVarSet.initialize(); }

      /// Signal the beginning of a new run
      virtual void beginRun() override final
      { m_multiVarSet.beginRun(); }

      /// Signal the beginning of a new event
      virtual void beginEvent() override
      { m_multiVarSet.beginEvent(); }

      /// Signal the end of a run
      virtual void endRun() override
      { m_multiVarSet.beginRun(); }

      /**
       *  Terminate all contained variable set after event processing.
       */
      virtual void terminate() override final
      { m_multiVarSet.terminate(); }

      /**
       *  Main method that extracts the variable values from the complex object.
       *  @returns  Indication whether the extraction could be completed successfully.
       */
      virtual bool extract(const Object* obj) override final
      { return m_multiVarSet.extract(obj); }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(std::string prefix = "") override
      { return m_multiVarSet.getNamedVariables(prefix); }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string varName) override
      { return m_multiVarSet.find(varName); }

    private:
      /// Container for the multiple variable sets.
      UnionVarSet<Object> m_multiVarSet;

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
