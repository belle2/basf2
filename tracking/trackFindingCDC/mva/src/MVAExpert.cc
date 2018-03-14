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

/** Impl Declaration **/
#include <tracking/trackFindingCDC/varsets/NamedFloatTuple.h>

#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Weightfile.h>
#include <mva/interface/Expert.h>
#include <framework/database/DBObjPtr.h>

#include <boost/algorithm/string/predicate.hpp>

namespace Belle2 {
  class DatabaseRepresentationOfWeightfile;
  namespace MVA {
    class Expert;
    class SingleDataset;
    class Weightfile;
  }

  namespace TrackFindingCDC {
    class MVAExpert::Impl {

    public:
      Impl(const std::string& identifier, std::vector<Named<Float_t*>> namedVariables);
      void initialize();
      void beginRun();
      std::unique_ptr<MVA::Weightfile> getWeightFile();
      double predict();

    private:
      /// References to the all named values from the source variable set.
      std::vector<Named<Float_t*> > m_allNamedVariables;

      /// References to the *selected* named values from the source variable set.
      std::vector<Named<Float_t*> > m_selectedNamedVariables;

      /// Database pointer to the Database representation of the weightfile
      std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile> > m_weightfileRepresentation;

      /// Pointer to the current MVA Expert
      std::unique_ptr<MVA::Expert> m_expert;

      /// Pointer to the current dataset
      std::unique_ptr<MVA::Dataset> m_dataset;

      /// DB identifier of the expert or file name
      std::string m_identifier;
    };
  }
}

/** Impl Definitions **/
#include <mva/interface/Interface.h>

#include <framework/utilities/FileSystem.h>
#include <framework/logging/Logger.h>

#include <algorithm>

using namespace Belle2;
using namespace TrackFindingCDC;

MVAExpert::Impl::Impl(const std::string& identifier,
                      std::vector<Named<Float_t*> > namedVariables)
  : m_allNamedVariables(std::move(namedVariables))
  , m_identifier(identifier)
{
}

void MVAExpert::Impl::initialize()
{
  MVA::AbstractInterface::initSupportedInterfaces();
  using boost::algorithm::ends_with;
  if (not m_weightfileRepresentation and
      not(ends_with(m_identifier, ".root") or ends_with(m_identifier, ".xml"))) {
    using DBWeightFileRepresentation = DBObjPtr<DatabaseRepresentationOfWeightfile>;
    m_weightfileRepresentation = std::make_unique<DBWeightFileRepresentation>(m_identifier);
  }
}

void MVAExpert::Impl::beginRun()
{
  std::unique_ptr<MVA::Weightfile> weightfile = getWeightFile();
  if (weightfile) {
    if (weightfile->getElement<std::string>("method") == "FastBDT" and
        weightfile->getElement<int>("FastBDT_version") == 1) {

      int nExpectedVars = weightfile->getElement<int>("number_feature_variables");

      m_selectedNamedVariables.clear();
      for (int iVar = 0; iVar < nExpectedVars; ++iVar) {
        std::string variableElementName = "variable" + std::to_string(iVar);
        std::string expectedName = weightfile->getElement<std::string>(variableElementName);

        auto itNamedVariable = std::find_if(m_allNamedVariables.begin(),
                                            m_allNamedVariables.end(),
        [expectedName](const Named<Float_t*>& namedVariable) {
          return namedVariable.getName() == expectedName;
        });

        if (itNamedVariable == m_allNamedVariables.end()) {
          B2ERROR("Variable name " << iVar << " mismatch for FastBDT. " <<
                  "Could not find expected variable '" << expectedName << "'");
        }
        m_selectedNamedVariables.push_back(*itNamedVariable);
      }
      B2ASSERT("Number of variables mismatch", nExpectedVars == static_cast<int>(m_selectedNamedVariables.size()));
    } else {
      B2WARNING("Unpacked new kind of classifier. Consider to extend the feature variable check.");
      m_selectedNamedVariables = m_allNamedVariables;
    }

    std::map<std::string, MVA::AbstractInterface*> supportedInterfaces =
      MVA::AbstractInterface::getSupportedInterfaces();
    MVA::GeneralOptions generalOptions;
    weightfile->getOptions(generalOptions);
    m_expert = supportedInterfaces[generalOptions.m_method]->getExpert();
    m_expert->load(*weightfile);

    std::vector<float> dummy;
    dummy.resize(m_selectedNamedVariables.size(), 0);
    m_dataset = std::make_unique<MVA::SingleDataset>(generalOptions, std::move(dummy), 0);
  } else {
    B2ERROR("Could not find weight file for identifier " << m_identifier);
  }
}

std::unique_ptr<MVA::Weightfile> MVAExpert::Impl::getWeightFile()
{
  if (m_weightfileRepresentation) {
    std::stringstream ss((*m_weightfileRepresentation)->m_data);
    return std::make_unique<MVA::Weightfile>(MVA::Weightfile::loadFromStream(ss));
  } else {
    std::string weightFilePath = FileSystem::findFile(m_identifier);
    return std::make_unique<MVA::Weightfile>(MVA::Weightfile::loadFromFile(weightFilePath));
  }
}

double MVAExpert::Impl::predict()
{
  if (not m_expert) {
    B2ERROR("MVA Expert is not loaded! I will return 0");
    return NAN;
  }

  // Transfer the extracted values to the data set were the expert can find them
  for (unsigned int i = 0; i < m_selectedNamedVariables.size(); ++i) {
    m_dataset->m_input[i] = *m_selectedNamedVariables[i];
  }
  return m_expert->apply(*m_dataset)[0];
}

/** PImpl Interface **/
MVAExpert::MVAExpert(const std::string& identifier,
                     std::vector<Named<Float_t*> > namedVariables)
  : m_impl(std::make_unique<MVAExpert::Impl>(identifier, std::move(namedVariables)))
{
}

MVAExpert::~MVAExpert() = default;

void MVAExpert::initialize()
{
  return m_impl->initialize();
}

void MVAExpert::beginRun()
{
  return m_impl->beginRun();
}

double MVAExpert::predict()
{
  return m_impl->predict();
}
