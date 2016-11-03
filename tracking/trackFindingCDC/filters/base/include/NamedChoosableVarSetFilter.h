/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  Filter adapter to make a filter work on a set of variables and return
     *  on variable as the result of the filter.
     */
    template <class AFilter>
    class ChooseableFromVarSet : public OnVarSet<AFilter> {

    private:
      /// Type of the super class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the filtered object.
      using Object = typename AFilter::Object;

    private:
      /// Type of the variable set
      using AVarSet = BaseVarSet<Object>;

    public:
      /**
       *  Constructor taking the variable set the filter should work on and the  default name of the
       * variable to be used.
       */
      ChooseableFromVarSet(std::unique_ptr<AVarSet> varSet, std::string varName = "")
        : Super(std::move(varSet))
        , m_param_varName(varName)
      {
      }

      /// Add the parameters of this filter to the given parameter list
      virtual void exposeParameters(ModuleParamList* parameterList,
                                    const std::string& prefix = "") override
      {
        Super::exposeParameters(parameterList, prefix);

        if (m_param_varName == "") {
          // Make a forced parameter if no default variable name is present
          parameterList
          ->addParameter(prefixed(prefix, "chosenVariable"),
                         m_param_varName,
                         "Choose the name of the variable that will be put out as a weight.");
        } else {
          // Normal unforced parameter if default name is present
          parameterList
          ->addParameter(prefixed(prefix, "chosenVariable"),
                         m_param_varName,
                         "Choose the name of the variable that will be put out as a weight.",
                         m_param_varName);
        }
      }

      /// Initialisation method sets up a reference to the value in the variable set to be returned.
      virtual void initialize() override
      {
        Super::initialize();
        MayBePtr<Float_t> foundVariable = Super::getVarSet().find(m_param_varName);
        if (not foundVariable) {
          B2ERROR("Could not find request variable name " << m_param_varName << " in variable set");
          B2INFO("Valid names are: ");
          std::vector<Named<Float_t*>> namedVariables = Super::getVarSet().getNamedVariables();
          for (const Named<Float_t*>& namedVariable : namedVariables) {
            std::string name = namedVariable.getName();
            B2INFO("* " << name);
          }
        }
        m_variable = foundVariable;
      }

      /// Main filter function returning the variable with the set requested name from the variable
      /// set.
      virtual Weight operator()(const Object& object) override
      {
        Weight extracted = Super::operator()(object);
        if (std::isnan(extracted)) {
          return NAN;
        } else {
          if (m_variable) {
            return *m_variable;
          } else {
            return NAN;
          }
        }
      }

    private:
      /// Memory for the name of the variable selected as the return value of the filter.
      std::string m_param_varName;

      /// Reference to the location of the value in the variable set to be returned
      Float_t* m_variable = nullptr;
    };

    /// Convience template to create a filter returning on variable from a set of variables.
    template <class ATruthVarSet>
    class NamedChoosableVarSetFilter
      : public ChooseableFromVarSet<Filter<typename ATruthVarSet::Object>> {

    private:
      /// Type of the super class
      using Super = ChooseableFromVarSet<Filter<typename ATruthVarSet::Object>>;

    public:
      /// Type of the filtered object.
      using Object = typename ATruthVarSet::Object;

    public:
      /// Constructor
      NamedChoosableVarSetFilter()
        : Super(makeUnique<ATruthVarSet>())
      {
      }
    };
  }
}
