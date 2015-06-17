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


  /** takes care of collecting data and storing it to root branches. */
  class RootParameterTracker {
  protected:

    /** short-cut since strings are always used as keys here: */
    template<class ValueType> using StringKeyBox = KeyValBox<std::string, ValueType>;


    /** short-cut for typical algorithm-types used */
    template<class DataType> using AnalyzingAlgorithm = AnalyzingAlgorithmBase<DataType, AnalizerTCInfo, TVector3>;


    /** contains all the trees for rootFile-filling later-on.
    *
    * Key is tcTypeName as a string
    * Value are TTrees, one tree for each tcType
    */
    StringKeyBox<TTree*> m_treeBox;


    /** contains all the algorithms to be applied onto the AnalizerTCInfo-instances.
     *
     * key is tcType,
     * val collects all the algorithms to be applied on that tcType */
    StringKeyBox< std::vector<AnalyzingAlgorithm<double>* > > m_algorithmBox;


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
      m_file->ls();
    }


    /** for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
    void addParameters(std::string tcTypeName, std::string algorithmName);


    /** take vector and fill for each tcType stored in rootParameterTracker */
    void collectData(const std::vector<AnalizerTCInfo>& tcVector)
    {
      B2WARNING("RootParameterTracker::collectData(), size of given tcVector is: " << tcVector.size())
      B2DEBUG(1, "RootParameterTracker::collectData(), size of given tcVector is: " << tcVector.size())

      for (const AnalizerTCInfo& tc : tcVector) {
        std::string tcTypeName = TCType().getTypeName(tc.getType());
        B2DEBUG(1, "RootParameterTracker::collectData(), executing TC of type: " << tcTypeName)
        auto* foundAlgorithms = m_algorithmBox.find(tcTypeName);

        // skip if there are no algorithms stored for this tcType:
        if (foundAlgorithms == NULL) { continue; }

        auto* foundData = m_dataBox.find(tcTypeName); // a KeyValBox with all data collected to given tcTypeName

        if (foundData == NULL) {
          B2ERROR("RootParameterTracker::collectData(), while algorithms exist for tcType " << tcTypeName <<
                  ", there is no data linked to same type. This is a sign for unintended behavior! Skipping TC...")
          continue;
        }

        B2DEBUG(1, "RootParameterTracker::collectData(), foundAlgorithms-size: " << foundAlgorithms->size())

        // loop over algorithms to collect their data:
        for (auto* anAlgorithm : *foundAlgorithms) {
          std::string algoName = anAlgorithm->getID();
          B2DEBUG(1, "RootParameterTracker::collectData(), executing algorithm of type: " << algoName)

          std::vector<double>** data4algorithm = foundData->find(algoName);

          if (data4algorithm == NULL) {
            B2ERROR("RootParameterTracker::collectData(), there is no data linked to algorithm " << algoName <<
                    ", although algorithm exist. This is a sign for unintended behavior! Skipping TC...")
            continue;
          }

          B2DEBUG(1, "RootParameterTracker::collectData(),  data4algorithm-size: " << (*data4algorithm)->size())

          if (tc.assignedTC == NULL) {
            B2ERROR("RootParameterTracker::collectData(), second tc is missing, can not do anything! TODO that has to be fixed!");
            continue;
          } else {
            double calcVal = anAlgorithm->calcData(tc, *tc.assignedTC);
            (*data4algorithm)->push_back(calcVal);
            B2WARNING("RootParameterTracker::collectData(), tc with type " << tcTypeName <<
                      " applied algorithm " << algoName <<
                      " and got " << calcVal << " as a result!")
          }
        }

      }
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
      B2DEBUG(1, "RootParameterTracker::fillRoot() was called. Executing " << m_treeBox.size() << " ttrees.")

      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module

      m_file->ls();

      for (auto& boxEntry : m_treeBox) {
        int nBytesWritten = boxEntry.second->Fill();
        B2DEBUG(1, "RootParameterTracker::fillRoot() ttree " << boxEntry.first << " got " << nBytesWritten << " Bytes written")
        boxEntry.second->Print();
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
      if (m_file == NULL) {
        B2WARNING("RootParameterTracker::terminate(): no rootFile found! skipping writing data into root file!")
        return;
      }

      m_file->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
      m_file->ls();

      for (auto& boxEntry : m_treeBox) {
        boxEntry.second->Write();
        boxEntry.second->Print();
      }
      m_file->Close();

      for (auto& vecOfAlgorithms : m_algorithmBox) {
        for (auto* algoPtr : vecOfAlgorithms.second) {
          delete algoPtr;
        }
      }
    }

  };
}
