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
#include <tracking/trackFindingCDC/tmva/Recorder.h>
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter based on a tmva method.
    template<class AVarSet>
    class RecordingFilter: public FilterOnVarSet<AVarSet> {

    private:
      /// Type of the super class
      typedef FilterOnVarSet<AVarSet> Super;

    public:
      /// Type of the object to be analysed.
      typedef typename AVarSet::Object Object;

    public:
      /// Constructor of the filter.
      RecordingFilter(const std::string& defaultRootFileName = "records.root",
                      const std::string& defaultTreeName = "records") :
        Super(),
        m_recorder(nullptr),
        m_rootFileName(defaultRootFileName),
        m_treeName(defaultTreeName),
        m_returnedCellWeight(NOT_A_CELL)
      {}

      /// Initialize the recorder before event processing.
      virtual void initialize() override
      {
        Super::initialize();
        AVarSet& varSet = Super::getVarSet();
        m_recorder.reset(new Recorder(varSet.getAllVariables(), m_rootFileName, m_treeName));
      }

      /// Initialize the recorder after event processing.
      virtual void terminate() override
      {
        m_recorder->write();
        m_recorder.reset();
        Super::terminate();
      }

      /** Set the parameter with key to values.
       *
       *  Parameters are:
       *  root_file_name = Name of the ROOT file to be written.
       *  tree_name -  Name of the Tree to be written.
       *  returned_cell_weight - CellWeight this filter should return when called. Defaults to NOT_A_CELL
       */
      virtual void setParameter(const std::string& key, const std::string& value) override
      {
        if (key == "root_file_name") {
          m_rootFileName = value;
          B2INFO("Filter received parameter 'root_file_name' = " << m_rootFileName);
        } else if (key == "tree_name") {
          m_treeName = value;
          B2INFO("Filter received parameter 'tree_name' = " << m_treeName);
        } else if (key == "returned_cell_weight") {
          m_returnedCellWeight = value == "NOT_A_CELL" ? NOT_A_CELL : std::stod(value);
          B2INFO("Filter received parameter 'returned_cell_weight' = " << m_returnedCellWeight);
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
        des["root_file_name"] = "Name of the ROOT file to be written";
        des["tree_name"] = "Name of the Tree to be written";
        des["returned_cell_weight"] = "CellWeight this filter should return when called. Defaults to NOT_A_CELL";
        return des;
      }
    public:
      /// Function to evaluate the cluster for its backgroundness.
      virtual CellWeight operator()(const Object& obj) override final
      {
        CellWeight extracted = Super::operator()(obj);
        if (not isNotACell(extracted)) {
          m_recorder->capture();
        }

        return m_returnedCellWeight;
      }

    private:
      /// Recorder to write all variable sets of the encountered objects.
      std::unique_ptr<Recorder> m_recorder;

      /// Name of the ROOT file to which shall be written.
      std::string m_rootFileName;

      /// Name of Tree to be written.
      std::string m_treeName;

      /// Returns CellWeight when this filter is called
      CellWeight m_returnedCellWeight;
    };
  }
}
