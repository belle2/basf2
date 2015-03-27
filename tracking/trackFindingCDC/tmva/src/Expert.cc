/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/tmva/Expert.h>
#include <framework/utilities/FileSystem.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Expert::Expert(const std::string& weightFolderName, const std::string& trainingName) :
  m_reader("!Silent"),
  m_weightFolderName(weightFolderName),
  m_trainingName(trainingName)
{
  TMVAUtilities::loadPlugins("FastBDT");
}

std::string Expert::getAbsWeightFilePath()
{
  std::string weightFilePath =
    m_weightFolderName == "" ?
    getWeightFileName() : m_weightFolderName + "/" + getWeightFileName();

  std::string absWeightFilePath = FileSystem::findFile(weightFilePath);
  return absWeightFilePath;
}

void Expert::initializeReader(const std::function<void(TMVA::Reader&)> setReaderAddresses)
{
  setReaderAddresses(m_reader);
  m_reader.BookMVA("FastBDT", getAbsWeightFilePath());
}

void Expert::initializeReader(std::vector<NamedFloatTuple*> allVariables)
{
  initializeReader([&allVariables](TMVA::Reader & reader) {
    for (NamedFloatTuple* variables : allVariables) {
      size_t nVars = variables->size();
      for (size_t iVar = 0; iVar < nVars; ++iVar) {
        std::string name = variables->getNameWithPrefix(iVar);
        TString tName(name.c_str());
        Float_t& value = (*variables)[iVar];
        reader.AddVariable(tName, &value);
      }
    }
  });
  m_reader.BookMVA("FastBDT", getAbsWeightFilePath());
}

double Expert::predict()
{
  return m_reader.EvaluateMVA("FastBDT");
}
