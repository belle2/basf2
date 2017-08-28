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

    std::string inputString;
    int totCalSets = 0; // counter to check how many stest are seen in the input list
    // reads the Input file line by-line
    while (std::getline(inputDirectoryListFile, inputString)) {
      //  B2DEBUG("Reading string " << inputString << " for histogram initialization.");

      parseInputDirectoryLine(
        inputString); // This initializes m_calSetDirectory and m_calSetLabel, even if now only m_calSetLabel will be used

      B2INFO("Initializing histograms for Calibration set located at " << m_calSetDirectory << " with label " << m_calSetLabel);


      // Initializes all the histograms
      for (int iSlot = 0; iSlot < 16; iSlot ++) {
        // single CalSet monitoring
        std::string name = str(format("SlotAverageDeltaT_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        std::string title = str(format("Average value of #Delta T VS Channel number.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotAverageDeltaT[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotRMSDeltaT_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("RMS of #Delta T VS Channel number.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaT[iSlot].push_back(new TH1F(name.c_str(), title.c_str(), 512, 0., 512.));

        name = str(format("SlotAverageDeltaTMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the average value of #Delta T on the 256 samples.  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotAverageDeltaTMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

        name = str(format("SlotRMSDeltaTMap_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Map of the RMS of #Delta T on the 256 samples .  Slot %1%, CalSet %2%") % (iSlot) % m_calSetLabel);
        m_slotSigmaDeltaTMap[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 64, 0., 64., 8, 0., 8.));

        name = str(format("SlotDeltaTScatter_Slot%1%_CalSet_%2%") % (iSlot) % m_calSetLabel);
        title = str(format("Mean and sigma of #DeltaT VS channel number (256 samples summed together).  Slot %1%, CalSet %2%") %
                    (iSlot) % m_calSetLabel);
        m_slotDeltaTScatter[iSlot].push_back(new TH2F(name.c_str(), title.c_str(), 512, 0., 512., 1000, 15., 25.));


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
    B2INFO(" Starting the analisys");

    if (m_slotID < 0 || m_boardstackID < 0 || m_scrodID < 0) {
      B2WARNING("Negative slot, BS or scrod ID found while calling analyzeCalFile(). Looks like they have not been initialized, or that a function re-initialized them");
      return 0;
    }

    // Now loops over all the histograms that should be found in the file
    for (short iChannel = 0; iChannel < 128; iChannel++) {
      if (!m_calSetFile->Get(str(format("timeDiff_ch%1%") % (iChannel)).c_str())) {
        //      B2INFO("No Calibrations have been calculated slot " << m_slotID << ", BS " << m_boardstackID << " channel " << iChannel);
        continue;
      }
      if (!m_calSetFile->Get(str(format("timeDiffcal_ch%1%") % (iChannel)).c_str())) {
        B2WARNING("Unexpoected problem in opening " << str(format("timeDiffcal_ch%1%") % (iChannel)));
        continue;
      }

      // First: calulate average and RMS of the DeltaT distribution after the calibration
      TH2F* h_timeDiffcal = (TH2F*)m_calSetFile->Get(str(format("timeDiffcal_ch%1%") % (iChannel)).c_str());
      for (int iSample = 1; iSample < m_numSamples + 1; iSample++) {
        TH1D* h_projection = h_timeDiffcal->ProjectionY("h_projection", iSample, iSample); // 1 bin-wide projection onto Y
        m_slotAverageDeltaT[m_slotID][m_calSetID]->SetBinContent(iSample + 1, h_projection->GetMean());
        m_slotAverageDeltaT[m_slotID][m_calSetID]->SetBinError(iSample + 1, h_projection->GetMeanError()); // Do we trust root on this?
        m_slotSigmaDeltaT[m_slotID][m_calSetID]->SetBinContent(iSample + 1,
                                                               h_projection->GetRMS()); // WARNING! What root calls "RMS" is actually a standard deviation
        m_slotSigmaDeltaT[m_slotID][m_calSetID]->SetBinError(iSample + 1,
                                                             h_projection->GetRMSError()); // WARNING! What root calls "RMS" is actually a standard deviation
        delete h_projection;
      }

      // Channel-by-channel summary maps
      short moduleChannel = iChannel + 128 * m_boardstackID; // go to the 0-511 representation of the channels
      short colNum = (moduleChannel - 1) % 64 + 1; // 1- 64 as the map bins
      short rowNum = (moduleChannel - 1) / 64 + 1; // 1- 8 as the map bins

      TH1D* h_projection = h_timeDiffcal->ProjectionY("h_projection", 1, 256); // full projection
      m_slotAverageDeltaTMap[m_slotID][m_calSetID]->SetBinContent(colNum, rowNum, h_projection->GetMean());
      m_slotSigmaDeltaTMap[m_slotID][m_calSetID]->SetBinContent(colNum, rowNum,
                                                                h_projection->GetRMS()); // WARNING! What root calls "RMS" is actually a standard deviation

      m_slotDeltaTScatter[m_slotID][m_calSetID]->SetBinContent(colNum, rowNum, h_projection->GetMean());
      m_slotDeltaTScatter[m_slotID][m_calSetID]->SetBinError(colNum, rowNum,
                                                             h_projection->GetRMS()); // WARNING! What root calls "RMS" is actually a standard deviation
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

    // Writes the single-set plots in order of calibration set
    for (int iSet = 0; iSet < m_calSetID; iSet++) {
      for (int iSlot = 0; iSlot < 16; iSlot ++) {
        m_slotAverageDeltaT[iSlot][iSet]->Write();
        m_slotSigmaDeltaT[iSlot][iSet]->Write();
        m_slotAverageDeltaTMap[iSlot][iSet]->Write();
        m_slotSigmaDeltaTMap[iSlot][iSet]->Write();
        m_slotDeltaTScatter[iSlot][iSet]->Write();
      }
    }

    // Writes the comparisons for slot
    for (int iSlot = 0; iSlot < 16; iSlot ++) {
      for (int iSet = 0; iSet < m_calSetID; iSet++) {

        // to be done!

      }
    }


  }

} // end Belle2 namespace
