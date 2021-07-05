/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingVXD/analyzingTools/RootParameterTracker.h>

using namespace std;
using namespace Belle2;


/** takes aTC with tcTypeName and applies the algorithms for it */
void RootParameterTracker::collectData4DoubleAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC)
{
  auto* foundTCTypeData = m_algoDataDouble.find(tcTypeName); // a KeyValBox with all algorithms and data collected to given tcTypeName

  // skip if there is nothing stored for this tcType:
  if (foundTCTypeData == nullptr) { return; }

  // looping over algorithms:
  for (auto& entry : *foundTCTypeData) {
    // increase readability:
    auto* anAlgorithm = entry.second.first;
    auto* dataVector = entry.second.second;
    // sanity check: key has to be compatible with stored algorithm:
    string algoName = anAlgorithm->getIDName();
    if (entry.first != algoName) {
      B2ERROR("RootParameterTracker::collectData4DoubleAlgorithms() key (" << entry.first <<
              ") of container does not match to its content (" << algoName <<
              ") - skipping entry! ");
      continue;
    }
    B2DEBUG(50, "RootParameterTracker::collectData4DoubleAlgorithms(), executing algorithm of type: " << algoName <<
            " with collected data-entries of " << dataVector->size());

    try {
      double calcVal = anAlgorithm->calcData(aTC);
      dataVector->push_back(calcVal);
      B2DEBUG(20, "RootParameterTracker::collectData4DoubleAlgorithms(), tc with type " << tcTypeName <<
              " and applied algorithm " << algoName <<
              " and got " << calcVal << " as a result!");
    }  catch (AnalyzingAlgorithm<double>::No_refTC_Attached& anException) {
      B2WARNING("RootParameterTracker::collectData4DoubleAlgorithms(), Exception caught for tc with type " << tcTypeName <<
                " and applied algorithm " << algoName <<
                ". Failed with exception: " << anException.what() <<
                " -> skipping tc!");
    }
  }// looping over algorithms
}





/** takes aTC with tcTypeName and applies the algorithms for it */
void RootParameterTracker::collectData4IntAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC)
{
  auto* foundTCTypeData = m_algoDataInt.find(tcTypeName); // a KeyValBox with all algorithms and data collected to given tcTypeName

  // skip if there is nothing stored for this tcType:
  if (foundTCTypeData == nullptr) { return; }

  // looping over algorithms:
  for (auto& entry : *foundTCTypeData) {
    // increase readability:
    auto* anAlgorithm = entry.second.first;
    auto* dataVector = entry.second.second;
    // sanity check: key has to be compatible with stored algorithm:
    string algoName = anAlgorithm->getIDName();
    if (entry.first != algoName) {
      B2ERROR("RootParameterTracker::collectData4DoubleAlgorithms() key (" << entry.first <<
              ") of container does not match to its content (" << algoName <<
              ") - skipping entry! ");
      continue;
    }
    B2DEBUG(50, "RootParameterTracker::collectData4DoubleAlgorithms(), executing algorithm of type: " << algoName <<
            " with collected data-entries of " << dataVector->size());

    try {
      int calcVal = anAlgorithm->calcData(aTC);
      dataVector->push_back(calcVal);
      B2DEBUG(20, "RootParameterTracker::collectData4DoubleAlgorithms(), tc with type " << tcTypeName <<
              " and applied algorithm " << algoName <<
              " and got " << calcVal << " as a result!");
    } catch (AnalyzingAlgorithm<int>::No_refTC_Attached& anException) {
      B2WARNING("RootParameterTracker::collectData4DoubleAlgorithms(), Exception caught for tc with type " << tcTypeName <<
                " and applied algorithm " << algoName <<
                ". Failed with exception: " << anException.what() <<
                " -> skipping tc!");
    }
  }// looping over algorithms
}





/** takes aTC with tcTypeName and applies the algorithms for it */
void RootParameterTracker::collectData4VecDoubleAlgorithms(std::string tcTypeName, const AnalizerTCInfo& aTC)
{
  auto* foundTCTypeData = m_algoDataVecDouble.find(
                            tcTypeName); // a KeyValBox with all algorithms and data collected to given tcTypeName

  // skip if there is nothing stored for this tcType:
  if (foundTCTypeData == nullptr) { return; }

  // looping over algorithms:
  for (auto& entry : *foundTCTypeData) {
    // increase readability:
    auto* anAlgorithm = entry.second.first;
    auto* dataVector = entry.second.second; // vector< vector < double>>
    // sanity check: key has to be compatible with stored algorithm:
    string algoName = anAlgorithm->getIDName();
    if (entry.first != algoName) {
      B2ERROR("RootParameterTracker::collectData4VecDoubleAlgorithms() key (" << entry.first <<
              ") of container does not match to its content (" << algoName <<
              ") - skipping entry! ");
      continue;
    }
    B2DEBUG(50, "RootParameterTracker::collectData4VecDoubleAlgorithms(), executing algorithm of type: " << algoName <<
            " with collected data-entries of " << dataVector->size());

    try {
      vector<double> calcVal = anAlgorithm->calcData(aTC);
      dataVector->push_back(calcVal);
      auto printVec = [&]() -> string {
        string out;
        for (double val : calcVal)
        {
          out += (" " + to_string(val));
        }
        return (out += "\n");
      };
      B2DEBUG(20, "RootParameterTracker::collectData4VecDoubleAlgorithms(), tc with type " << tcTypeName <<
              " and applied algorithm " << algoName <<
              " and got: " << printVec() <<
              " as a result!");
    }  catch (AnalyzingAlgorithm<vector<double>>::No_refTC_Attached& anException) {
      B2WARNING("RootParameterTracker::collectData4VecDoubleAlgorithms(), Exception caught for tc with type " << tcTypeName <<
                " and applied algorithm " << algoName <<
                ". Failed with exception: " << anException.what() <<
                " -> skipping tc!");
    }
  }// looping over algorithms
}





/** relevant for all algorithms storing one double per TC:
 *
 * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
void RootParameterTracker::addParameters4DoubleAlgorithms(std::string tcTypeName, std::string algorithmName)
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
  B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), given parameters are tcTypeName/algorithmName: " << tcTypeName
          << "/" <<
          algorithmName);

  TTree* tree4tcType = prepareTTree(tcTypeName);

/// make sure that container for algorithms of given tcType exists:
  auto* algorithms4tcType = m_algoDataDouble.find(tcTypeName);
  if (algorithms4tcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), tcType " << tcTypeName <<
            " not yet added to m_algoDataDouble, doing it now...");
    m_algoDataDouble.push_back({
      tcTypeName,
      StringKeyBox<pair<AnalyzingAlgorithm<double>*, vector<double>*> >()
    });
    algorithms4tcType = m_algoDataDouble.find(tcTypeName);
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), m_algoDataDouble has " << m_algoDataDouble.size() <<
          " tcTypes stored");

/// make sure that algorithm and its dataStuff exists:
  auto* data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);
  if (data4AlgorithmOftcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), algorithm " << algorithmName <<
            " not yet added to m_algoDataDouble[tcType], doing it now...");
    AnalyzingAlgorithm<double>* newAlgorithm = AnalyzingAlgorithmFactoryDouble<double, AnalizerTCInfo, TVector3>
                                               (AlgoritmType::getTypeEnum(algorithmName));
    algorithms4tcType->push_back({
      algorithmName,
      {
        newAlgorithm,
        new vector<double>()
      }
    });
    data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);

    /// make sure that there is a branch linked to the raw data stored for this tcType <-> algorithm combination:
    auto* newBranch = tree4tcType->Branch(algorithmName.c_str(), data4AlgorithmOftcType->second);
    if (newBranch == nullptr) B2ERROR("Could not create Branch " << algorithmName); // mainly to suppress compiler warning
    // newBranch->Print();
  } else {
    B2WARNING("RootParameterTracker::addParameters4DoubleAlgorithms() given tcTypeName/algorithmName: " << tcTypeName <<
              "/" << algorithmName <<
              " was already added and will not be added again. This is a sign for unintended behavior - nothing will be added!");
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), m_algoDataDouble[tcType] has " << algorithms4tcType->size() <<
          " algorithms stored");
}





/** relevant for all algorithms storing one int per TC:
 *
 * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
void RootParameterTracker::addParameters4IntAlgorithms(std::string tcTypeName, std::string algorithmName)
{
  B2DEBUG(5, "RootParameterTracker::addParameters4IntAlgorithms(), given parameters are tcTypeName/algorithmName: " << tcTypeName <<
          "/" <<
          algorithmName);

  TTree* tree4tcType = prepareTTree(tcTypeName);

  /// make sure that container for algorithms of given tcType exists:
  auto* algorithms4tcType = m_algoDataInt.find(tcTypeName);
  if (algorithms4tcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4IntAlgorithms(), tcType " << tcTypeName <<
            " not yet added to m_algoDataInt, doing it now...");
    m_algoDataInt.push_back({
      tcTypeName,
      StringKeyBox<pair<AnalyzingAlgorithm<int>*, vector<int>*> >()
    });
    algorithms4tcType = m_algoDataInt.find(tcTypeName);
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4IntAlgorithms(), m_algoDataInt has " << m_algoDataInt.size() << " tcTypes stored");

  /// make sure that algorithm and its dataStuff exists:
  auto* data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);
  if (data4AlgorithmOftcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4IntAlgorithms(), algorithm " << algorithmName <<
            " not yet added to m_algoDataInt[tcType], doing it now...");
    AnalyzingAlgorithm<int>* newAlgorithm = AnalyzingAlgorithmFactoryInt<int, AnalizerTCInfo, TVector3>(AlgoritmType::getTypeEnum(
                                              algorithmName));
    algorithms4tcType->push_back({
      algorithmName,
      {
        newAlgorithm,
        new vector<int>()
      }
    });
    data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);

    /// make sure that there is a branch linked to the raw data stored for this tcType <-> algorithm combination:
    auto* newBranch = tree4tcType->Branch(algorithmName.c_str(), data4AlgorithmOftcType->second);
    if (newBranch == nullptr) B2ERROR("Could not create Branch " << algorithmName); // mainly to suppress compiler warning
    // newBranch->Print();
  } else {
    B2WARNING("RootParameterTracker::addParameters4DoubleAlgorithms() given tcTypeName/algorithmName: " << tcTypeName <<
              "/" << algorithmName <<
              " was already added and will not be added again. This is a sign for unintended behavior - nothing will be added!");
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4DoubleAlgorithms(), m_algoDataInt[tcType] has " << algorithms4tcType->size() <<
          " algorithms stored");
}





/** relevant for all algorithms storing one vector of double per TC:
 *
 * for given tcTypename <-> algorithmName-combination, this tracker will be prepared for tracking them in a root-file. */
void RootParameterTracker::addParameters4VecDoubleAlgorithms(std::string tcTypeName, std::string algorithmName)
{
  B2DEBUG(5, "RootParameterTracker::addParameters4VecDoubleAlgorithms(), given parameters are tcTypeName/algorithmName: " <<
          tcTypeName << "/" <<
          algorithmName);

  TTree* tree4tcType = prepareTTree(tcTypeName);

  /// make sure that container for algorithms of given tcType exists:
  auto* algorithms4tcType = m_algoDataVecDouble.find(tcTypeName);
  if (algorithms4tcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4VecDoubleAlgorithms(), tcType " << tcTypeName <<
            " not yet added to m_algoDataVecDouble, doing it now...");
    m_algoDataVecDouble.push_back({
      tcTypeName,
      StringKeyBox<pair<AnalyzingAlgorithm<vector<double>>*, vector<vector<double>>*> >()
    });
    algorithms4tcType = m_algoDataVecDouble.find(tcTypeName);
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4VecDoubleAlgorithms(), m_algoDataVecDouble has " << m_algoDataVecDouble.size() <<
          " tcTypes stored");

  /// make sure that algorithm and its dataStuff exists:
  auto* data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);
  if (data4AlgorithmOftcType == nullptr) {
    B2DEBUG(5, "RootParameterTracker::addParameters4VecDoubleAlgorithms(), algorithm " << algorithmName <<
            " not yet added to m_algoDataVecDouble[tcType], doing it now...");
    AnalyzingAlgorithm<vector<double>>* newAlgorithm = AnalyzingAlgorithmFactoryVecDouble<vector<double>, AnalizerTCInfo, TVector3>
                                                       (AlgoritmType::getTypeEnum(algorithmName));
    algorithms4tcType->push_back({
      algorithmName,
      {
        newAlgorithm,
        new vector<vector<double>>()
      }
    });
    data4AlgorithmOftcType = algorithms4tcType->find(algorithmName);

    /// make sure that there is a branch linked to the raw data stored for this tcType <-> algorithm combination:
    auto* newBranch = tree4tcType->Branch(algorithmName.c_str(), data4AlgorithmOftcType->second);
    if (newBranch == nullptr) B2ERROR("Could not create Branch " << algorithmName); // mainly to suppress compiler warning
    // newBranch->Print();
  } else {
    B2WARNING("RootParameterTracker::addParameters4VecDoubleAlgorithms() given tcTypeName/algorithmName: " << tcTypeName <<
              "/" << algorithmName <<
              " was already added and will not be added again. This is a sign for unintended behavior - nothing will be added!");
  }
  B2DEBUG(5, "RootParameterTracker::addParameters4VecDoubleAlgorithms(), m_algoDataVecDouble[tcType] has " <<
          algorithms4tcType->size() << " algorithms stored");
}
