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

#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>
#include <tracking/trackFindingCDC/mva/TMVA.h>
#include <TMVA/Reader.h>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class to interact with TMVA.
    class TMVAExpert {
    public:
      /** Construct the Expert with the specified weight folder and
       *  the name of the training that was used in the teacher run.
       *  @param weightFolderName   Folder name where the weight file of the TMVA method
                                    should be sought.
       *  @param trainingName       Name of the training of the teacher run.
       */
      TMVAExpert(const std::string& weightFolderName, const std::string& trainingName);

      /// Evaluate the TMVA method and return the MVAOutput
      double predict();

      /** Add variable names and values addresses to the underlying tmva reader
       *  and initalize the method.
       */
      void initializeReader(std::vector<Named<Float_t*> > namedVariables);

      /// Evaluate the TMVA method and return the MVAOutput
      double useWeight() /*__attribute__((deprecated))*/
      { return predict(); }

      /** Add variable names and values addresses to the underlying tmva reader
       *  and initalize the method.
       *
       *  @param setReaderAddresses    Function invoked with the underlying reader which is supposed
       *                               to add the variable names and adresses.
       */
      void initializeReader(const std::function<void(TMVA::Reader&)>& setReaderAddresses);

      /// Setter for the folder of the weight files
      void setWeightFolderName(const std::string& weightFolderName)
      { m_weightFolderName = weightFolderName; }

      /// Getter for the folder of the weight files
      std::string getWeightFolderName()
      { return m_weightFolderName; }

      /// Setter for the training name
      void setTrainingName(const std::string& trainingName)
      { m_trainingName = trainingName; }

      /// Getter for the training name
      std::string getTrainingName()
      { return m_trainingName; }

      /// Getter for the file name associated with training name.
      std::string getWeightFileName()
      { return  getTrainingName() + "_FastBDT.weights.xml"; }

      /** Resolves the file name through the framework FileSystem::findFile()
       *  to a name in the local or central repostitory. Files relative to the current
       *  working directory are also valid.*/
      std::string getAbsWeightFilePath();

    private:
      /// Reader instance used to make the predictions.
      TMVA::Reader m_reader;

      /// Folder of the TMVA weight file.
      std::string m_weightFolderName;

      /// Name of the training that should be used
      std::string m_trainingName;
    };

  }
}
