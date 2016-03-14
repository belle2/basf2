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
      /// Constructure taking a optional prefix that can be attached to the names if request.
      explicit VarSet(const std::string& prefix = "") :
        m_superLayerCenters(),
        m_variables(prefix),
        m_nestedVarSet(prefix)
      {}

      /// Extract the nested variables next
      bool extractNested(const Object* obj)
      {
        return m_nestedVarSet.extract(AObjectVarNames::getNested(obj));
      }

      using Super::extract;

      /// Main method that extracts the variable values from the complex object.
      virtual
      bool extract(const Object* obj) override
      {
        return extractNested(obj);
      }

      /** Initialize the variable set before event processing.
       *  Can be specialised if the derived variable set has setup work to do.
       */
      virtual
      void initialize() override
      {
        m_nestedVarSet.initialize();
      }

      /** Terminate the variable set after event processing.
       *  Can be specialised if the derived variable set has to tear down aquired resources.
       */
      virtual
      void terminate() override
      {
        m_nestedVarSet.terminate();
      }

      /** Getter for the named tuples storing the values of all the (possibly nested) VarSets
       *  Base implementation returns the  named tuple of this variable set.
       */
      virtual
      std::vector<Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() override
      {
        std::vector<NamedFloatTuple*> result = m_nestedVarSet.getAllVariables();
        result.push_back(&m_variables);
        return result;
      }

      /** Const getter for the named tuples storing the values of all the (possibly nested)
       *  variable sets. Base implementation returns the named tuple of this variable set.
       */
      virtual
      std::vector<const Belle2::TrackFindingCDC::NamedFloatTuple*> getAllVariables() const override
      {
        std::vector<const NamedFloatTuple*> result = m_nestedVarSet.getAllVariables();
        result.push_back(&m_variables);
        return result;
      }

    protected:
      /// The cylinder radius of the super layer centers
      std::vector<double> m_superLayerCenters;

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
        if (std::isnan(value)) {
          var<I>() = valueIfNaN;
        } else {
          var<I>() = value;
        }
      }

      /// Prepare the superlayer center array with information coming from the CDCWireTopology.
      void prepareSuperLayerCenterArray()
      {
        const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

        m_superLayerCenters.clear();
        m_superLayerCenters.reserve(wireTopology.getNSuperLayers());

        for (const CDCWireSuperLayer& superLayer : wireTopology.getWireSuperLayers()) {
          Float_t superLayerCenter = superLayer.getMiddleCylindricalR();
          m_superLayerCenters.push_back(superLayerCenter);
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
