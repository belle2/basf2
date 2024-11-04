/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <top/dbobjects/TOPCalPhotonYields.h>
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {

  void TOPCalPhotonYields::set(int slot, const TH1F* photonYields, const TH1F* backgroundYields, const TH1F* alphaRatio,
                               const TH1F* activePixels, const TH2F* pulseHeights, const TH1F* muonZ)
  {
    string slotName = (slot < 10) ? "_0" + to_string(slot) : "_" + to_string(slot);

    m_photonYields.push_back(TH2F(("photonYields" + slotName).c_str(),
                                  ("Photon yields for slot " + to_string(slot) + "; pixel column; pixel row").c_str(),
                                  c_numCols, 0.5, c_numCols + 0.5, c_numRows, 0.5, c_numRows + 0.5));
    copyContent(photonYields, m_photonYields.back());

    m_backgroundYields.push_back(TH2F(("backgroundYields" + slotName).c_str(),
                                      ("Background yields for slot " + to_string(slot) + "; pixel column; pixel row").c_str(),
                                      c_numCols, 0.5, c_numCols + 0.5, c_numRows, 0.5, c_numRows + 0.5));
    copyContent(backgroundYields, m_backgroundYields.back());

    m_alphaRatio.push_back(TH2F(("alphaRatio" + slotName).c_str(),
                                ("Equalized alpha ratio for slot " + to_string(slot) + "; pixel column; pixel row").c_str(),
                                c_numCols, 0.5, c_numCols + 0.5, c_numRows, 0.5, c_numRows + 0.5));
    copyContent(alphaRatio, m_alphaRatio.back());

    m_activePixels.push_back(TH2F(("activePixels" + slotName).c_str(),
                                  ("Active pixels for slot " + to_string(slot) + "; pixel column; pixel row").c_str(),
                                  c_numCols, 0.5, c_numCols + 0.5, c_numRows, 0.5, c_numRows + 0.5));
    copyContent(activePixels, m_activePixels.back());
    m_activePixels.back().Scale(1 / muonZ->GetEntries());

    m_pulseHeights.push_back(*pulseHeights);
    m_muonZ.push_back(*muonZ);
  }


  void TOPCalPhotonYields::copyContent(const TH1F* input, TH2F& output)
  {
    for (int bin = 1; bin <= input->GetNbinsX(); bin++) {
      int row = (bin - 1) / 64 + 1;
      int col = (bin - 1) % 64 + 1;
      output.SetBinContent(col, row, input->GetBinContent(bin));
      output.SetBinError(col, row, input->GetBinError(bin));
    }
  }


  const TH2F* TOPCalPhotonYields::getPhotonYields(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_photonYields.size()) return &m_photonYields[index];
    return 0;
  }


  const TH2F* TOPCalPhotonYields::getBackgroundYields(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_backgroundYields.size()) return &m_backgroundYields[index];
    return 0;
  }


  const TH2F* TOPCalPhotonYields::getAlphaRatio(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_alphaRatio.size()) return &m_alphaRatio[index];
    return 0;
  }


  const TH2F* TOPCalPhotonYields::getActivePixels(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_activePixels.size()) return &m_activePixels[index];
    return 0;
  }


  const TH2F* TOPCalPhotonYields::getPulseHeights(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_pulseHeights.size()) return &m_pulseHeights[index];
    return 0;
  }


  const TH1F* TOPCalPhotonYields::getMuonZ(int slot) const
  {
    unsigned index = slot - 1;
    if (index < m_muonZ.size()) return &m_muonZ[index];
    return 0;
  }


}
