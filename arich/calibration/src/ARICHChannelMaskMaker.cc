/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <arich/calibration/ARICHChannelMaskMaker.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <TH2F.h>

using namespace Belle2;

ARICHChannelMaskMaker::ARICHChannelMaskMaker(): CalibrationAlgorithm("ARICHChannelMask")
{
  setDescription(
    " --------------------- ARICHChannelMask Calibration Algoritm ------------------\n"
    "                                                                               \n"
    "  Produces channel mask for arich hot/dead channels, based on criteria of      \n"
    "  minimal and maximal occupancy                                                \n"
    " ------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult ARICHChannelMaskMaker::calibrate()
{

  auto hist = getObjectPtr<TH2F>("ch_occupancy");
  if (!hist) return c_Failure;

  B2INFO("Number of Entries in ARICH ch_occupancy histogram was " << hist->GetEntries());

  const int NumberOfChannelsPerHapd = 144;
  int numChannels = hist->GetNbinsX();

  int nevt = hist->GetBinContent(numChannels, 1);

  auto* mask = new ARICHChannelMask();

  double ringChnAvg[7] = {0.};
  double ringChnAvgS2N[7] = {0.};
  int hapdInRing[7] = {42, 48, 54, 60, 66, 72, 78};

  for (int bin = 1; bin < numChannels; ++bin) {
    int moduleID = (bin - 1) / NumberOfChannelsPerHapd + 1;
    double nsig = hist->GetBinContent(bin, 2) + hist->GetBinContent(bin, 3) - hist->GetBinContent(bin, 1) - hist->GetBinContent(bin, 4);
    double s2n = nsig  / (hist->GetBinContent(bin, 2) + hist->GetBinContent(bin, 3));
    int ring = getRing(moduleID);
    if (nsig / float(nevt) > 0.02) continue; // skip channels with anomalously high occupancy
    ringChnAvg[ring] += nsig;
    ringChnAvgS2N[ring] += s2n;
  }

  for (int i = 0; i < 7; i++) {
    ringChnAvg[i] /= float(hapdInRing[i] * NumberOfChannelsPerHapd);
    ringChnAvgS2N[i] /= float(hapdInRing[i] * NumberOfChannelsPerHapd);
  }
  B2INFO("Average hits in channel in outter HAPD ring is " << ringChnAvg[6] << " (which is less that minimaly required,  " <<
         m_minHitPerChn << ")");
  if (ringChnAvg[6] < m_minHitPerChn) return c_NotEnoughData;

  for (int bin = 1; bin < numChannels; ++bin) {
    int moduleID = (bin - 1) / NumberOfChannelsPerHapd + 1;
    int channelID = (bin - 1) % NumberOfChannelsPerHapd;
    double nsig = hist->GetBinContent(bin, 2) + hist->GetBinContent(bin, 3) - hist->GetBinContent(bin, 1) - hist->GetBinContent(bin, 4);
    double s2n = nsig / (hist->GetBinContent(bin, 2) + hist->GetBinContent(bin, 3));
    int ring = getRing(moduleID);
    bool value = true;
    if (nsig < ringChnAvg[ring]*m_minFrac || s2n < m_minS2N) value = false;
    mask->setActiveCh(moduleID, channelID, value);
  }

  saveCalibration(mask);

  return c_OK;

}


int ARICHChannelMaskMaker::getRing(int modID)
{
  if (modID <= 42) return 0;
  if (modID <= 90) return 1;
  if (modID <= 144) return 2;
  if (modID <= 204) return 3;
  if (modID <= 270) return 4;
  if (modID <= 342) return 5;
  if (modID <= 420) return 6;
  return -1; // -1 if invalid input
}
