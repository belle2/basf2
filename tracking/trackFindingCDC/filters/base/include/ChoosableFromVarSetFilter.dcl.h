/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <RtypesCore.h>

#include <string>
#include <memory>

namespace Belle2 {
  class ModuleParamList;
  namespace TrackFindingCDC {

    /**
     *  Filter adapter to make a filter work on a set of variables and return
     *  on variable as the result of the filter.
     */
    template <class AFilter>
    class ChoosableFromVarSet : public OnVarSet<AFilter> {

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
       *  Constructor taking the variable set the filter should work on and the default name of the
       *  variable to be used.
       */
      ChoosableFromVarSet(std::unique_ptr<AVarSet> varSet, std::string varName = "");

      /// Default destructor
      ~ChoosableFromVarSet();

      /// Add the parameters of this filter to the given parameter list
      void exposeParameters(ModuleParamList* parameterList, const std::string& prefix) override;

      /// Initialisation method sets up a reference to the value in the variable set to be returned.
      void initialize() override;

      /// Returns the variable with the set requested name from the variable  set.
      Weight operator()(const Object& object) override;

    private:
      /// Memory for the name of the variable selected as the return value of the filter.
      std::string m_param_varName;

      /// Reference to the location of the value in the variable set to be returned
      Float_t* m_variable = nullptr;
    };

    /// Convience template to create a filter returning on variable from a set of variables.
    template <class ATruthVarSet>
    class ChoosableFromVarSetFilter
      : public ChoosableFromVarSet<Filter<typename ATruthVarSet::Object>> {

    private:
      /// Type of the super class
      using Super = ChoosableFromVarSet<Filter<typename ATruthVarSet::Object>>;

    public:
      /// Type of the filtered object.
      using Object = typename ATruthVarSet::Object;

    public:
      /// Constructor
      ChoosableFromVarSetFilter(const std::string& varName = "");

      /// Default destructor
      ~ChoosableFromVarSetFilter();
    };
  }
}
