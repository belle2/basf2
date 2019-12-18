#include <arich/calibration/ARICHChannelMaskMaker.h>
#include <arich/dbobjects/ARICHChannelMask.h>
#include <TH1F.h>

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

  auto hist = getObjectPtr<TH1F>("ch_occupancy");
  if (!hist) return c_Failure;

  B2INFO("Number of Entries in ARICH ch_occupancy histogram was " << hist->GetEntries());

  const int NumberOfChannelsPerHapd = 144;
  int numChannels = hist->GetNbinsX();

  int nevt = hist->GetBinContent(numChannels);

  if (m_minOcc * nevt  < m_minHitAtMinOcc) return c_NotEnoughData;


  auto* mask = new ARICHChannelMask();

  for (int bin = 1; bin < numChannels; ++bin) {
    int moduleID = (bin - 1) / NumberOfChannelsPerHapd + 1;
    int channelID = (bin - 1) % NumberOfChannelsPerHapd;
    bool value = true;
    double occ = hist->GetBinContent(bin) / nevt;

    if (occ > m_maxOcc || occ < m_minOcc) value = false;

    mask->setActiveCh(moduleID, channelID, value);
  }

  saveCalibration(mask);

  return c_OK;

}
