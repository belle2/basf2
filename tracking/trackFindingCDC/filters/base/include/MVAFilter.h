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

#include <tracking/trackFindingCDC/utilities/ParamList.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// Filter based on a mva method.
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
      explicit MVA(std::unique_ptr<BaseVarSet<Object>> varSet,
                   const std::string& identifier = "",
                   double defaultCut = NAN)
        : Super(std::move(varSet))
        , m_param_cut(defaultCut)
        , m_param_identifier(identifier)
      {
      }

      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParams(ParamList* paramList, const std::string& prefix) override
      {
        Super::exposeParams(paramList, prefix);
        paramList->addParameter(prefixed(prefix, "cut"),
                                m_param_cut,
                                "The cut value of the mva output below which the object is rejected",
                                m_param_cut);

        paramList->addParameter(prefixed(prefix, "identifier"),
                                m_param_identifier,
                                "Database identfier of the expert of weight file name",
                                m_param_identifier);

      }

      /// Initialize the expert before event processing.
      void initialize() override
      {
        Super::initialize();
        m_mvaExpert = makeUnique<MVAExpert>(m_param_identifier,
                                            Super::getVarSet().getNamedVariables());
        m_mvaExpert->initialize();
      }

      /// Signal to load new run parameters
      void beginRun() override
      {
        Super::beginRun();
        m_mvaExpert->beginRun();
      }

    public:
      /// Function to object for its signalness
      Weight operator()(const Object& obj) override
      {
        double prediction = predict(obj);
        return prediction < m_param_cut ? NAN : prediction;
      }

      /// Evaluate the mva method
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
        :  Super(makeUnique<AVarSet>(),
                 defaultTrainingName,
                 defaultCut)
      {}
    };
  }
}
