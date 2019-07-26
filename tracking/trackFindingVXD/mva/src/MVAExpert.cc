/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Jonas Wagner                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingVXD/mva/MVAExpert.h>


#include <mva/interface/Interface.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <boost/algorithm/string/predicate.hpp>

using namespace Belle2;

MVAExpert::MVAExpert(const std::string& identifier,
                     std::vector<Named<float*>>& namedVariables)
  : m_namedVariables(namedVariables)
  , m_identifier(identifier)
{
}

void MVAExpert::initialize()
{
  MVA::AbstractInterface::initSupportedInterfaces();
  if (not m_weightfileRepresentation and
      not(boost::ends_with(m_identifier, ".root") or
          boost::ends_with(m_identifier, ".xml"))) {
    using DBWeightFileRepresentation = DBObjPtr<DatabaseRepresentationOfWeightfile>;
    m_weightfileRepresentation = std::make_unique<DBWeightFileRepresentation>(m_identifier);
  }
}

void MVAExpert::beginRun()
{
  std::unique_ptr<MVA::Weightfile> weightfile = getWeightFile();
  if (weightfile) {
    // TODO: add other methods besides FastBDT?
    // FastBDT_version refers to the weightfile version, only FastBDT_VERSION_MAJOR >= 5 can handle FastBDT_version==2
    // Currently using FastBDT_VERSION_MAJOR = 3 (in externals/v01-05-01/include/root/FastBDT.h )
    if (weightfile->getElement<std::string>("method") == "FastBDT" and
        (weightfile->getElement<int>("FastBDT_version") == 1 or
         weightfile->getElement<int>("FastBDT_version") == 2)) {

      int nExpectedVars = weightfile->getElement<int>("number_feature_variables");
      int nActualVars = m_namedVariables.size();

      if (nExpectedVars != nActualVars) {
        B2ERROR("Variable number mismatch for FastBDT. " <<
                "Expected '" << nExpectedVars << "' " <<
                "but received '" << nActualVars << "'");
      }

      for (int iVar = 0; iVar < nActualVars; ++iVar) {
        std::string variableElementName = "variable" + std::to_string(iVar);
        std::string expectedName = weightfile->getElement<std::string>(variableElementName);
        std::string actualName = m_namedVariables[iVar].getName();
        if (expectedName != actualName) {
          B2ERROR("Variable name " << iVar << " mismatch for FastBDT. " <<
                  "Expected '" << expectedName << "' " <<
                  "but received '" << actualName << "'");
        }
      }
    } else {
      B2WARNING("Unpacked new kind of classifier. Consider to extend the feature variable check.");
    }

    std::map<std::string, MVA::AbstractInterface*> supportedInterfaces =
      MVA::AbstractInterface::getSupportedInterfaces();
    MVA::GeneralOptions generalOptions;
    weightfile->getOptions(generalOptions);
    m_expert = supportedInterfaces[generalOptions.m_method]->getExpert();
    m_expert->load(*weightfile);

    std::vector<float> dummy;
    dummy.resize(m_namedVariables.size(), 0);
    m_dataset = std::make_unique<MVA::SingleDataset>(generalOptions, std::move(dummy), 0);
  } else {
    B2ERROR("Could not find weight file for identifier " << m_identifier);
  }
}

std::unique_ptr<MVA::Weightfile> MVAExpert::getWeightFile()
{
  if (m_weightfileRepresentation) {
    std::stringstream ss((*m_weightfileRepresentation)->m_data);
    return std::make_unique<MVA::Weightfile>(MVA::Weightfile::loadFromStream(ss));
  } else {
    std::string weightFilePath = FileSystem::findFile(m_identifier);
    return std::make_unique<MVA::Weightfile>(MVA::Weightfile::loadFromFile(weightFilePath));
  }
}

float MVAExpert::predict()
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return 0;
  }

  // Transfer the extracted values to the data set were the expert can find them
  for (unsigned int i = 0; i < m_namedVariables.size(); ++i) {
    m_dataset->m_input[i] = *(m_namedVariables[i].getValue());
  }

  return m_expert->apply(*m_dataset)[0];
}
