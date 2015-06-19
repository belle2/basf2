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
    B2FATAL("RootParameterTracker::addParameters(), Root file not initialized yet! Please call RootParameterTracker::initialize(...) first!")
  }
  m_file->cd();
//   m_file->ls();

  B2DEBUG(5, "RootParameterTracker::addParameters(), given parameters are tcTypeName/algorithmName: " << tcTypeName << "/" <<
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
  B2DEBUG(5, "RootParameterTracker::addParameters(), m_treeBox has " << m_treeBox.size() << " entries")

/// make sure that container for algorithms of given tcType exists:
  auto* algorithms4tcType = m_algoDataBox.find(tcTypeName);
  if (algorithms4tcType == NULL) {
    B2DEBUG(5, "RootParameterTracker::addParameters(), tcType " << tcTypeName << " not yet added to m_algoDataBox, doing it now...")
    m_algoDataBox.push_back({
      tcTypeName,
      StringKeyBox<std::pair<AnalyzingAlgorithm<double>*, std::vector<double>*> >()
    });
    algorithms4tcType = m_algoDataBox.find(tcTypeName);
  }
  B2DEBUG(5, "RootParameterTracker::addParameters(), m_algoDataBox has " << m_algoDataBox.size() << " tcTypes stored")

/// make sure that algorithm and its dataStuff exists:
  auto* data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);
  if (data4AlgorithmOftcType == NULL) {
    B2DEBUG(5, "RootParameterTracker::addParameters(), algorithm " << algorithmName <<
            " not yet added to m_algoDataBox[tcType], doing it now...")
    AnalyzingAlgorithm<double>* newAlgorithm = AnalyzingAlgorithmFactory<double, AnalizerTCInfo, TVector3>(algorithmName);
    algorithms4tcType->push_back({
      algorithmName,
      {
        newAlgorithm,
        new std::vector<double>()
      }
    });
    data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);

/// make sure that there is a branch linked to the raw data stored for this tcType <-> algorithm combination:
    auto* newBranch = (*tree4tcType)->Branch(algorithmName.c_str(), data4AlgorithmOftcType->second);
    newBranch->Print();
  } else {
    B2WARNING("RootParameterTracker::addParameters() given tcTypeName/algorithmName: " << tcTypeName <<
              "/" << algorithmName <<
              " was already added and will not be added again. This is a sign for unintended behavior - nothing will be added!")
  }
  B2DEBUG(5, "RootParameterTracker::addParameters(), m_algoDataBox[tcType] has " << algorithms4tcType->size() << " algorithms stored")
}
