/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1.h>
#include <TSystem.h>

/* C++ headers. */
#include <cstdlib>
#include <iostream>
#include <string>

using namespace Belle2;

int main(int argc, char* argv[])
{
  /* Print the usage message if --help or -h are used. */
  if (argc == 1 or std::string(argv[1]) == "--help" or std::string(argv[1]) == "-h") {
    std::cout << "Usage: " << argv[0] << " [INPUT_FILE] [CHANNEL1] [CHANNEL2] ... [CHANNELN]\n\n"
              "   This tool masks the given channels of the KLM DQM reference plots stored\n"
              "   in the given input file (here 'channel' means 'channel number').\n\n"
              "   The plots on which this tool acts are:\n"
              "     KLM/masked_channels;\n"
              "     KLM/strip_hits_subdetector_<X>_section_<Y>_sector_<W>_<Z>.\n\n"
              "   This tool is not intended to be run standalone, since it is executed\n"
              "   by 'b2klm-mask-dqm', which automatically detects the channels to be masked.\n";
    return 0;
  }
  /* Print error messages when needed. */
  int nChannels = argc - 2;
  if (nChannels == 0) {
    B2ERROR("There are no channels to mask!");
    return 0;
  }
  std::string inputFileName(argv[1]);
  if (inputFileName.find(".root") == std::string::npos) {
    B2ERROR("The input file is not a .root file!");
    return 0;
  }
  if (gSystem->AccessPathName(inputFileName.c_str())) {
    B2ERROR("The input file does not exist!");
    return 0;
  }
  TFile* inputFile = new TFile(inputFileName.c_str(), "UPDATE");
  if (!inputFile or inputFile->IsZombie()) {
    B2ERROR("The input file is not working!");
    return 0;
  }
  /* Now we can safely execute the masking. */
  inputFile->cd();
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  const KLMChannelArrayIndex* channelArrayIndex = &(KLMChannelArrayIndex::Instance());
  const KLMSectorArrayIndex* sectorArrayIndex = &(KLMSectorArrayIndex::Instance());
  TH1* histoSummary = (TH1*)inputFile->Get("KLM/masked_channels");
  if (!histoSummary) {
    B2ERROR("The histogram KLM/masked_channels is not found!");
    return 0;
  }
  for (int i = 2; i <= nChannels + 1; ++i) {
    KLMChannelNumber channelNumber = std::atoi(argv[i]);
    int subdetector, section, sector, layer, plane, strip;
    elementNumbers->channelNumberToElementNumbers(
      channelNumber, &subdetector, &section, &sector, &layer, &plane, &strip);
    /* First: mask the channel in occupancy plot. */
    KLMChannelNumber channelIndex = channelArrayIndex->getIndex(channelNumber);
    int nHistoOccupancy;
    if (subdetector == KLMElementNumbers::c_BKLM)
      nHistoOccupancy = 2;
    else
      nHistoOccupancy = 3;
    for (int j = 0; j < nHistoOccupancy; ++j) {
      std::string histoOccupancyName = "KLM/strip_hits_subdetector_" + std::to_string(subdetector) +
                                       "_section_" + std::to_string(section) +
                                       "_sector_" + std::to_string(sector) +
                                       "_" + std::to_string(j);
      TH1* histoOccupancy = (TH1*)inputFile->Get(histoOccupancyName.c_str());
      if (!histoOccupancy) {
        B2ERROR("The histogram " << histoOccupancyName << " is not found!");
        return 0;
      }
      TAxis* xAxis = histoOccupancy->GetXaxis();
      double xMin = xAxis->GetXmin();
      double xMax = xAxis->GetXmax();
      if ((channelIndex >= xMin) and (channelIndex < xMax)) {
        int bin = xAxis->FindBin(channelIndex);
        histoOccupancy->SetBinContent(bin, 0);
        inputFile->Write("", TObject::kOverwrite);
      }
    }
    /* Second: add the masked channel to the summary plot. */
    KLMSectorNumber sectorNumber;
    if (subdetector == KLMElementNumbers::c_BKLM)
      sectorNumber = elementNumbers->sectorNumberBKLM(section, sector);
    else
      sectorNumber = elementNumbers->sectorNumberEKLM(section, sector);
    uint16_t sectorIndex = sectorArrayIndex->getIndex(sectorNumber);
    histoSummary->Fill(sectorIndex);
  }
  inputFile->Write("", TObject::kOverwrite);
  inputFile->Close();
  delete inputFile;
  B2INFO("Masking complete: the reference file " << inputFileName << " is now ready.");
  return 0;
}
