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
#include <tracking/trackFindingVXD/analyzingTools/KeyValBox.h>
#include <framework/logging/Logger.h>

// root:
#include <TTree.h>
#include <TFile.h>
#include <TVector3.h>

// stl:
#include <string>
#include <vector>
#include <utility> // for pair
#include <algorithm> // for find_if




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

  /** takes care of collecting data and storing it to root branches. */
  class RootParameterTracker {
  protected:

    /** short-cut since strings are always used as keys here: */
    template<class ValueType> using StringKeyBox = KeyValBox<std::string, ValueType>;


    /** short-cut for typical algorithm-types used */
    template<class DataType> using AnalyzingAlgorithm = AnalyzingAlgorithmBase<DataType, AnalizerTCInfo, TVector3>;


    /** contains all the trees for rootFile-filling later-on */
    StringKeyBox<TTree*> m_treeBox;


    /** contains all the algorithms to be applied onto the AnalizerTCInfo-instances.
     *
     * key is tcType,
     * val collects all the algorithms to be applied on that tcType */
    StringKeyBox< std::vector<AnalyzingAlgorithm<double> > > m_algorithmBox;


    /** contains all the raw data to be streamed into ttrees.
     *
     * key of dataBox is tcType,
     * val is another box with:
     *  key is algorithmType
     *  val is pointer to data collected for that algorithm
     */
    StringKeyBox< StringKeyBox< std::vector<double>* > > m_dataBox;


    /** stores pointer to file */
    TFile* m_file;

  public:

    /** constructor setting standard values */
    RootParameterTracker() : m_file(NULL) {}


    /** creates rootFile, first parameter is fileName, second one specifies how the file shall be treated. Valid values for fileTreatment: 'RECREATE' or 'UPDATE'.
    *
    * calling it more than once will result in a B2FATAL-message.
    */
    void initialize(std::string fileName, std::string fileTreatment)
    {
      if (fileTreatment != std::string("RECREATE") and fileTreatment != std::string("UPDATE")) {
        B2FATAL("RootParameterTracker::initialize(), specified fileTreatment is " << fileTreatment <<
                ", which is invalid, please read the documentation!")
      }

      if (m_file != NULL) {
        B2FATAL("RootParameterTracker::initialize(), there was a file already linked to this ParameterTracker, which is invalid, please read the documentation!")
      }
      B2WARNING("RootParameterTracker::initialize(), given parameters are fileName/fileTreatment: " << fileName << "/" <<
                fileTreatment)
      m_file = new TFile(fileName.c_str(), fileTreatment.c_str()); // alternative: UPDATE
    }


    /** for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
    void addParameters(std::string tcTypeName, std::string algorithmName);


    /** take vector and fill for each tcType stored in rootParameterTracker */
    void prepareRoot(const std::vector<AnalizerTCInfo>& tcVector)
    {
      B2WARNING("RootParameterTracker::prepareRoot(), size of given tcVector is: " << tcVector.size())
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
      B2WARNING("RootParameterTracker::fillRoot() was called...")
      B2DEBUG(1, "RootParameterTracker::fillRoot() was called...")
      /** Production notes - pseudo code:
       * for each tree:
       *  tree->Fill()
       *
       * for each link in internalLinkContainer: clearStuff
       * */
      for (auto& boxEntry : m_treeBox) {
        boxEntry.second->Fill();
      }

      for (auto& data2tcType : m_dataBox) {
        for (auto& data2algorithm : data2tcType.second) {
          data2algorithm.second->clear();
        }
      }
    }


    /** final cleanup and closing rootFile */
    void terminate()
    {
      /** Production notes - pseudo code:
         * for each tree:
         *  tree->Write()
         * */
      if (m_file != NULL) {
        B2WARNING("RootParameterTracker::terminate(): no rootFile found! skipping writing data into root file!")
        return;
      }

      B2WARNING("RootParameterTracker::terminate(): b4 cd(); ")
      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module

      B2WARNING("RootParameterTracker::terminate(): b4 loop; ")
      for (auto& boxEntry : m_treeBox) {
        B2WARNING("RootParameterTracker::terminate(): b4 Write(); ")
        boxEntry.second->Write();
      }
      B2WARNING("RootParameterTracker::terminate(): b4 Close(); ")
      m_file->Close();
    }
  };
}
