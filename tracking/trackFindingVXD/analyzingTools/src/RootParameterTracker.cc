/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingVXD/analyzingTools/RootParameterTracker.h>

// using namespace std;
using namespace Belle2;


/** for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
void RootParameterTracker::addParameters(std::string tcTypeName, std::string algorithmName)
{
  /** Production notes - pseudo code:
   *
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

  if (m_file == NULL) {
    B2WARNING("RootParameterTracker::addParameters(), Root file not initialized yet! Please call RootParameterTracker::initialize(...) first!")
    B2FATAL("RootParameterTracker::addParameters(), Root file not initialized yet! Please call RootParameterTracker::initialize(...) first!")
  }
  m_file->cd();
  m_file->ls();

  B2WARNING("RootParameterTracker::addParameters(), given parameters are tcTypeName/algorithmName: " << tcTypeName << "/" <<
            algorithmName)
  B2DEBUG(1, "RootParameterTracker::addParameters(), given parameters are tcTypeName/algorithmName: " << tcTypeName << "/" <<
          algorithmName)

  ///make sure that tree exists:
  TTree** tree4tcType = m_treeBox.find(tcTypeName);
  if (tree4tcType == NULL) {
    m_treeBox.push_back({
      tcTypeName,
      new TTree(tcTypeName.c_str(), (std::string("collects data collected to tcType ") + tcTypeName).c_str())
    }
                       );
    tree4tcType = m_treeBox.find(tcTypeName);
  }
  B2WARNING("RootParameterTracker::addParameters(), m_treeBox has " << m_treeBox.size() << " entries")

  /// make sure that container for algorithms of given tcType exists:
  auto* algorithms4tcType = m_algorithmBox.find(tcTypeName);
  if (algorithms4tcType == NULL) {
    AnalyzingAlgorithm<double>* newAlgorithm = AnalyzingAlgorithmFactory<double, AnalizerTCInfo, TVector3>(algorithmName);
    m_algorithmBox.push_back({
      tcTypeName,
      {newAlgorithm}
    }
                            );
    algorithms4tcType = m_algorithmBox.find(algorithmName);
  } else { // there is already a vector of algorithms for that
    std::vector<AnalyzingAlgorithm<double>* >::iterator foundPos = std::find_if(
          algorithms4tcType->begin(),
          algorithms4tcType->end(),
          [&](const AnalyzingAlgorithm<double>* anAlgorithm) -> bool
    { return algorithmName == anAlgorithm->getID(); }
        );
    if (foundPos == algorithms4tcType->end()) {
      AnalyzingAlgorithm<double>* newAlgorithm = AnalyzingAlgorithmFactory<double, AnalizerTCInfo, TVector3>(algorithmName);
      algorithms4tcType->push_back(newAlgorithm);
    } else {
      B2WARNING("RootParameterTracker::addParameters() given tcTypeName/algorithmName: " << tcTypeName <<
                "/" << algorithmName <<
                " was already added and will not be added again. This is a sign for unintended behavior - skipping addParameters()!")
      return;
    }
  }
  B2WARNING("RootParameterTracker::addParameters(), m_algorithmBox has " << m_algorithmBox.size() << " entries")

  /// make sure that there is a branch linked to the raw data stored for this tcType <-> algorithm combination
  auto* data4tcType = m_dataBox.find(tcTypeName); // contains all algorithms stored for given tcTypeName
  if (data4tcType == NULL) {
    B2WARNING("RootParameterTracker::addParameters(), tcType " << tcTypeName << " not yet added to m_dataBox, doing it now...")
    m_dataBox.push_back({
      tcTypeName,
      StringKeyBox<std::vector<double>*>()
    }
                       );
    data4tcType = m_dataBox.find(tcTypeName);
  }
  B2WARNING("RootParameterTracker::addParameters(), m_dataBox has " << m_dataBox.size() << " entries")
  B2WARNING("RootParameterTracker::addParameters(), data4tcType has " << data4tcType->size() << " entries (b4 data4algorithm-check)")
  auto** data4algorithm = data4tcType->find(algorithmName); // contains all calculated values stored for given algorithmName
  if (data4algorithm == NULL) {
    B2WARNING("RootParameterTracker::addParameters(), algorithm " << algorithmName << " not yet added to m_dataBox.find(" << tcTypeName
              << "), doing it now...")
    data4tcType->push_back({
      algorithmName,
      new std::vector<double>()
    }
                          );
    data4algorithm = data4tcType->find(algorithmName);
    auto* newBranch = (*tree4tcType)->Branch(algorithmName.c_str(), *data4algorithm);
    newBranch->Print();
  }
  B2WARNING("RootParameterTracker::addParameters(), data4tcType has " << data4tcType->size() <<
            " entries (after data4algorithm-check)")
  B2WARNING("RootParameterTracker::addParameters(), data4algorithm has " << (*data4algorithm)->size() << " entries")

  /// link data to the branch:
}
