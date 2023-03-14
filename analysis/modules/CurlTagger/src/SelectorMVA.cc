/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <analysis/modules/CurlTagger/SelectorMVA.h>

#include <analysis/variables/TrackVariables.h>
#include <analysis/variables/MCTruthVariables.h>
#include <analysis/variables/Variables.h>

using namespace Belle2;
using namespace CurlTagger;

SelectorMVA::SelectorMVA(bool belleFlag, bool trainFlag, std::string tFileName)
{
  m_TrainFlag = trainFlag;
  m_TFileName = tFileName;
  if (belleFlag) {
    m_identifier = "CurlTagger_FastBDT_Belle";
  } else {
    m_identifier = "CurlTagger_FastBDT_BelleII";
  }
}

SelectorMVA::~SelectorMVA() = default;

void SelectorMVA::updateVariables(Particle* iPart, Particle* jPart)
{
  if (m_TrainFlag) {
    m_IsCurl = Variable::genParticleIndex(iPart) == Variable::genParticleIndex(jPart);
  }
  m_ChargeProduct = iPart->getCharge() * jPart->getCharge();

  m_PPhi  = acos(iPart->getMomentum().Unit().Dot(jPart->getMomentum().Unit()));

  m_PtDiffEW = abs(Variable::particlePt(iPart) - Variable::particlePt(jPart)) / sqrt(pow(Variable::particlePtErr(
                 iPart), 2) + pow(Variable::particlePtErr(jPart), 2));

  m_PzDiffEW = abs(Variable::particlePz(iPart) - Variable::particlePz(jPart)) / sqrt(pow(Variable::particlePzErr(
                 iPart), 2) + pow(Variable::particlePzErr(jPart), 2));

  m_TrackD0DiffEW = abs(Variable::trackD0(iPart) - Variable::trackD0(jPart)) / sqrt(pow(Variable::trackD0Error(
                      iPart), 2) + pow(Variable::trackD0Error(jPart), 2));

  m_TrackZ0DiffEW = abs(Variable::trackZ0(iPart) - Variable::trackZ0(jPart)) / sqrt(pow(Variable::trackZ0Error(
                      iPart), 2) + pow(Variable::trackZ0Error(jPart), 2));

  m_TrackTanLambdaDiffEW = abs(Variable::trackTanLambda(iPart) - Variable::trackTanLambda(jPart)) / sqrt(pow(
                             Variable::trackTanLambdaError(
                               iPart), 2) + pow(Variable::trackTanLambdaError(jPart), 2));

  m_TrackPhi0DiffEW = abs(Variable::trackPhi0(iPart) - Variable::trackPhi0(jPart)) / sqrt(pow(Variable::trackPhi0Error(
                        iPart), 2) + pow(Variable::trackPhi0Error(jPart), 2));

  m_TrackOmegaDiffEW = abs(Variable::trackOmega(iPart) - Variable::trackOmega(jPart)) / sqrt(pow(Variable::trackOmegaError(
                         iPart), 2) + pow(Variable::trackOmegaError(jPart), 2));
}

std::vector<float> SelectorMVA::getVariables(Particle* iPart, Particle* jPart)
{
  updateVariables(iPart, jPart);
  return {m_PPhi, m_ChargeProduct, m_PtDiffEW,
          m_PzDiffEW, m_TrackD0DiffEW, m_TrackZ0DiffEW,
          m_TrackTanLambdaDiffEW, m_TrackPhi0DiffEW, m_TrackOmegaDiffEW};
}

void SelectorMVA::collectTrainingInfo(Particle* iPart, Particle* jPart)
{
  updateVariables(iPart, jPart);
  m_TTree -> Fill();
}

void SelectorMVA::initialize()
{
  if (m_TrainFlag) {
    //make training data
    m_TFile = TFile::Open(m_TFileName.c_str(), "RECREATE");
    m_TTree = new TTree("ntuple", "Training Data for the Curl Tagger MVA");

    m_TTree -> Branch("PPhi", &m_PPhi, "PPhi/F");
    m_TTree -> Branch("ChargeProduct", &m_ChargeProduct, "ChargeProduct/F");
    m_TTree -> Branch("PtDiffEW", &m_PtDiffEW, "PtDiffEW/F");
    m_TTree -> Branch("PzDiffEW", &m_PzDiffEW, "PzDiffEW/F");
    m_TTree -> Branch("TrackD0DiffEW", &m_TrackD0DiffEW, "TrackD0DiffEW/F");
    m_TTree -> Branch("TrackZ0DiffEW", &m_TrackZ0DiffEW, "TrackZ0DiffEW/F");
    m_TTree -> Branch("TrackTanLambdaDiffEW", &m_TrackTanLambdaDiffEW, "TrackTanLambdaDiffEW/F");
    m_TTree -> Branch("TrackPhi0DiffEW", &m_TrackPhi0DiffEW, "TrackPhi0DiffEW/F");
    m_TTree -> Branch("TrackOmegaDiffEW", &m_TrackOmegaDiffEW, "TrackOmegaDiffEW/F");

    m_TTree -> Branch("IsCurl", &m_IsCurl, "IsCurl/O");

    m_target_variable = "IsCurl";
    m_variables = {"PPhi", "ChargeProduct", "PtDiffEW",
                   "PzDiffEW", "TrackD0DiffEW", "TrackZ0DiffEW",
                   "TrackTanLambdaDiffEW", "TrackPhi0DiffEW", "TrackOmegaDiffEW"
                  };

  } else {
    // normal application
    m_weightfile_representation = std::make_unique<DBObjPtr<DatabaseRepresentationOfWeightfile>>(
                                    MVA::makeSaveForDatabase(m_identifier));
    (*m_weightfile_representation.get()).addCallback([this]() { initializeMVA();});
    initializeMVA();
  }
}

void SelectorMVA::initializeMVA()
{
  std::stringstream ss((*m_weightfile_representation)->m_data);
  m_weightfile = MVA::Weightfile::loadFromStream(ss);
  m_weightfile.getOptions(m_generalOptions);
  m_expert.load(m_weightfile);
}


float SelectorMVA::getOptimalResponseCut()
{
  if (m_TrainFlag) {
    return 0.5;
  }
  std::string elementIdentfier = "optimal_cut";
  if (!m_weightfile.containsElement(elementIdentfier)) {
    B2FATAL("No optimal cut stored in curlTagger MVA payload!");
  }
  // require the default value for the compiler to deduce the template class
  return m_weightfile.getElement(elementIdentfier, 0.5);
}

void SelectorMVA::finalize()
{
  if (m_TrainFlag) {
    m_TFile -> cd();
    m_TTree -> Write();
    m_TFile -> Write();
    m_TFile -> Close();
  }
}

float SelectorMVA::getResponse(Particle* iPart, Particle* jPart)
{
  MVA::SingleDataset dataset(m_generalOptions, getVariables(iPart, jPart));
  return m_expert.apply(dataset)[0];
}
