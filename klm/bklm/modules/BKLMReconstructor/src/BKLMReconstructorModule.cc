/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/bklm/modules/BKLMReconstructor/BKLMReconstructorModule.h>

/* KLM headers. */
#include <klm/bklm/geometry/Module.h>

/* Belle 2 headers. */
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

/* CLHEP headers. */
#include <CLHEP/Vector/ThreeVector.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::bklm;

REG_MODULE(BKLMReconstructor)

BKLMReconstructorModule::BKLMReconstructorModule() :
  Module(),
  m_GeoPar(nullptr)
{
  setDescription("Create BKLMHit1ds from BKLMDigits and then create BKLMHit2ds from BKLMHit1ds.");
  setPropertyFlags(c_ParallelProcessingCertified);
  // MC 1 GeV/c muons: 1-sigma width is 0.43 ns
  addParam("CoincidenceWindow", m_CoincidenceWindow,
           "Half-width time coincidence window between adjacent BKLMDigits or orthogonal BKLMHit1ds (ns).",
           double(50.0));
  // MC 1 GeV/c muons: mean prompt time is 0.43 ns
  addParam("PromptTime", m_PromptTime,
           "Nominal time of prompt BKLMHit2ds (ns).",
           double(0.0));
  // MC 1 GeV/c muons: 1-sigma width is 0.15 ns
  addParam("PromptWindow", m_PromptWindow,
           "Half-width time window of BKLMHit2ds relative to PrompTime (ns).",
           //double(50.0));
           double(2000.0));
  addParam("IfAlign", m_IfAlign,
           "Perform alignment correction (true) or not (false).",
           bool(true));
  addParam("LoadTimingFromDB", m_LoadTimingFromDB,
           "Load timing window from database (true) or not (false).",
           bool(true));
  addParam("IgnoreScintillators", m_IgnoreScintillators,
           "Ignore scintillators (to debug their electronics mapping).",
           false);
}

BKLMReconstructorModule::~BKLMReconstructorModule()
{
}

void BKLMReconstructorModule::initialize()
{
  m_Digits.isRequired();
  m_Hit1ds.registerInDataStore();
  m_Hit2ds.registerInDataStore();
  m_Hit1ds.registerRelationTo(m_Digits);
  m_Hit2ds.registerRelationTo(m_Hit1ds);
  m_GeoPar = bklm::GeometryPar::instance();
}

void BKLMReconstructorModule::beginRun()
{
  if (m_LoadTimingFromDB) {
    if (!m_Timing.isValid())
      B2FATAL("BKLM time window data are not available.");
    m_CoincidenceWindow = m_Timing->getCoincidenceWindow();
    m_PromptTime = m_Timing->getPromptTime();
    m_PromptWindow = m_Timing->getPromptWindow();
  }
}

void BKLMReconstructorModule::event()
{
  /* Construct BKLMHit1Ds from BKLMDigits. */
  /* Sort BKLMDigits by module and strip number. */
  std::map<uint16_t, int> channelDigitMap;
  for (int index = 0; index < m_Digits.getEntries(); ++index) {
    const BKLMDigit* digit = m_Digits[index];
    if (m_IgnoreScintillators && !digit->inRPC())
      continue;
    if (digit->inRPC() || digit->isAboveThreshold()) {
      int module = digit->getModuleID();
      uint16_t channel = BKLMElementNumbers::getChannelByModule(module);
      channelDigitMap.insert(std::pair<uint16_t, int>(channel, index));
    }
  }
  if (channelDigitMap.empty())
    return;
  std::vector<const BKLMDigit*> digitCluster;
  uint16_t previousChannel = channelDigitMap.begin()->first;
  double averageTime = m_Digits[channelDigitMap.begin()->second]->getTime();
  for (std::map<uint16_t, int>::iterator it = channelDigitMap.begin(); it != channelDigitMap.end(); ++it) {
    const BKLMDigit* digit = m_Digits[it->second];
    double digitTime = digit->getTime();
    if ((it->first > previousChannel + 1) || (std::fabs(digitTime - averageTime) > m_CoincidenceWindow)) {
      m_Hit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> BKLMDigit
      digitCluster.clear();
    }
    previousChannel = it->first;
    double n = (double)(digitCluster.size());
    averageTime = (n * averageTime + digitTime) / (n + 1.0);
    digitCluster.push_back(digit);
  }
  m_Hit1ds.appendNew(digitCluster); // Also sets relation BKLMHit1d -> BKLMDigit

  /* Construct BKLMHit2Ds from orthogonal same-module BKLMHit1Ds. */
  for (int i = 0; i < m_Hit1ds.getEntries(); ++i) {
    int moduleID = m_Hit1ds[i]->getModuleID();
    const bklm::Module* m = m_GeoPar->findModule(m_Hit1ds[i]->getSection(), m_Hit1ds[i]->getSector(), m_Hit1ds[i]->getLayer());
    bool isPhiReadout = m_Hit1ds[i]->isPhiReadout();
    for (int j = i + 1; j < m_Hit1ds.getEntries(); ++j) {
      if (!BKLMElementNumbers::hitsFromSameModule(
            moduleID, m_Hit1ds[j]->getModuleID()))
        continue;
      if (isPhiReadout == m_Hit1ds[j]->isPhiReadout())
        continue;
      int phiIndex = isPhiReadout ? i : j;
      int zIndex   = isPhiReadout ? j : i;
      const BKLMHit1d* phiHit = m_Hit1ds[phiIndex];
      const BKLMHit1d* zHit = m_Hit1ds[zIndex];
      CLHEP::Hep3Vector local = m->getLocalPosition(phiHit->getStripAve(), zHit->getStripAve());
      CLHEP::Hep3Vector propagationTimes = m->getPropagationTimes(local);
      double phiTime = phiHit->getTime() - propagationTimes.y();
      double zTime = zHit->getTime() - propagationTimes.z();
      if (std::fabs(phiTime - zTime) > m_CoincidenceWindow)
        continue;
      // The second param in localToGlobal is whether do the alignment correction (true) or not (false)
      CLHEP::Hep3Vector global = m->localToGlobal(local + m->getLocalReconstructionShift(), m_IfAlign);
      double time = 0.5 * (phiTime + zTime) - global.mag() / Const::speedOfLight;
      BKLMHit2d* hit2d = m_Hit2ds.appendNew(phiHit, zHit, global, time); // Also sets relation BKLMHit2d -> BKLMHit1d
      if (fabs(time - m_PromptTime) > m_PromptWindow)
        hit2d->isOutOfTime();
    }
  }
}

void BKLMReconstructorModule::endRun()
{
}

void BKLMReconstructorModule::terminate()
{
}
