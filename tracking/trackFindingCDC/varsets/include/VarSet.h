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

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/varsets/FixedSizeNamedFloatTuple.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /** Generic class that generates some named float values from a given object.
     *  This object template provides the memory and the names of the float values.
     *  The filling from the complex object is specialised in the derived class
     *
     *  As a template parameter it takes class with three containing parameters.
     *  Object - The class of the complex object from which to extract the variable.
     *  nNames - Number of variables that will be extracted from the complex object.
     *  names - Array of names which contain the nNames names of the float values.
     **/
    template<class AObjectVarNames>
    class VarSet : public BaseVarSet<typename AObjectVarNames::Object> {

    private:
      /// Type of the super class
      typedef BaseVarSet<typename AObjectVarNames::Object> Super;

    public:
      /// Type from which variables should be extracted
      typedef typename Super::Object Object;

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = AObjectVarNames::nNames;

    public:
      /**
       *  Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
       */
      virtual void initialize() override
      { m_nestedVarSet.initialize(); }

      /// Allow setup work to take place at beginning of new run
      virtual void beginRun() override
      { m_nestedVarSet.beginRun(); }

      /// Allow setup work to take place at beginning of new event
      virtual void beginEvent() override
      { m_nestedVarSet.beginEvent(); }

      /// Allow clean up to take place at end of run
      virtual void endRun() override
      { m_nestedVarSet.endRun(); }

      /**
       *  Terminate the variable set after event processing.
       *  Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual void terminate() override
      { m_nestedVarSet.terminate(); }

      /// Extract the nested variables next
      bool extractNested(const Object* obj)
      {
        return m_nestedVarSet.extract(AObjectVarNames::getNested(obj));
      }

      using Super::extract;

      /// Main method that extracts the variable values from the complex object.
      virtual bool extract(const Object* obj) override
      {
        return extractNested(obj);
      }

      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(std::string prefix = "") override
      {
        std::vector<Named<Float_t*> > result = m_nestedVarSet.getNamedVariables(prefix);
        std::vector<Named<Float_t*> > extend = m_variables.getNamedVariables(prefix);
        result.insert(result.end(), extend.begin(), extend.end());
        return result;
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string varName) override
      {
        MayBePtr<Float_t> found = m_nestedVarSet.find(varName);
        if (found) return found;
        return m_variables.find(varName);
      }

    protected:
      /** Getter for the index from the name.
       *  Looks through the associated names and returns the right index if found
       *  Returns nVars (one after the last element) if not found.
       *
       *  @param name       Name of the sought variable
       *  @return           Index of the name, nVars if not found.
       */
      constexpr
      static int named(const char* const name)
      {
        return index<nVars>(AObjectVarNames::getName, name);
      }

      /// Getter for the value of the ith variable. Static version.
      template<int I>
      Float_t get() const
      {
        static_assert(I < nVars, "Requested variable index exceeds number variables.");
        return m_variables.get(I);
      }

      /// Reference getter for the value of the ith variable. Static version.
      template<int I>
      Float_t& var()
      {
        static_assert(I < nVars, "Requested variable index exceeds number variables.");
        return m_variables[I];
      }

      /// Set the given variable to the value if the value is not NaN els set it to valueIfNaN.
      template<int I>
      void setVariableIfNotNaN(const Float_t& value, const Float_t& valueIfNaN = 0)
      {
        if (std::isnan(value) or std::isinf(value)) {
          var<I>() = valueIfNaN;
        } else {
          var<I>() = value;
        }
      }

    public:
      /// Memory for nNames floating point values.
      FixedSizeNamedFloatTuple<AObjectVarNames> m_variables;

      /// Nested VarSet implementing a chain of sets until EmptyVarSet terminates the sequence.
      typename AObjectVarNames::NestedVarSet m_nestedVarSet;

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
