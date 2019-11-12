#include <arich/calibration/ARICHChannelMaskMaker.h>

#include <memory>
#include <iostream>
#include <TTree.h>
#include <TH1F.h>
#include <TClonesArray.h>
#include <TRandom.h>

#include <arich/dbobjects/ARICHChannelMask.h>


using namespace Belle2;

ARICHChannelMaskMaker::ARICHChannelMaskMaker(): CalibrationAlgorithm("ARICHChannelMask")
{
  setDescription(
    " -------------------------- Test Calibration Algoritm -------------------------\n"
    "                                                                               \n"
    "  Testing algorithm which just gets mean of a test histogram collected by      \n"
    "  CaTest module and provides a DB object with another histogram with one       \n"
    "  entry at calibrated value.                                                   \n"
    " ------------------------------------------------------------------------------\n"
  );
}

CalibrationAlgorithm::EResult ARICHChannelMaskMaker::calibrate()
{

  auto hist = getObjectPtr<TH1F>("ch_occupancy");
  if (!hist) return c_Failure;

  B2INFO("Number of Entries in ch_occupancy histogram was " << hist->GetEntries());

  const int NumberOfChannelsPerHapd = 144;
  const int NumberOfHapds = 420;

  double mean = hist->GetEntries() / NumberOfChannelsPerHapd / NumberOfHapds;

  if (mean < 100) return c_NotEnoughData;


  auto* mask = new ARICHChannelMask();

  int numChannels = hist->GetNbinsX();

  for (int bin = 1; bin <= numChannels; ++bin) {
    int moduleID = (bin - 1) / NumberOfChannelsPerHapd + 1;
    int channelID = (bin - 1) % NumberOfChannelsPerHapd;
    bool value = true;
    if (hist->GetBinContent(bin) == 0) value = false;
    if (hist->GetBinContent(bin) > 20 * mean) value = false;

    mask->setActiveCh(moduleID, channelID, value);
  }

  saveCalibration(mask);

  return c_OK;

}
