/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

// in fw:
#include <tracking/trackFindingVXD/analyzingTools/AnalizerTCInfo.h>
#include <tracking/trackFindingVXD/analyzingTools/AnalyzingAlgorithmFactory.h>
#include <tracking/trackFindingVXD/analyzingTools/KeyValBox.h>
// #include <tracking/trackFindingVXD/analyzingTools/algorithms/AnalyzingAlgorithmClusterBased.h> // TODO
#include <framework/logging/Logger.h>

// root:
#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>

// stl:
#include <string>
#include <vector>
#include <utility> // for pair

namespace Belle2 {


  /** Production notes for RootParameterTracker:
  *
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


  /** takes care of collecting data of track candidates and storing it to root branches. */
  class RootParameterTracker {
  protected:

    /** short-cut since strings are always used as keys here: */
    template<class ValueType> using StringKeyBox = KeyValBox<std::string, ValueType>;


    /** short-cut for typical algorithm-types used */
    template<class DataType> using AnalyzingAlgorithm = AnalyzingAlgorithmBase<DataType, AnalizerTCInfo, TVector3>;


    /** contains algorithms and its raw data to be streamed into ttrees.
     *
     * key of AlgoDataBox is tcType,
     * val is another box with:
     *  key is algorithmType
     *  val is pair:
     *   .first is the algorithm to be applied onto the AnalizerTCInfo-instances.
     *   .second is the data collected for .first to be streamed into ttrees. */
    template<class DataType> using AlgoDataBox =
      StringKeyBox<StringKeyBox<std::pair<AnalyzingAlgorithm<DataType>*, std::vector<DataType>*>>>;


    /** contains all the trees for rootFile-filling later-on.
    *
    * Key is tcTypeName as a string
    * Value are TTrees, one tree for each tcType
    */
    StringKeyBox<TTree*> m_treeBox;


    /** contains all algorithms and its data storing one double per TC */
    AlgoDataBox<double> m_algoDataDouble;


    /** contains all algorithms and its data storing one int per TC */
    AlgoDataBox<int> m_algoDataInt;


    /** contains all algorithms and its data storing one vector of double per TC */
    AlgoDataBox<std::vector<double>> m_algoDataVecDouble;


    /** stores pointer to file */
    TFile* m_file;


    /** checks if ttree for given tcTypeName exists and creates it if not. returns ttree for given tcType */
    TTree* prepareTTree(std::string tcTypeName)
    {
      if (m_file == nullptr) {
        B2FATAL("RootParameterTracker::prepareTTree(), Root file not initialized yet! Please call RootParameterTracker::initialize(...) first!");
      }
      m_file->cd();
      //   m_file->ls();

      ///make sure that tree exists:
      TTree** tree4tcType = m_treeBox.find(tcTypeName);
      if (tree4tcType == nullptr) {
        m_treeBox.push_back({
          tcTypeName,
          new TTree(tcTypeName.c_str(), (std::string("collects data collected to tcType ") + tcTypeName).c_str())
        }
                           );
        tree4tcType = m_treeBox.find(tcTypeName);
        B2WARNING("RootParameterTracker::prepareTTree: new tree for tcType " << tcTypeName << " created, m_treeBox has now " <<
                  m_treeBox.size() << " entries");
        return *tree4tcType;
      }

      B2WARNING("RootParameterTracker::prepareTTree: ttree for tcType " << tcTypeName << " is reused, m_treeBox has " << m_treeBox.size()
                << " entries");
      return *tree4tcType;
    }


    /** takes aTC with tcTypeName and applies the algorithms for it */
    void collectData4DoubleAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC);


    /** takes aTC with tcTypeName and applies the algorithms for it */
    void collectData4IntAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC);


    /** takes aTC with tcTypeName and applies the algorithms for it */
    void collectData4VecDoubleAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC);


  public:

    /** constructor setting standard values */
    RootParameterTracker() : m_file(nullptr) {}


    /** creates rootFile, first parameter is fileName, second one specifies how the file shall be treated. Valid values for fileTreatment: 'RECREATE' or 'UPDATE'.
    *
    * calling it more than once will result in a B2FATAL-message.
    */
    void initialize(std::string fileName, std::string fileTreatment)
    {
      if (fileTreatment != std::string("RECREATE") and fileTreatment != std::string("UPDATE")) {
        B2FATAL("RootParameterTracker::initialize(), specified fileTreatment is " << fileTreatment <<
                ", which is invalid, please read the documentation!");
      }

      if (m_file != nullptr) {
        B2FATAL("RootParameterTracker::initialize(), there was a file already linked to this ParameterTracker, which is invalid, please read the documentation!");
      }
      B2DEBUG(5, "RootParameterTracker::initialize(), given parameters are fileName/fileTreatment: " << fileName << "/" <<
              fileTreatment);
      m_file = new TFile(fileName.c_str(), fileTreatment.c_str()); // alternative: UPDATE
      m_file->ls();
    }


    /** relevant for all algorithms storing one double per TC:
    *
    * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
    void addParameters4DoubleAlgorithms(std::string tcTypeName, std::string algorithmName);


    /** relevant for all algorithms storing one int per TC:
     *
     * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
    void addParameters4IntAlgorithms(std::string tcTypeName, std::string algorithmName);


    /** relevant for all algorithms storing one vector of double per TC:
     *
     * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
    void addParameters4VecDoubleAlgorithms(std::string tcTypeName, std::string algorithmName);


    /** take vector and fill for each tcType stored in rootParameterTracker */
    void collectData(const std::vector<AnalizerTCInfo>& tcVector)
    {
      /** Production notes:
       * take internal container where all the requested parameters are tracked:
       * for currenttcType in internalAlgorithmContainer:
       *  for tc in tcVector
       *   if tc.tctype == currenttcType
       *    fill internalLinkContainer with result of applied algorithm
       * */
      B2DEBUG(15, "RootParameterTracker::collectData(), size of given tcVector is: " << tcVector.size());

      for (const AnalizerTCInfo& tc : tcVector) {
        std::string tcTypeName = TCType::getTypeName(tc.getType());
        B2DEBUG(50, "RootParameterTracker::collectData(), executing TC of type: " << tcTypeName);

        collectData4DoubleAlgorithms(tcTypeName, tc);
        collectData4IntAlgorithms(tcTypeName, tc);
        collectData4VecDoubleAlgorithms(tcTypeName, tc);
      } // looping over TCs
    }


    /** fills tree/branches with their stuff, clear intermediate results afterwards */
    void fillRoot()
    {
      B2DEBUG(15, "RootParameterTracker::fillRoot(), Executing " << m_treeBox.size() << " ttrees.");

      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
//       m_file->ls();

      for (auto& boxEntry : m_treeBox) {
        int nBytesWritten = boxEntry.second->Fill();
        B2DEBUG(20, "RootParameterTracker::fillRoot() ttree " << boxEntry.first << " got " << nBytesWritten << " Bytes written");
        // boxEntry.second->Print();
      }

      for (auto& algoData2tcType : m_algoDataDouble) {
        for (auto& algoPack : algoData2tcType.second) {
          algoPack.second.second->clear();
        }
      }

      for (auto& algoData2tcType : m_algoDataInt) {
        for (auto& algoPack : algoData2tcType.second) {
          algoPack.second.second->clear();
        }
      }

      for (auto& algoData2tcType : m_algoDataVecDouble) {
        for (auto& algoPack : algoData2tcType.second) {
          algoPack.second.second->clear();
        }
      }
    }


    /** final cleanup and closing rootFile */
    void terminate()
    {
      B2DEBUG(2, "RootParameterTracker::terminate(), Writing results to root-file and clean up heap.");
      if (m_file == nullptr) {
        B2WARNING("RootParameterTracker::terminate(): no rootFile found! skipping writing data into root file!");
        return;
      }

      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
      m_file->ls();

      for (auto& boxEntry : m_treeBox) {
        boxEntry.second->Write();

        if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 5, PACKAGENAME()) == true) {
          B2DEBUG(5, "RootParameterTracker::terminate(), TTree " << boxEntry.first << " was written:");
          // boxEntry.second->Print();
        }
      }
      m_file->Close();

      for (auto& algoData2tcType : m_algoDataDouble) {
        for (auto& algoPack : algoData2tcType.second) {
          delete algoPack.second.first; // algorithm
          delete algoPack.second.second; // vector(data) to algorithm
        }
      }

      for (auto& algoData2tcType : m_algoDataInt) {
        for (auto& algoPack : algoData2tcType.second) {
          delete algoPack.second.first; // algorithm
          delete algoPack.second.second; // vector(data) to algorithm
        }
      }

      for (auto& algoData2tcType : m_algoDataVecDouble) {
        for (auto& algoPack : algoData2tcType.second) {
          delete algoPack.second.first; // algorithm
          delete algoPack.second.second; // vector(data) to algorithm
        }
      }
    }

  };
}
