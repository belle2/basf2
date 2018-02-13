/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <background/dataobjects/BackgroundInfo.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <unordered_set>

using namespace std;
using namespace Belle2;


bool BackgroundInfo::canBeMerged(const BackgroundInfo* otherObj)
{

  if (otherObj->getMethod() != m_method) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different method)");
    return false;
  }

  std::unordered_set<int> bgThis;
  for (const auto& bg : m_backgrounds) {
    int key = bg.tag * 16 + bg.fileType;
    bgThis.emplace(key);
  }
  std::unordered_set<int> bgOther;
  for (const auto& bg : otherObj->getBackgrounds()) {
    int key = bg.tag * 16 + bg.fileType;
    bgOther.emplace(key);
  }
  if (bgOther != bgThis) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different backgrounds)");
    return false;
  }

  auto compThis = m_components;
  std::sort(compThis.begin(), compThis.end());
  auto compOther = otherObj->getComponents();
  std::sort(compOther.begin(), compOther.end());
  if (compOther != compThis) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different components)");
    return false;
  }

  if (otherObj->getMinTime() != m_minTime) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different minTime)");
    return false;
  }

  if (otherObj->getMaxTime() != m_maxTime) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different maxTime)");
    return false;
  }

  if (otherObj->getMinTimeECL() != m_minTimeECL) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different minTimeECL)");
    return false;
  }

  if (otherObj->getMaxTimeECL() != m_maxTimeECL) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different maxTimeECL)");
    return false;
  }

  if (otherObj->getMinTimePXD() != m_minTimePXD) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different minTimePXD)");
    return false;
  }

  if (otherObj->getMaxTimePXD() != m_maxTimePXD) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different maxTimePXD)");
    return false;
  }

  if (otherObj->getWrapAround() != m_wrapAround) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different wrapAround)");
    return false;
  }

  if (otherObj->getMaxEdepECL() != m_maxEdepECL) {
    B2ERROR("BackgroundInfo: objects cannot be merged (different maxEdepECL)");
    return false;
  }

  return true;
}


void BackgroundInfo::merge(const Mergeable* other)
{

  auto otherObj = static_cast<const BackgroundInfo*>(other);

  if (otherObj->getMethod() == c_Unknown and otherObj->getBackgrounds().empty())
    return; // no merge for empty object

  if (m_method == c_Unknown and m_backgrounds.empty()) { // empty, replace it with other
    *this = *otherObj;
    return;
  }

  if (!canBeMerged(otherObj)) throw BackgroundInfoNotMergeable();

  for (const auto& otherBg : otherObj->getBackgrounds()) {
    bool added{false};
    for (auto& bg : m_backgrounds) {
      if (otherBg.tag != bg.tag) continue;
      if (otherBg.fileType != bg.fileType) continue;
      if (otherBg.fileNames == bg.fileNames) {
        if (otherBg.scaleFactor != bg.scaleFactor) {
          B2ERROR("BackgroundInfo: objects cannot be merged (different scaleFactor)");
          throw BackgroundInfoNotMergeable();
        }
        bg.reused += otherBg.reused;
        added = true;
        break;
      }
    }
    if (!added) m_backgrounds.push_back(otherBg);
  }

}

void BackgroundInfo::clear()
{

  for (auto& bg : m_backgrounds) {
    bg.reused = 0;
  }

}


void BackgroundInfo::print() const
{
  switch (m_method) {
    case c_Mixing:
      cout << "Method: BG mixing" << endl;
      printForMixing();
      break;
    case c_Overlay:
      cout << "Method: BG overlay" << endl;
      printForOverlay();
      break;
    default:
      cout << "Method: unknown" << endl;
      printForMixing();
  }
}


void BackgroundInfo::printForMixing() const
{
  cout << "Components: ";
  if (m_components.empty()) {
    cout << "all ";
  } else {
    for (const auto& component : m_components) cout << component << " ";
  }
  cout << endl;

  cout << "Time window: ";
  cout << "PXD [" << m_minTimePXD << ", " << m_maxTimePXD << "] ns, ";
  cout << "ECL [" << m_minTimeECL << ", " << m_maxTimeECL << "] ns, ";
  cout << "other components [" << m_minTime << ", " << m_maxTime << "] ns";
  cout << endl;

  cout << "Wrapping around: ";
  if (m_wrapAround) {
    cout << "enabled";
  } else {
    cout << "disabled";
  }
  cout << endl;

  cout << "ECL deposited energy cut: " << m_maxEdepECL << " GeV" << endl;

  cout << "Samples: " << endl;
  for (const auto& bkg : m_backgrounds) {
    cout << " " << bkg.type << endl;
    cout << " equivalent time: " << bkg.realTime / 1000000 << " ms";
    cout << ", events: " << bkg.numEvents;
    cout << ", scale factor: " << bkg.scaleFactor;
    cout << ", rate: " << bkg.rate << " GHz";
    cout << ", re-used: " << bkg.reused << " times";
    cout << endl;

    for (const auto& fileName : bkg.fileNames) cout << " " << fileName << endl;
    cout << endl;

  }

}

void BackgroundInfo::printForOverlay() const
{
  cout << "Components: ";
  if (m_components.empty()) {
    cout << "all ";
  } else {
    for (const auto& component : m_components) cout << component << " ";
  }
  cout << endl;

  cout << "Samples: " << endl;
  for (const auto& bkg : m_backgrounds) {
    cout << " " << bkg.type << endl;
    cout << " events: " << bkg.numEvents;
    cout << ", re-used: " << bkg.reused << " times";
    cout << endl;

    for (const auto& fileName : bkg.fileNames) cout << " " << fileName << endl;
    cout << endl;
  }
}

