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

Expert::Expert(const std::string& weightFolder, const std::string& name) : m_reader("!Silent")
{
  TMVAUtilities::loadPlugins("FastBDT");
  std::string fileName = name + "_FastBDT.weights.xml";
  m_weightFileName = FileSystem::findFile(weightFolder + "/" + fileName);
}

void Expert::initializeReader(const std::function<void(TMVA::Reader&)> setReaderAddresses)
{
  setReaderAddresses(m_reader);
  m_reader.BookMVA("FastBDT", m_weightFileName);
}

double Expert::useWeight()
{
  return m_reader.EvaluateMVA("FastBDT");
}
