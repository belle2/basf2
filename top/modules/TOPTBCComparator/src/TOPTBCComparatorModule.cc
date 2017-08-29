/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/HistoModule.h>
#include <top/modules/TOPTBCComparator/TOPTBCComparatorModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/geometry/FrontEndMapper.h>
#include <framework/pcore/RbTuple.h>
#include <top/dataobjects/TOPDigit.h>
#include <utility>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "TDirectory.h"
#include "TSystemDirectory.h"
#include "TSystemFile.h"
#include "TSystem.h"
#include "TString.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TFile.h"
#include <boost/format.hpp>
#include <fstream>

using namespace std;
using boost::format;

namespace Belle2 {
  using namespace TOP;
  REG_MODULE(TOPTBCComparator)

  TOPTBCComparatorModule::TOPTBCComparatorModule() : HistoModule()
  {
    setDescription("TOP Time Base Correction monitor module ");

    addParam("inputDirectorList", m_inputDirectoryList,
             "List of the directories (one per IOV) in which the files with the calibration constants of the SCODS are stored. The format of each line must be: folderpath/  label ",
             string(" "));
    addParam("compareToPreviousSet", m_compareToPreviousSet,
             "If true, each CalSet is compared to the previous one in the order determined by the inputDirectorList file. If false, the reference CalSet is the first of the list",
             bool(true));
    addParam("outputFile", m_outputFile,
             "output file containing the monitoring plots", string("TBCComparisonResults.root"));
    addParam("minCalPulses", m_minCalPulses,
             "minimum number of calibration pulses need to flag a sample as non-empty", short(200));
    addParam("numSamples", m_numSamples,
             "number of samples that have been calibrated", short(256));

  }


  int TOPTBCComparatorModule::parseInputDirectoryLine(std::string inputString)
  {
    // resets the strings
    m_calSetDirectory.clear();
    m_calSetLabel.clear();

    // reads the string char by char to break it up in m_calSetDirectory and m_calSetLabel
    bool isDirectoryNameChar = true;
    bool isAtBeginning = true;

    for (std::string::size_type i = 0; i < inputString.size(); ++i) {
      char c = inputString[i];
      // The following ifs should catch all the possible cases
      if (c == ' ' && isAtBeginning) continue; // and empty space at the beginning of the line
      if (c == ' ' && !isAtBeginning) { // an empty space between the two parts
        isDirectoryNameChar = false;
        isAtBeginning = false;
        continue;
      }
      if (c != ' ' && isDirectoryNameChar) { // a good char belonging to the first part of the string
        m_calSetDirectory += c;
        isAtBeginning = false;
        continue;
      }
      if (c != ' ' && !isDirectoryNameChar) { // a good char belonging to the second part of the string
        m_calSetLabel += c;
        isAtBeginning = false;
        continue;
      }
      B2WARNING("Uncaught exception in parsing the input string. Ending the parsing."); // I should never reach thispoint
      return 0;
    }

    return 1;
  }



  int TOPTBCComparatorModule::parseSlotAndScrodIDfromFileName(std::string inputString)
  {
    // resets the IDs
    m_slotID = -1;
    m_boardstackID = -1;
    m_scrodID = -1;

    // reads the string char by char to break it up in m_calSetDirectory and m_calSetLabel


    std::string stringSlot = "";
    std::string stringBS = "";
    std::string stringScrod  = "";


    // We may eventually implement a more clever parsere that is not so sensitive to the file name...
    // tbcSlotXX_Y-scrodZZZ.root
    // tbcSlotXX_Y-scrodZZ.root
    // 012345678901234567890
    // char 19 is sometimes a ".", sometimes a number


    stringSlot += inputString[7];
    stringSlot += inputString[8];
    stringBS += inputString[10];
    stringScrod += inputString[17];
    stringScrod += inputString[18];
    if (inputString[19] != '.')
      stringScrod += inputString[19];

    B2INFO("Parsed infos from filename " << inputString << " : Slot = " << stringSlot << ", Scrod = " << stringScrod << ", BS = " <<
           stringBS);


    m_slotID = std::stoi(stringSlot);
    m_scrodID = std::stoi(stringScrod);
    m_boardstackID = std::stoi(stringBS);

    return 1;
  }





  void TOPTBCComparatorModule::defineHisto()
  {
    // opens the file contining the list of directories and the labels
    ifstream inputDirectoryListFile(m_inputDirectoryList.c_str());

    // checsk the input file
    if (!inputDirectoryListFile) {
      B2ERROR("Unable to open the input file with the list of CalSets to analyze");
      return;
    }

    // Counter to check how many stest are seen in the input list. Used only for the log output
    int totCalSets = 0;

    std::string inputString;

    // reads the Input file line by-line to initialize the correct number of histogram sets, and reat the labels..
    // This is effectively a loop over all the calsets.
    while (std::getline(inputDirectoryListFile, inputString)) {

      // This initializes m_calSetDirectory and m_calSetLabel, even if now only m_calSetLabel will be used
      parseInputDirectoryLine(inputString);

      B2INFO("Initializing histograms for Calibration set located at " << m_calSetDirectory << " with label " << m_calSetLabel);

      // To be used to initialize the histograms in the following
      std::string name;
      std::string title;


      // ------
      // Calset-by-calset histograms.
      // initialize here all the histograms that appear once per calibration set, and not slot-by-slot
      // ------

      name = str(format("TOPAverageDeltaT_CalSet_%1%") % m_calSetLabel);
      title = str(format("Average value of #Delta T VS global channel number. CalSet %1%") %  m_calSetLabel);
      m_topAverageDeltaT.push_back(new TH1F(name.c_str(), title.c_str(), 512 * 16, 0., 512 * 16.));


      name = str(format("TOPSigmaDeltaT_CalSet_%1%") % m_calSetLabel);
      title = str(format("Sigma value of #Delta T VS global channel number. CalSet %1%") %  m_calSetLabel);
      m_topSigmaDeltaT.push_back(new TH1F(name.c_str(), title.c_str(), 512 * 16, 0., 512 * 16.));

      name = str(format("TOPSampleOccupancy_CalSet_%1%") % m_calSetLabel);
      title = str(format("Average number of calpulses per sample VS global channel number.  CalSet %1%") %  m_calSetLabel);
      m_topSampleOccupancy.push_back(new TH1F(name.c_str(), title.c_str(), 512 * 16, 0., 512 * 16.));


      // ------
      // Slot-by-slot histograms.
      // Use this loop to initialize  all the histograms that appear once per calibration set and per each slot
      // ------
      for (int iSlot = 0; iSlot < 16; iSlot ++) {

        // Average DeltaT stuff
        name = str(format("SlotAverageDeltaT_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Average value of #Delta T VS Channel number.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotAverageDeltaT[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotSigmaDeltaT_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Standard deviation of #Delta T VS Channel number.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaT[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotAverageDeltaTMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the average value of #Delta T on the 256 samples.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotAverageDeltaTMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

        name = str(format("SlotSigmaDeltaTMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the RMS of #Delta T on the 256 samples .  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaTMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));


        // Occupancy stuff
        name = str(format("SlotSampleOccupancy_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Average number of calpulses per sample used to calibrate Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotSampleOccupancy[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotEmptySamples_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Number samples with less than %3% calpulses per each slot channel. Slot %1%, CalSet %2%") %
                    (iSlot) % m_calSetLabel % m_minCalPulses);
        m_slotEmptySamples[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotSampleOccupancyMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the average number of calpulses per sample used to calibrate Slot %1%, CalSet %2%") %
                    (iSlot) % m_calSetLabel);
        m_slotSampleOccupancyMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

        name = str(format("SlotEmptySamplesMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the number samples with less than %3% calpulses per each. Slot %1%, CalSet %2%") %
                    (iSlot) % m_calSetLabel % m_minCalPulses);
        m_slotEmptySamplesMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));



        // Ratios
        name = str(format("SlotAverageDeltaTComparison_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Ratio of the average #Delta T in CalSet %2% over the previous one.  Slot %1%") % (iSlot) % m_calSetLabel);
        m_slotAverageDeltaTComparison[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotRMSDeltaTComparison_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Ratio of the RMS of #Delta T in CalSet %2% over the previous one.  Slot %1%") % (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaTComparison[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotAverageDeltaTMapComparison_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the ratio of the average #Delta T in CalSet %2% over the previous one.  Slot %1%") %
                    (iSlot) % m_calSetLabel);
        m_slotAverageDeltaTMapComparison[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

        name = str(format("SlotRMSDeltaTMapComparison_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the ratio of the RMS of #Delta T in CalSet %2% over the previous one.  Slot %1%") %
                    (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaTMapComparison[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

      }

      totCalSets++;
    }
    B2INFO("Initialization of the histograms for " << totCalSets << " CalSets done.");
    return;
  }



  int TOPTBCComparatorModule::analyzeCalFile()
  {
    // Here the histograms are filled
    // WARNING! What root calls "RMS" is actually a standard deviation

    B2INFO(" Starting the analisys of " << m_calSetDirectory << m_calSetFile->GetName());

    // Sanity check on the parsed parameters
    if (m_slotID < 0 || m_boardstackID < 0 || m_scrodID < 0) {
      B2WARNING("Negative slot, BS or scrod ID found while calling analyzeCalFile(). Looks like they have not been initialized, or that a function re-initialized them");
      return 0;
    }

    // Loop over all the histograms that should be found in the file
    for (short iChannel = 0; iChannel < 128; iChannel++) {

      // Pics up one histogram just to check that the channel iChannel was actually used to calculate the calibrations
      if (!m_calSetFile->Get(str(format("timeDiff_ch%1%") % (iChannel)).c_str())) {
        B2INFO("No Calibrations have been calculated slot " << m_slotID << ", BS " << m_boardstackID << " channel " << iChannel);
        continue;
      }



      // ---------
      // 1) Define some channel numbering quantities
      // Watchout: m_slotID is in [1..16] so slot m_slotID is on the element m_slotID-1 of the array
      // ---------

      short hardwareChannel = iChannel + 128 * m_boardstackID; // 0-511 across the whole module, BS by BS
      auto& chMapper = TOP::TOPGeometryPar::Instance()->getChannelMapper();
      short pixelID = chMapper.getPixelID(hardwareChannel); // 1-512 across the whole module, in rows
      short colNum = (pixelID - 1) % 64 + 1; // 1- 64  column ID (right to left looking from the quartz to the PMTs)
      short rowNum = (pixelID - 1) / 64 + 1 ; // 1- 8 row ID (bottom to top looking from the quartz to the PMTs)
      short globalChannel = hardwareChannel + 512 * (m_slotID -
                                                     1); // channel number across the whole detector, 0-8191. Used for cal monitorin only


      // ---------
      // 2) Channel-by-channel DeltaT summaries (average on the 256 samples of each set)
      // ---------

      // Checks that the histogram needed here is not corrupted before using it
      if (!m_calSetFile->Get(str(format("timeDiffcal_ch%1%") % (iChannel)).c_str())) {
        B2WARNING("Error opening " << str(format("timeDiffcal_ch%1%") % (iChannel)));
      } else {
        TH2F* h_timeDiffcal = (TH2F*)m_calSetFile->Get(str(format("timeDiffcal_ch%1%") % (iChannel)).c_str());
        TH1D* h_projection = h_timeDiffcal->ProjectionY("h_projection", 1, m_numSamples); // full projection

        m_slotAverageDeltaT[m_slotID - 1][m_calSetID]->SetBinContent(hardwareChannel + 1, h_projection->GetMean());
        m_slotAverageDeltaT[m_slotID - 1][m_calSetID]->SetBinError(hardwareChannel + 1,
                                                                   h_projection->GetMeanError()); // Do we trust root on this?
        m_slotSigmaDeltaT[m_slotID - 1][m_calSetID]->SetBinContent(hardwareChannel + 1,
                                                                   h_projection->GetRMS()); // WARNING! What root calls "RMS" is actually a standard deviation
        m_slotSigmaDeltaT[m_slotID - 1][m_calSetID]->SetBinError(hardwareChannel + 1,
                                                                 h_projection->GetRMSError()); // WARNING! What root calls "RMS" is actually a standard deviation

        m_slotAverageDeltaTMap[m_slotID - 1][m_calSetID]->SetBinContent(colNum, rowNum, h_projection->GetMean());
        m_slotSigmaDeltaTMap[m_slotID - 1][m_calSetID]->SetBinContent(colNum, rowNum,
            h_projection->GetRMS());

        m_topAverageDeltaT[m_calSetID]->SetBinContent(globalChannel + 1, h_projection->GetMean());
        m_topSigmaDeltaT[m_calSetID]->SetBinContent(globalChannel + 1, h_projection->GetRMS());
      }

      // ---------
      // 3) Channel-by-channel Occupancy summaries (average on the 256 samples of each set)
      // ---------

      // Checks that the histogram needed here is not corrupted before using it
      if (!m_calSetFile->Get(str(format("sampleOccup_ch%1%") % (iChannel)).c_str())) {
        B2WARNING("Error opening " << str(format("sampleOccup_ch%1%") % (iChannel)));
      } else {
        TH1F* h_sampleOccup = (TH1F*)m_calSetFile->Get(str(format("sampleOccup_ch%1%") % (iChannel)).c_str());

        // reads the occupancy histogram bin-by-by to look for (almost) empty samples
        int nEmpty = 0;
        for (int iSample = 1; iSample < m_numSamples + 1 ; iSample++) {
          if (h_sampleOccup->GetBinContent(iSample) < m_minCalPulses) nEmpty++;
        }

        m_slotSampleOccupancy[m_slotID - 1][m_calSetID]->SetBinContent(hardwareChannel + 1,  h_sampleOccup->Integral() / m_numSamples);
        m_slotEmptySamples[m_slotID - 1][m_calSetID]->SetBinContent(hardwareChannel + 1,  nEmpty);

        m_slotSampleOccupancyMap[m_slotID - 1][m_calSetID]->SetBinContent(colNum, rowNum,  h_sampleOccup->Integral() / m_numSamples);
        m_slotEmptySamplesMap[m_slotID - 1][m_calSetID]->SetBinContent(colNum, rowNum,  nEmpty);

        m_topSampleOccupancy[m_calSetID]->SetBinContent(globalChannel + 1, h_sampleOccup->Integral() / m_numSamples);

      }



    }
    return 1;
  }

  void TOPTBCComparatorModule::initialize()
  {
    REG_HISTOGRAM;
  }

  void TOPTBCComparatorModule::beginRun()
  {
    return;
  }


  void TOPTBCComparatorModule::event()
  {
    return;
  }


  // the real stuff actually happens here
  void TOPTBCComparatorModule::endRun()
  {
    // opens the file containing the list of directories and the labels
    ifstream inputDirectoryListFile(m_inputDirectoryList.c_str());

    // checsk the input file
    if (!inputDirectoryListFile) {
      B2ERROR("Unable to open the input file with the list of CalSets to analyze");
      return;
    }

    std::string inputString;
    // reads the Input file line by-line
    while (std::getline(inputDirectoryListFile, inputString)) {
      // This initializes m_calSetDirectory and m_calSetLabel
      parseInputDirectoryLine(inputString);

      B2INFO("Processing the calibration set located in " << m_calSetDirectory << " and labelled " << m_calSetLabel);
      TSystemDirectory calSetDir(m_calSetDirectory.c_str(), m_calSetDirectory.c_str());
      TList* calSetFiles = calSetDir.GetListOfFiles();
      if (calSetFiles) {
        TSystemFile* file;
        TIter next(calSetFiles);
        while ((file = (TSystemFile*)next())) {
          std::string fileName = file->GetName();
          if (!file->IsDirectory() && TString(fileName.c_str()).EndsWith(".root")) {
            B2INFO("opening file " << m_calSetDirectory + fileName << endl);
            m_calSetFile = new TFile((m_calSetDirectory + fileName).c_str(), " ");

            // finds out the Slot and BS ID
            parseSlotAndScrodIDfromFileName(fileName);

            // Fills the histograms
            analyzeCalFile();

            m_calSetFile->Close();
          }
        }
      } else {
        B2WARNING("Error in creating the TList form the directory " << m_calSetDirectory);
        continue;
      }
      m_calSetID++; // jump to the next directory (i.e. the next calset)
    }
    B2INFO("Analisys concluded.");
    return;
  }


  void TOPTBCComparatorModule::terminate()
  {
    // writes the histos
    B2INFO("Creating output file " << m_outputFile);
    TFile outfile(m_outputFile.c_str(), "recreate");

    B2INFO("Writing histograms ");

    // opens the input list to retrive, one last time, the labels
    ifstream inputDirectoryListFile(m_inputDirectoryList.c_str());
    std::string inputString;
    int iSet = 0;
    while (std::getline(inputDirectoryListFile, inputString)) {
      parseInputDirectoryLine(inputString);
      TDirectory* dirSet = outfile.mkdir(m_calSetLabel.c_str());
      dirSet->cd();

      m_topAverageDeltaT[iSet]->Write();
      m_topSigmaDeltaT[iSet]->Write();
      m_topSampleOccupancy[iSet]->Write();

      for (int iSlot = 0; iSlot < 16; iSlot ++) {
        TDirectory* dirSlot = dirSet->mkdir(str(format("slot%1%") % (iSlot + 1)).c_str());
        dirSlot->cd();

        m_slotAverageDeltaT[iSlot][iSet]->Write();
        m_slotSigmaDeltaT[iSlot][iSet]->Write();
        m_slotAverageDeltaTMap[iSlot][iSet]->Write();
        m_slotSigmaDeltaTMap[iSlot][iSet]->Write();
        m_slotSampleOccupancy[iSlot][iSet]->Write();
        m_slotEmptySamples[iSlot][iSet]->Write();
        m_slotSampleOccupancyMap[iSlot][iSet]->Write();
        m_slotEmptySamplesMap[iSlot][iSet]->Write();
      }
      dirSet->cd();
      iSet++;
    }
  }

} // end Belle2 namespace
