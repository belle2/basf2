/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mva/MVAExpert.h>
#include <framework/utilities/FileSystem.h>

#include <mva/interface/Interface.h>

#include <boost/algorithm/string/predicate.hpp>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAExpert::MVAExpert(const std::string& identifier)
  : m_identifier(identifier)
{
}

void MVAExpert::initialize(std::vector<Named<Float_t*> > namedVariables)
{
  m_namedVariables = std::move(namedVariables);

  MVA::AbstractInterface::initSupportedInterfaces();
  std::unique_ptr<MVA::Weightfile> weightfile = getWeightFile();

  if (weightfile) {
    std::map<std::string, MVA::AbstractInterface*> supportedInterfaces =
      MVA::AbstractInterface::getSupportedInterfaces();
    MVA::GeneralOptions generalOptions;
    weightfile->getOptions(generalOptions);
    m_expert = supportedInterfaces[generalOptions.m_method]->getExpert();
    m_expert->load(*weightfile);

    std::vector<float> dummy;
    dummy.resize(m_namedVariables.size(), 0);
    m_dataset =
      std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(generalOptions, std::move(dummy), 0));
  }
}

std::unique_ptr<MVA::Weightfile> MVAExpert::getWeightFile()
{
  if (not m_weightfileRepresentation and not(boost::ends_with(m_identifier, ".root") or
                                             boost::ends_with(m_identifier, ".xml"))) {
    using DBWeightFileRepresentation = DBObjPtr<DatabaseRepresentationOfWeightfile>;
    m_weightfileRepresentation
      = std::unique_ptr<DBWeightFileRepresentation>(new DBWeightFileRepresentation(m_identifier));
  }

  if (m_weightfileRepresentation) {
    if (m_weightfileRepresentation->hasChanged()) {
      std::stringstream ss((*m_weightfileRepresentation)->m_data);
      return std::unique_ptr<MVA::Weightfile>(new MVA::Weightfile(MVA::Weightfile::loadFromStream(ss)));
    } else {
      return nullptr;
    }
  } else {
    std::string weightFilePath = FileSystem::findFile(m_identifier);
    return std::unique_ptr<MVA::Weightfile>(new MVA::Weightfile(MVA::Weightfile::loadFromFile(weightFilePath)));
  }
}

double MVAExpert::predict()
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return NAN;
  }

  // Transfer the extracted values to the data set were the expert can find them
  for (unsigned int i = 0; i < m_namedVariables.size(); ++i) {
    m_dataset->m_input[i] = *m_namedVariables[i];
  }
  return m_expert->apply(*m_dataset)[0];
}
