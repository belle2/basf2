/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// in fw:
#include <tracking/trackFindingVXD/analyzingTools/AnalizerTCInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/AnalyzingAlgorithmFactory.h>
#include <framework/logging/Logger.h>

// root:
#include <TTree.h>
#include <TFile.h>

// stl:
#include <string>
#include <vector>
#include <utility> // for pair




namespace Belle2 {

  /// maybe outdated draft for class down below. Kept for comparison later (maybe some things were forgotten to be implemented and stuff)
  //    /** Root data collector */
  //    template <class DataType>
  //    class RootDataCollector {
  //    protected:
  //    std::vector<DataType> m_collectedData;
  //    std::string m_name;
  //    AnalyzingAlgorithmBase m_calcAlgorithm;
  //
  //    public:
  //
  //    RootDataCollector(std::string name, AnalyzingAlgorithmBase<DataType> algorithm) : m_name(name), m_calcAlgorithm(algorithm) {}
  //
  //    std::string getName() { return m_name; }
  //
  //    std::vector<DataType>* linkToData() { return & m_collectedData; }
  //
  //    void reset4event() { m_collectedData.clear(); }
  //
  //    void addData(const AnalyzerTCInfo& refTC, const AnalyzerTCInfo& testTC) { m_collectedData.push_back(m_calcAlgorithm.calcData(refTC, testTC)); }
  //    };

  /** takes care of collecting data and storing it to root branches*/
  class RootParameterTracker {
  protected:

    /** stores pointer to file */
    TFile* m_file;
  public:
    /** constructor setting standard values */
    RootParameterTracker() : m_file(NULL) {}


    /**
     * internalTreeContainer, internalAlgorithmContainer, internalLinkContainer
     * are all a vector< pair < key, value> >.
     * .find returns a link to the entry (.second only) searched for and takes a key and compares only with the key (like a map)
     * .push_back simply push_back a pair
     * .clearAll() for each entry.second .clear();
     *
     * shall be a simple class just containing this vector and a find function doing what one wants to do
     *
     * reread pseudo code down below to be sure that the container does what it shall do.
     */

    /** creates rootFile, first parameter is fileName, second one specifies how the file shall be treated. Valid values for fileTreatment: 'RECREATE' or 'UPDATE' */
    void createFile(std::string fileName, std::string fileTreatment)
    {
      if (fileTreatment != std::string("RECREATE") and fileTreatment != std::string("UPDATE")) {
        B2FATAL("RootParameterTracker::createFile(), specified fileTreatment is " << fileTreatment <<
                ", which is invalid, please read the documentation!")
      }

      if (m_file != NULL) {
        B2FATAL("RootParameterTracker::createFile(), there was a file already linked to this ParameterTracker, which is invalid, please read the documentation!")
      }
      m_file = new TFile(fileName.c_str(), fileTreatment.c_str()); // alternative: UPDATE
    }


    // store in internal bla, prepare branch
    void addParameters(std::string tcTypeName, std::string algorithmName)
    {
      // TODO
      B2DEBUG(1, "RootParameterTracker::createFile(), given parameters are tcTypeName/algorithmName: " << tcTypeName << "/" <<
              algorithmName)
      /**
       * want to have one ttree for each tcType.
       *
       * if TCType.convertToString (tcTypeName) not in internalTreeContainer:
       *  internalTreeContainer.push_back(tcTypeName, new TTree(tcTypeName, ("collects data collected to tcType" + tcTypeName).c_str());
       *
       * if TCType.convertToString (tcTypeName) not in internalAlgorithmContainer:
       *  internalAlgorithmContainer.push_back(tcTypeName, {} ); // .second contains all the algorithms in the end
       *
       * if algorithmName already in internalAlgorithmContainer:
       *  B2WARNING
       *  return;
       *
       * internalAlgorithmContainer.find(tcTypeName).push_back(newAlgorithm);
       * internalLinkContainer.find(tcTypeName).push_back( { algorithmName, {} ); // .second (empty curly braces) contains the collected data in the end
       *
       * auto linkToData = internalLinkContainer.find(tcTypeName).find(algorithmName);
       *
       * internalTreeContainer.find(tcTypeName)->Branch("algorithmName", &linkToData);
       * */
    }


    // take vector and fill for each tcType stored in rootParameterTracker
    void prepareRoot(const std::vector<AnalizerTCInfo>& tcVector)
    {
      B2DEBUG(1, "RootParameterTracker::prepareRoot(), size of given tcVector is: " << tcVector.size())
      // TODO
      /**
       * take internal container where all the requested parameters are tracked:
       * for currenttcType in internalAlgorithmContainer:
       *  for tc in tcVector
       *   if tc.tctype == currenttcType
       *    fill internalLinkContainer with result of applied algorithm
       * */
    }

    /** fills tree/branches with their stuff, clear intermediate results afterwards */
    void fillRoot()
    {
      // TODO
      /**
       * for each tree:
       *  tree->Fill()
       *
       * for each link in internalLinkContainer: clearStuff
       * */
    }

    /** final cleanup and closing rootFile */
    void terminate()
    {
      // TODO
      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
      /**
       * for each tree:
       *  tree->Write()
       * */
      m_file->Close();
    }
  };
}
