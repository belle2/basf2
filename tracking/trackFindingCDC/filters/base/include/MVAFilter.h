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

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/mva/MVAExpert.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AFilter>
    class MVA : public OnVarSet<AFilter> {

    private:
      /// Type of the super class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    public:
      /// Constructor of the filter.
      explicit MVA(std::unique_ptr<BaseVarSet<Object> > varSet,
                   const std::string& identifier = "",
                   double defaultCut = NAN) :
        Super(std::move(varSet)),
        m_param_cut(defaultCut),
        m_param_identifier(identifier)
      {}

      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override
      {
        Super::exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "cut"),
                                      m_param_cut,
                                      "The cut value of the mva output below which the object is rejected",
                                      m_param_cut);

        moduleParamList->addParameter(prefixed(prefix, "identifier"),
                                      m_param_identifier,
                                      "Database identfier of the expert of weight file name",
                                      m_param_identifier);

      }

      /// Initialize the expert before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        m_mvaExpert = std::unique_ptr<MVAExpert>(new MVAExpert(m_param_identifier));
        m_mvaExpert->initialize(Super::getVarSet().getNamedVariables());
      }

      /// Signal to load new run parameter
      virtual void beginRun() override
      {
        Super::beginRun();
        m_mvaExpert->initialize(Super::getVarSet().getNamedVariables());
      }

    public:
      /// Function to object for its signalness
      virtual Weight operator()(const Object& obj) override final
      {
        double prediction = predict(obj);
        return prediction < m_param_cut ? NAN : prediction;
      }

      /// Evaluate the tmva method
      virtual double predict(const Object& obj)
      {
        Weight extracted = Super::operator()(obj);
        if (std::isnan(extracted)) {
          return NAN;
        } else {
          return m_mvaExpert->predict();
        }
      }

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
                         double defaultCut = NAN)
        :  Super(std::unique_ptr<AVarSet> (new AVarSet()),
                 defaultTrainingName,
                 defaultCut)
      {}
    };
  }
}
