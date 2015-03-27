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

#include "Filter.h"
#include <tracking/trackFindingCDC/tmva/Expert.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class VarSet>
    class TMVAFilter: public Filter<typename VarSet::Object> {

    public:
      /// Type of the object to be analysed.
      typedef typename VarSet::Object Object;

    public:
      /// Constructor of the filter.
      TMVAFilter(const std::string& defaultTrainingName = "") :
        Filter<Object>(),
        m_param_cut(NAN),
        m_expert("data/tracking", defaultTrainingName)
      {;}

      /// Initialize the expert before event processing.
      virtual void initialize() override
      {
        m_varset.initialize();
        m_expert.initializeReader(m_varset.getAllVariables());
      }

      /// Initialize the terminate after event processing.
      virtual void terminate() override
      {
        m_varset.terminate();
      }


      /** Set the parameter with key to values.
       *
       *  Parameters are:
       *  cut           - The cut value of the mva output below which the object is rejected
       *  weight_folder - The name of the folder to look for weight files from trainings.
       *  training_name - The name of the training that should be used for the prediction.
       */
      virtual void setParameter(const std::string& key, const std::string& value)
      {
        if (key == "cut") {
          m_param_cut = stod(value);
          B2INFO("Filter received parameter 'cut' = " << m_param_cut);
        } else if (key == "weight_folder") {
          m_expert.setWeightFolderName(value);
          B2INFO("Filter received parameter 'weight_folder' = " << m_expert.getWeightFolderName());
        } else if (key == "training_name") {
          m_expert.setTrainingName(value);
          B2INFO("Filter received parameter 'training_name' = " << m_expert.getTrainingName());

        } else {
          Filter<Object>::setParameter(key, value);
        }
      }

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual std::map<std::string, std::string> getParameterDescription()
      {
        std::map<std::string, std::string> des = Filter<Object>::getParameterDescription();
        des["cut"] =  "The cut value of the mva output below which the object is rejected.";
        des["folder_weight"] = "The name of the folder to look for weight files from trainings.";
        des["training_name"] = "The name of the training that should be used for the prediction.";
        return des;
      }
    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual CellWeight operator()(const Object& obj) override final
      {
        bool extracted = m_varset.extract(&obj);
        if (not extracted) return NOT_A_CELL;
        double prediction = m_expert.predict();
        return prediction < m_param_cut ? NOT_A_CELL : prediction;
      }

    private:
      /// The cut on the TMVA output.
      double m_param_cut;

      /// TMVA Expert to decide if a cluster is background or not.
      Expert m_expert;

      /// VarSet to generate the variables from the object
      VarSet m_varset;
    };
  }
}
