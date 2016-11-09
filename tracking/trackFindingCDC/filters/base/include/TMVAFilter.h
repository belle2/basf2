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

#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>
#include <tracking/trackFindingCDC/mva/TMVAExpert.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AFilter>
    class TMVA : public OnVarSet<AFilter> {

    private:
      /// Type of the super class
      using Super = OnVarSet<AFilter>;

    public:
      /// Type of the object to be analysed.
      using Object = typename AFilter::Object;

    public:
      /// Constructor of the filter.
      explicit TMVA(std::unique_ptr<BaseVarSet<Object> > varSet,
                    const std::string& defaultTrainingName = "",
                    double defaultCut = NAN) :
        Super(std::move(varSet)),
        m_param_cut(defaultCut),
        m_param_weightFolder("tracking/data"),
        m_param_trainingName(defaultTrainingName),
        m_tmvaExpert(nullptr)
      {}

      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
        moduleParamList->addParameter(prefixed(prefix, "cut"),
                                      m_param_cut,
                                      "The cut value of the mva output below which the object is rejected",
                                      m_param_cut);

        moduleParamList->addParameter(prefixed(prefix, "weightFolder"),
                                      m_param_weightFolder,
                                      "The name of the folder to look for weight files from trainings",
                                      m_param_weightFolder);

        moduleParamList->addParameter(prefixed(prefix, "trainingName"),
                                      m_param_trainingName,
                                      "The name of the training that should be used for the prediction",
                                      m_param_trainingName);
      }

      /// Initialize the expert before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        m_tmvaExpert = makeUnique<TMVAExpert>(m_param_weightFolder, m_param_trainingName);
        m_tmvaExpert->initializeReader(Super::getVarSet().getNamedVariables());
      }

    public:
      /// Function to object for its signalness
      virtual Weight operator()(const Object& obj) override
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
          double prediction = m_tmvaExpert->predict();
          return prediction;
        }
      }

    private:
      /// The cut on the TMVA output.
      double m_param_cut;

      /// The name of the folder to look for weight files from trainings
      std::string m_param_weightFolder;

      /// The name of the training that should be used for the prediction
      std::string m_param_trainingName;

      /// TMVA Expert to examine the object
      std::unique_ptr<TMVAExpert> m_tmvaExpert;
    };

    /// Convience template to create a tmva filter for a set of variables.
    template<class AVarSet>
    class TMVAFilter: public TMVA<Filter<typename AVarSet::Object> > {

    private:
      /// Type of the super class
      using Super = TMVA<Filter<typename AVarSet::Object> >;

    public:
      /// Type of the object to be analysed.
      using Object = typename AVarSet::Object;

      /// Constructor of the filter.
      explicit TMVAFilter(const std::string& defaultTrainingName = "",
                          double defaultCut = NAN)
        :  Super(makeUnique<AVarSet>(),
                 defaultTrainingName,
                 defaultCut)
      {}
    };
  }
}
