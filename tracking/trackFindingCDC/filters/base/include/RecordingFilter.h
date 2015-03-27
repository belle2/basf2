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
#include <tracking/trackFindingCDC/tmva/Recorder.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class VarSet>
    class RecordingFilter: public Filter<typename VarSet::Object> {

    public:
      /// Type of the object to be analysed.
      typedef typename VarSet::Object Object;

    public:
      /// Constructor of the filter.
      RecordingFilter(const std::string& defaultRootFileName = "records.root",
                      const std::string& defaultTreeName = "recorded_tree") :
        Filter<Object>(),
        m_recorder(nullptr),
        m_varset(),
        m_rootFileName(defaultRootFileName),
        m_treeName(defaultTreeName)
      {;}

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        m_varset.initialize();
        m_recorder.reset(new Recorder(m_varset.getAllVariables(), m_rootFileName, m_treeName));
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        m_recorder->write();
        m_recorder.reset();
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
        if (key == "root_file_name") {
          m_rootFileName = value;
          B2INFO("Filter received parameter 'root_file_name' = " << m_rootFileName);
        } else if (key == "tree_name") {
          m_treeName = value;
          B2INFO("Filter received parameter 'tree_name' = " << m_treeName);

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
        des["root_file_name"] = "Name of the ROOT file to be written.";
        des["tree_name"] = "Name of the Tree to be written.";
        return des;
      }
    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual CellWeight operator()(const Object& obj) override final
      {
        bool extracted = m_varset.extract(&obj);
        if (extracted) {
          m_recorder->capture();
        }
        return NOT_A_CELL;
      }

    private:
      /// Recorder to write all variable sets of the encountered objects.
      std::unique_ptr<Recorder> m_recorder;

      /// VarSet to generate the variables from the object
      VarSet m_varset;

      /// Name of the ROOT file to which shall be written.
      std::string m_rootFileName;

      /// Name of Tree to be written.
      std::string m_treeName;
    };
  }
}
