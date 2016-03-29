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
#include <iostream>

using namespace std;
using namespace Belle2;

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
}

