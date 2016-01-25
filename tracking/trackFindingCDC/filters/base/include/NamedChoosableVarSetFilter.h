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

namespace Belle2 {
  namespace TrackFindingCDC {
    /// MC Filter Type using a VarSet and the truth variable in it.
    template<class ATruthVarSet>
    class NamedChoosableVarSetFilter : public FilterOnVarSet<ATruthVarSet> {

    public:
      /// Type of the super class
      typedef FilterOnVarSet<ATruthVarSet> Super;
      typedef typename ATruthVarSet::Object Object;

    public:
      /// Constructor
      NamedChoosableVarSetFilter() : Super() { }

      /// Reject an item if the truth variable is 0, else accept it.
      virtual Weight operator()(const Object& object) override
      {
        Super::operator()(object);
        const std::map<std::string, Float_t>& varSet = Super::getVarSet().getNamedValuesWithPrefix();

        return varSet.at(m_chosenVariable);
      }

      virtual void exposeParameters(ModuleParamList* parameterList, const std::string& prefix = "")
      {
        parameterList->addParameter(prefix + "chosenVariable", m_chosenVariable,
                                    "Choose the name of the variable that will be put out as a weight.");
      }

    private:
      std::string m_chosenVariable;
    };
  }
}
