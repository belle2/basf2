/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.dcl.h>
#include <tracking/trackFindingCDC/filters/base/Filter.fwd.h>

#include <tracking/trackFindingCDC/varsets/BaseVarSet.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <memory>
#include <string>
#include <cmath>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class MVAExpert;

    /// Filter based on a mva method.
    template<class AFilter>
    class MVA : public OnVarSet<AFilter> {

    private:
      /// Type of the super class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    private:
      /// Type of the variable set
      using AVarSet = BaseVarSet<Object>;

    public:
      /// Constructor of the filter.
      explicit MVA(std::unique_ptr<AVarSet> varSet,
                   const std::string& identifier = "",
                   double defaultCut = NAN);

      /// Default destructor
      virtual ~MVA();

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Initialize the expert before event processing.
      void initialize() override;

      /// Signal to load new run parameters
      void beginRun() override;

    public:
      /// Function to object for its signalness
      Weight operator()(const Object& obj) override;

      /// Evaluate the mva method
      virtual double predict(const Object& obj);

    private:
      /// The cut on the MVA output.
      double m_param_cut;

      /// Database identifier of the expert or weight file name
      std::string m_param_identifier;

      /// MVA Expert to examine the object
      std::unique_ptr<MVAExpert> m_mvaExpert;
    };

    /// Convience template to create a mva filter for a set of variables.
    template<class AVarSet>
    class MVAFilter: public MVA<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      using Super = MVA<Filter<typename AVarSet::Object> >;

    public:
      /// Type of the object to be analysed.
      using Object = typename AVarSet::Object;

      /// Constructor of the filter.
      explicit MVAFilter(const std::string& defaultTrainingName = "",
                         double defaultCut = NAN);

      /// Default destructor
      ~MVAFilter();
    };
  }
}
