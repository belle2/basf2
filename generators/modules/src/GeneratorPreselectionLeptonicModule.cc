/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <generators/modules/GeneratorPreselectionLeptonicModule.h>
#include <framework/gearbox/Unit.h>
#include <numeric>

using namespace std;
using namespace Belle2;

REG_MODULE(GeneratorPreselectionLeptonic);

GeneratorPreselectionLeptonicModule::GeneratorPreselectionLeptonicModule() : Module()
{
  // Set module properties
  setDescription("Filtering based on generator truth information. Returns 0 if cuts have not been passed, 1 only if all cuts are passed.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("signalLeptonPDG", m_signalLeptonPDG, "PDG code for the signal lepton", 11.0);
  addParam("signalLeptonPMin", m_signalLeptonPMin, "minimum momentum (CMS) for signal lepton", 0.0);
  addParam("signalLeptonPMax", m_signalLeptonPMax, "maximum momentum (CMS) for signal lepton", 5.0);
  addParam("tauLeptonPMin", m_tauLeptonPMin, "minimum momentum (CMS) for tau daughter lepton", 0.0);
  addParam("tauLeptonPMax", m_tauLeptonPMax, "maximum momentum (CMS) for tau daughter lepton", 5.0);
  addParam("projectionMin", m_projectionMin, "minimum value for projection of tau lepton onto signal lepton momentum (in CMS)",
           -10.0);
  addParam("projectionMax", m_projectionMax, "maximum value for projection of tau lepton onto signal lepton momentum (in CMS)", 10.0);
  addParam("angleMin", m_angleMin,
           "minimum value for the cosine of the angle between the tau and signal lepton momentum vectors (in CMS)", -1.0);
  addParam("angleMax", m_angleMax,
           "maximum value for the cosine of the angle between the tau and signal lepton momentum vectors (in CMS)", 1.0);
  addParam("zDiffMin", m_zDiffMin, "minimum value for difference between production vertex z-component for signal and tau leptons",
           -5.0);
  addParam("zDiffMax", m_zDiffMax, "maximum value for difference between production vertex z-component for signal and tau leptons",
           5.0);
  addParam("UMin", m_UMin, "minimum value of U = E - p (calculated using vector sum of momenta and energy for neutrinos in CMS)",
           -10.0);
  addParam("UMax", m_UMax, "maximum value of U = E - p (calculated using vector sum of momenta and energy for neutrinos in CMS)",
           10.0);
}

void GeneratorPreselectionLeptonicModule::initialize()
{
  m_mcparticles.isRequired(m_particleList);
  m_initial.isRequired();
}

void GeneratorPreselectionLeptonicModule::event()
{
  m_nSignalLepton = 0;
  m_nTauLepton = 0;
  m_missingPx = 0.0;
  m_missingPy = 0.0;
  m_missingPz = 0.0;
  m_missingE = 0.0;

  for (int i = 0; i < m_mcparticles.getEntries(); i++) {
    MCParticle& mc = *m_mcparticles[i];
    checkParticle(mc);
  }

  double missingP = pow(pow(m_missingPx, 2) + pow(m_missingPy, 2) + pow(m_missingPz, 2), 0.5);
  double U = m_missingE - missingP;

  int returnValue = 0;

  if (m_nSignalLepton == 1 && m_nTauLepton > 0 && U > m_UMin && U < m_UMax) {
    for (int i = 0; i < m_nTauLepton; i++) {
      double dotProduct = m_signalLeptonPVec.Dot(m_tauLeptonPVecs[i]);
      double signalLeptonPCMS = sqrt(m_signalLeptonPVec.Mag2());
      double tauLeptonPCMS = sqrt(m_tauLeptonPVecs[i].Mag2());
      double angle = dotProduct / (signalLeptonPCMS * tauLeptonPCMS);
      double projection = dotProduct / signalLeptonPCMS;
      double zDiff = m_signalLeptonZ - m_tauLeptonZs[i];
      if (angle > m_angleMin && angle < m_angleMax && projection > m_projectionMin && projection < m_projectionMax && zDiff > m_zDiffMin
          && zDiff < m_zDiffMax) {
        returnValue = 1;
        break;
      }
    }
  }

  setReturnValue(returnValue);

}

void GeneratorPreselectionLeptonicModule::checkParticle(const MCParticle& mc)
{
  if (!mc.hasStatus(MCParticle::c_PrimaryParticle)) return;
  if (mc.hasStatus(MCParticle::c_Initial) or mc.hasStatus(MCParticle::c_IsVirtual)) return;

  int pdg = mc.getPDG();
  const auto vec = m_initial->getLabToCMS() * mc.get4Vector();
  double p = vec.P();

  if ((abs(pdg) == Const::muon.getPDGCode() || abs(pdg) == Const::electron.getPDGCode()) && p > m_tauLeptonPMin
      && p < m_tauLeptonPMax) {
    m_nTauLepton++;
    ROOT::Math::XYZVector temp(vec.Px(), vec.Py(), vec.Pz());
    m_tauLeptonPVecs.push_back(temp);
    m_tauLeptonZs.push_back(mc.getVertex().Z());
  }

  if (abs(pdg) == m_signalLeptonPDG && p > m_signalLeptonPMin && p < m_signalLeptonPMax) {
    m_nSignalLepton++;
    ROOT::Math::XYZVector temp(vec.Px(), vec.Py(), vec.Pz());
    m_signalLeptonPVec = temp;
    m_signalLeptonZ = mc.getVertex().Z();
  }

  if (abs(pdg) == 12 || abs(pdg) == 14) {
    m_missingPx += vec.Px();
    m_missingPy += vec.Py();
    m_missingPz += vec.Pz();
    m_missingE += vec.E();
  }

}