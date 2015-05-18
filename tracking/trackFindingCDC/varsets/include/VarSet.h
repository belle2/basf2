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

#include "FixedSizeNamedFloatTuple.h"

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>

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
    template<class ObjectVarNames>
    class VarSet {

    private:
      /// Number of floating point values represented by this class.
      static const size_t nVars = ObjectVarNames::nNames;

    public:
      /// Object type from which variables shall be extracted.
      typedef typename ObjectVarNames::Object Object;

    public:
      /// Constructure taking a optional prefix that can be attached to the names if request.
      VarSet(const std::string& prefix = "") :
        m_variables(prefix),
        m_nestedVarSet(prefix)
      {;}

      /// Virtual destructor
      virtual ~VarSet() {;}

      /// Extract the nested variables next
      bool extractNested(const Object* obj)
      {
        return m_nestedVarSet.extract(ObjectVarNames::getNested(obj));
      }

      /// Main method that extracts the variable values from the complex object.
      virtual bool extract(const Object* obj)
      {
        return extractNested(obj);
      }

      /// Method for extraction from an object instead of a pointer.
      bool extract(const Object& obj)
      {
        return extract(&obj);
      }

      /** Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
       */
      virtual void initialize()
      {
        m_nestedVarSet.initialize();
      }

      /** Terminate the variable set after event processing.
       *  Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual void terminate()
      {
        m_nestedVarSet.terminate();
      }

      /** Getter for the named tuples storing the values of all the (possibly nested) VarSets
       *  Base implementation returns the  named tuple of this variable set.
       */
      virtual std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables()
      {
        std::vector<NamedFloatTuple*> result = m_nestedVarSet.getAllVariables();
        result.push_back(&m_variables);
        return result;
      }

      /** Const getter for the named tuples storing the values of all the (possibly nested)
       *  variable sets. Base implementation returns the named tuple of this variable set.
       */
      virtual std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() const
      {
        std::vector<const NamedFloatTuple*> result = m_nestedVarSet.getAllVariables();
        result.push_back(&m_variables);
        return result;
      }

      /// Getter for a map of all name including the optional prefix and value pairs
      std::map<std::string, Float_t> getNamedValuesWithPrefix() const
      {
        std::map<std::string, Float_t> namedValues;
        std::vector<const NamedFloatTuple*> allVariables = getAllVariables();
        for (const NamedFloatTuple* variables : allVariables) {
          size_t nVariables = variables->size();
          for (size_t iVariable = 0; iVariable < nVariables; ++iVariable) {
            std::string name = variables->getNameWithPrefix(iVariable);
            Float_t value = variables->get(iVariable);
            namedValues[name] = value;
          }
        }
        return namedValues;
      }

    protected:
      /** Getter for the index from the name.
       *  Looks through the associated names and returns the right index if found
       *  Returns nVars (one after the last element) if not found.
       *
       *  @param name       Name of the sought variable
       *  @return           Index of the name, nVars if not found.
       */
      IF_NOT_CINT(constexpr)
      static int named(const char* const name)
      {
        return index<nVars>(ObjectVarNames::getName, name);
      }

      /// Getter for the value of the ith variable. Static version.
      template<int iVar>
      Float_t get() const
      {
        IF_NOT_CINT(static_assert(iVar < nVars,
                                  "Requested variable index exceeds number variables.");)
        return m_variables.get(iVar);
      }

      /// Reference getter for the value of the ith variable. Static version.
      template<int iVar>
      Float_t& var()
      {
        IF_NOT_CINT(static_assert(iVar < nVars,
                                  "Requested variable index exceeds number variables.");)
        return m_variables[iVar];
      }

    public:
      /// Memory for nNames floating point values.
      FixedSizeNamedFloatTuple<ObjectVarNames> m_variables;

      /// Nested VarSet implementing a chain of sets until EmptyVarSet terminates the sequence.
      typename ObjectVarNames::NestedVarSet m_nestedVarSet;

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
