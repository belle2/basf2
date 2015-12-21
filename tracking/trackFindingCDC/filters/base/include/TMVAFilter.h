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
#include <tracking/trackFindingCDC/tmva/Expert.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AVarSet>
    class TMVAFilter: public FilterOnVarSet<AVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<AVarSet> Super;

    public:
      /// Type of the object to be analysed.
      typedef typename AVarSet::Object Object;

    public:
      /// Constructor of the filter.
      explicit TMVAFilter(const std::string& defaultTrainingName = "") :
        Super(),
        m_param_cut(NAN),
        m_param_weightFolder("data/tracking"),
        m_param_trainingName(defaultTrainingName),
        m_expert("data/tracking", defaultTrainingName)
      {}

      /** Expose the set of parameters of the filter to the module parameter list.
       *
       *  Note that not all filters have yet exposed their parameters in this way.
       */
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "")
      {
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
        AVarSet& varSet = Super::getVarSet();
        m_expert.setWeightFolderName(m_param_weightFolder);
        m_expert.setTrainingName(m_param_trainingName);
        m_expert.initializeReader(varSet.getAllVariables());
      }

      /** Set the parameter with key to values.
       *
       *  Parameters are:
       *  cut           - The cut value of the mva output below which the object is rejected
       *  weight_folder - The name of the folder to look for weight files from trainings.
       *  training_name - The name of the training that should be used for the prediction.
       */
      virtual void setParameter(const std::string& key, const std::string& value) override
      {
        if (key == "cut") {
          m_param_cut = stod(value);
          B2INFO("Filter received parameter 'cut' = " << m_param_cut);
        } else if (key == "weightFolder") {
          m_param_weightFolder = value;
          B2INFO("Filter received parameter 'weight_folder' = " << m_expert.getWeightFolderName());
        } else if (key == "trainingName") {
          m_param_trainingName = value;
          B2INFO("Filter received parameter 'training_name' = " << m_expert.getTrainingName());
        } else {
          Super::setParameter(key, value);
        }
      }

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual std::map<std::string, std::string> getParameterDescription() override
      {
        std::map<std::string, std::string> des = Super::getParameterDescription();
        des["cut"] =  "The cut value of the mva output below which the object is rejected";
        des["weightFolder"] = "The name of the folder to look for weight files from trainings";
        des["trainingName"] = "The name of the training that should be used for the prediction";
        return des;
      }
    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual Weight operator()(const Object& obj) override
      {
        Weight extracted = Super::operator()(obj);
        if (std::isnan(extracted)) {
          return NAN;
        } else {
          double prediction = m_expert.predict();
          return prediction < m_param_cut ? NAN : prediction;
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
      Expert m_expert;
    };
  }
}
