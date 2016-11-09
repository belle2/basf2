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

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <vector>
#include <string>
#include <limits>
#include <cassert>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Generic class that generates some named float values from a given object.
     *  This object template provides the memory and the names of the float values.
     *  The filling from the complex object is specialised in the derived class
     *
     *  As a template parameter it takes class with three containing parameters.
     *  Object - The class of the complex object from which to extract the variable.
     *  nNames - Number of variables that will be extracted from the complex object.
     *  names - Array of names which contain the nNames names of the float values.
     */
    template<class AObjectVarNames>
    class VarSet : public BaseVarSet<typename AObjectVarNames::Object> {

    private:
      /// Type of the super class
      using Super = BaseVarSet<typename AObjectVarNames::Object>;

    public:
      /// Type from which variables should be extracted
      using Object = typename Super::Object;

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = AObjectVarNames::nNames;

    public:
      /**
       *  Getter for the named references to the individual variables
       *  Base implementaton returns empty vector
       */
      virtual std::vector<Named<Float_t*> > getNamedVariables(std::string prefix) override
      {
        return m_variables.getNamedVariables(prefix);
      }

      /**
       *   Pointer to the variable with the given name.
       *   Returns nullptr if not found.
       */
      virtual MayBePtr<Float_t> find(std::string varName) override
      {
        return m_variables.find(varName);
      }

    protected:
      /**
       *  Getter for the index from the name.
       *  Looks through the associated names and returns the right index if found
       *  Returns nVars (one after the last element) if not found.
       *
       *  @param name       Name of the sought variable
       *  @return           Index of the name, nVars if not found.
       */
      constexpr static int named(const char* name)
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

      /// Helper construct to assign a finite value to float variables.
      template<typename AFloat>
      struct AssignFinite {
        /// Setup the assignment to a variable
        AssignFinite(AFloat& value) : m_value(value) {}

        /// Unpacker of the wrapper
        operator AFloat& () { return m_value; }

        /// Assign value replacing infinite values with the maximum value possible
        void operator=(const AFloat value)
        {
          m_value = value;
          if (not std::isfinite(value)) {
            m_value = std::copysign(std::numeric_limits<AFloat>::max(), value);
          }
        }

      private:
        /// Reference to the variable to be assigned
        AFloat& m_value;
      };

      /// Reference getter for the value of the ith variable. Transforms non-finite values to finite value
      template<int I>
      AssignFinite<Float_t> finitevar()
      {
        static_assert(I < nVars, "Requested variable index exceeds number variables.");
        return AssignFinite<Float_t>(m_variables[I]);
      }

    public:
      /// Memory for nNames floating point values.
      FixedSizeNamedFloatTuple<AObjectVarNames> m_variables;
    };
  }
}
