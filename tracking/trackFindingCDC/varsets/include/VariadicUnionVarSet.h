/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>
#include <tracking/trackFindingCDC/varsets/UnionVarSet.h>

#include <tracking/trackFindingCDC/utilities/EvalVariadic.h>
#include <tracking/trackFindingCDC/utilities/Named.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>
#include <tracking/trackFindingCDC/utilities/MayBePtr.h>

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Class that accomodates many variable sets and presents them as on set of variables.
     *  In contrast to the UnionVarSet the individual VarSets are given as variadic template parameters.
     *
     *  Dummy implementation based on UnionVarSet. The UnionVarSet can be optimized and leverage that
     *  the types of all nested variable sets are known at compile time.
     */
    template <class... AVarSets>
    class VariadicUnionVarSet : public BaseVarSet<typename FirstType<AVarSets...>::Object> {

    private:
      /// Type of the super class
      using Super = BaseVarSet<typename FirstType<AVarSets...>::Object>;

    public:
      /// Object type from which variables shall be extracted.
      using Object = typename Super::Object;

    private:
      /// Type of the contained variable sets
      using ContainedVarSet = BaseVarSet<Object>;

    public:
      /// Create the union variable set.
      explicit VariadicUnionVarSet()
      {
        EvalVariadic{(m_multiVarSet.push_back(makeUnique<AVarSets>()), std::ignore)...};
        assert(m_multiVarSet.size() == sizeof...(AVarSets));
      }

    public:
      using Super::extract;

      /// Initialize all contained variable set before event processing.
      void initialize() final {
        this->addProcessingSignalListener(&m_multiVarSet);
        Super::initialize();
      }

      /**
       *  Main method that extracts the variable values from the complex object.
       *  @returns  Indication whether the extraction could be completed successfully.
       */
      bool extract(const Object* obj) final {
        return m_multiVarSet.extract(obj);
      }

      // Importing name from the base class.
      using Super::getNamedVariables;

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      std::vector<Named<Float_t*>> getNamedVariables(const std::string& prefix) override
      {
        return m_multiVarSet.getNamedVariables(prefix);
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      MayBePtr<Float_t> find(const std::string& varName) override
      {
        return m_multiVarSet.find(varName);
      }

    private:
      /// Container for the multiple variable sets.
      UnionVarSet<Object> m_multiVarSet;
    };
  }
}
