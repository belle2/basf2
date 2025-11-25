/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/mva/MVAExpert.h>

/** Impl Declaration **/
#include <mva/dataobjects/DatabaseRepresentationOfWeightfile.h>
#include <mva/interface/Expert.h>
#include <mva/interface/Weightfile.h>
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
    /// Implementation of the class to interact with the MVA package
    class MVAExpert::Impl {

    public:
      Impl(const std::string& identifier, std::vector<Named<Float_t*>> namedVariables); /**< constructor */
      void initialize(); /**< Signal the beginning of the event processing */
      void beginRun(); /**< Called once before a new run begins */
      std::unique_ptr<MVA::Weightfile> getWeightFile(); /**< Get the weight file */
      double predict(); /**< Get the MVA prediction */
      std::vector<float> predict(float* /* test_data */, int /* nFeature */, int /* nRows */); /** Get predictions for several inputs */
      std::vector<std::string> getVariableNames();
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

      /// General options
      MVA::GeneralOptions m_generalOptions;

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
    if ((weightfile->getElement<std::string>("method") == "FastBDT" and
         (weightfile->getElement<int>("FastBDT_version") == 1 or
          weightfile->getElement<int>("FastBDT_version") == 2)) or
        (weightfile->getElement<std::string>("method") == "Python")) {

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
      B2WARNING("Unpacked new kind of classifier. Consider to extend the feature variable check. Identifier name: " << m_identifier
                << "; method name: " << weightfile->getElement<std::string>("method"));
      m_selectedNamedVariables = m_allNamedVariables;
    }

    std::map<std::string, MVA::AbstractInterface*> supportedInterfaces =
      MVA::AbstractInterface::getSupportedInterfaces();
    weightfile->getOptions(m_generalOptions);
    m_expert = supportedInterfaces[m_generalOptions.m_method]->getExpert();
    m_expert->load(*weightfile);

    std::vector<float> dummy;
    dummy.resize(m_selectedNamedVariables.size(), 0);
    m_dataset = std::make_unique<MVA::SingleDataset>(m_generalOptions, std::move(dummy), 0);
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

std::vector<float> MVAExpert::Impl::predict(float* test_data, int nFeature, int nRows)   /** Get predictions for several inputs */
{
  std::vector<std::vector<float>> spectators;
  std::vector<std::vector <float> > data;
  data.resize(nRows);
  for (int iRow = 0; iRow < nRows; iRow += 1) {
    data[iRow].resize(nFeature);
    for (int iFeature = 0; iFeature < nFeature; iFeature += 1) {
      data[iRow][iFeature] = test_data[nFeature * iRow + iFeature];
    }
  }

  MVA::MultiDataset dataSet(m_generalOptions, data, spectators);
  return m_expert->apply(dataSet);
}

std::vector<std::string> MVAExpert::Impl::getVariableNames()
{
  std::vector<std::string> out(m_selectedNamedVariables.size());
  for (size_t iName = 0; iName < m_selectedNamedVariables.size(); iName += 1) {
    out[iName] = m_selectedNamedVariables[iName].getName();
  }
  return out;
}


/** PImpl Interface **/
// Silence Doxygen which is complaining that "no matching class member found for"
// But there should be a better way that I just don't know of / find
//! @cond Doxygen_Suppress
MVAExpert::MVAExpert(const std::string& identifier,
                     std::vector<Named<Float_t*> > namedVariables)
  : m_impl(std::make_unique<MVAExpert::Impl>(identifier, std::move(namedVariables)))
//! @endcond
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

std::vector<float> MVAExpert::predict(float* test_data, int nFeature, int nRows)
{
  return m_impl->predict(test_data, nFeature, nRows);
}

std::vector<std::string> MVAExpert::getVariableNames()
{
  return m_impl->getVariableNames();
}

