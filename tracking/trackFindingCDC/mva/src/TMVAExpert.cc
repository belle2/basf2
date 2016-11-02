/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mva/TMVAExpert.h>
#include <framework/utilities/FileSystem.h>

#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TMVAExpert::TMVAExpert(const std::string& weightFolderName, const std::string& trainingName) :
  m_reader("Silent;!V"),
  m_weightFolderName(weightFolderName),
  m_trainingName(trainingName)
{
  B2DEBUG(100, "load plugin");
  TMVAUtilities::loadPlugins("FastBDT");
  B2DEBUG(100, "finished load plugin");
}

std::string TMVAExpert::getAbsWeightFilePath()
{
  std::string weightFilePath =
    m_weightFolderName == "" ?
    getWeightFileName() : m_weightFolderName + "/" + getWeightFileName();

  std::string absWeightFilePath = FileSystem::findFile(weightFilePath);
  return absWeightFilePath;
}

void TMVAExpert::initializeReader(const std::function<void(TMVA::Reader&)>& setReaderAddresses)
{
  setReaderAddresses(m_reader);
  B2DEBUG(100, "book method");
  m_reader.BookMVA("FastBDT", getAbsWeightFilePath());
}

void TMVAExpert::initializeReader(std::vector<Named<Float_t*> > namedVariables)
{
  initializeReader([&namedVariables](TMVA::Reader & reader) {
    for (const Named<Float_t*>& namedVariable : namedVariables) {
      std::string name = namedVariable.getName();
      TString tName(name.c_str());
      Float_t* variable = namedVariable;
      reader.AddVariable(tName, variable);
    }
  });
}

double TMVAExpert::predict()
{
  return m_reader.EvaluateMVA("FastBDT");
}
