/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Umberto Tamponi (tamponi@to.infn.it)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include "TH1F.h"
#include "TTree.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TMultiGraph.h"
#include <string>
#include <set>
#include <map>
#include <vector>
#include <deque>
#include <utility>

namespace Belle2 {
  /**
   * Module for the comparison of different sets of time base correction (TBC) constants and to produce monitoring plots out of a given set.
   *
   * 1) Input format
   * The input constants are given in the histogram format provided by the TOPTimeCalibrator module. According to the current (2017)
   * TBC production standards, all the root files belonging to the same calibration set must be locaded in one single directory, with no more than one level of subdirectories.
   * An example of a valid directory stucture for the input is:
   *
   * CalibrationSetRootFolder/tbc_ch0/
   *                          tbc_ch1/
   *                          tbc_ch2/
   *                          ...
   *                          tbc_ch7/
   *
   * where the root files are located under tbc_chX/
   * The module is sensitive to the name format of the input files, since slot, BS and scrod ID are parsed from them.
   * The naming is tbcSlotXX_Y-scrodZZZ.root, where XX is the slot number, Y is the BS number, and ZZZ is the scrodID. Both 2- and 3-digits scrodID are allowed.
   *
   * To specify the calibration sets to be analyzed and compared, the user must provide a text file containing the absolute path to the CalibrationSetRootFolder, and a label to identify the calibration set.
   * Separate sets are specified on separate lines of this configuration file:
   *
   * /absolute/path/to/CalibrationSetRootFolder1/  LabelSet1
   * /absolute/path/to/CalibrationSetRootFolder2/  LabelSet2
   * /absolute/path/to/CalibrationSetRootFolder3/  LabelSet3
   *
   *
   * 2) Basic functions
   * - The output histograms are declared as private members, and are initialized in the defineHisto() function
   * - The main loop that runs over all the datasets and all the rootfiles is implemented in endRun(), that calls the two main functions of this module,
   *   analyzeCalFile() and  makeComparisons().
   * - First all the calset-by-calset histograms are filled, then the comparisons are done by taking ratios of these histograms
   * - The output writing is done in the terminate() function
   * - The histogram filling is performed in the analyzeFile() function, called in the main loop over the calibration sets inside endRun().
   * - The histogram comparison is performed in the makeComparisons() function, called in endRun() outside the main loop over the calibration sets.
   *
   *
   * 3) How to add a new histogram
   * - Declare it as private member.
   * - Initialize it in defineHisto()
   * - Fill it in analyzeCalFile() (if it is a simple monitoring plot) or in  makeComparisons() (if it compares plots from different datasets)
   * - Write it in the output file in terminate()
   *
   *
   * 4) How to add a new comparison histogram
   * - Create an histogram array (hQuantity) that saves the quantity you want calset-by-calset, and fill it in analyzeCalFile()
   * - Create an histogram to save the comparson (hQuantityCom), and fill it in the makeComparisons() usig the hQuantity histograms. You can use the calculateHistoRatio() utility to do that.
   */
  class TOPTBCComparatorModule : public HistoModule {
  public:
    /**
     * Constructor
     */
    TOPTBCComparatorModule();

    /**
     * Destructor
     */
    ~TOPTBCComparatorModule() {};

    /**
     * Defining the histograms. Reads once the  m_inputDirectoryList to initialize the proper amount of histograms.
     * Every new histogram added to the module has to be initialized here.
     */
    void defineHisto();

    /**
     * Initialize the module
     */
    void initialize();

    /**
     * Called when entering a new run
     */
    void beginRun();

    /**
     * Event processor
     */
    void event();

    /**
     * End-of-run action.
     * The main analysis loop over the calibration sets happens here. This function has to be modified only if the directory structure
     * of the TOPTimeBaseCalibrator module output is changed
     * Both the core functions analyzeCalFile() and makeComparisons() are called here.
     */
    void endRun();

    /**
     * Termination action.
     * The historam writing takes place here
     */
    void terminate();

    /**
     * Analyzes the calibrations stored in the file m_calSetFile. This is the main function in which the analysis and the hisogram filling takes place.
     * Every new monitoring histogram added to the module has to be filled here.
     */
    int analyzeCalFile();

    /**
     * Last function to be called, compared the histograms of different datasets filled by analyzeCalFile()
     * Every new comparison histogram added to the module has to be filled here.
     */
    int makeComparisons();



    /**
     * Utility function to parse  the slot and BS id from the calibration file names
     */
    int parseSlotAndScrodIDfromFileName(std::string);

    /**
     * Utility function to get the directory name and the label from a line of the m_inputDirectoryList file
     * Sets the values of m_calSetDirectory and m_calSetLabel.
     */
    int parseInputDirectoryLine(std::string);

    /**
     * Utility function to take the ratio of two histograms using TH1::Divide(), without overwriting the
     * output name and title initialized in defineHisto().
     */
    TH1F* calculateHistoRatio(TH1F*, TH1F*, TH1F*);

    /**
     * Utility function to take the ratio of two histograms using TH2::Divide(), without overwriting the
     * output name and title initialized in defineHisto().
     */
    TH2F* calculateHistoRatio(TH2F*, TH2F*, TH2F*);




  private:
    // steering parameters
    std::string m_inputDirectoryList =
      ""; /**< List of the directories (one per IOV) in which the files with the calibration constants of the SCODS are stored (i.e. the output of the TBC production module)*/
    bool m_compareToPreviousSet =
      true; /**< Determines if the reverence set for the ratio is the first CalSet of the list (if false) or if each CalSet is compared to the previous one (if true) */
    std::string m_outputFile = ""; /**< File in which the output histograms are stored */
    short m_minCalPulses = 200; /**< Minimum number of calpulses to declare a sample as non-empty */
    short m_numSamples = 256; /**< Number of samples that have been calibrated*/


    // utilities
    std::string m_calSetDirectory; /**< Label to be used to indetify the histograms of a the calibration set*/
    TFile*  m_calSetFile =
      nullptr; /**< File containing the calibration constants of the SCROD being analyzed.*/
    std::string m_calSetLabel; /**< Label to be used to identify the histograms of a the calibration set. */
    short m_slotID = -1; /**< ID of the slot whose calibrations are being analyzed*/
    short m_boardstackID = -1; /**< ID of the slot whose calibrations are being analyzed*/
    short m_scrodID = -1; /**< ID of the scrod  whose calibrations are being analyzed*/
    short m_calSetID = 0; /**< Internal ID of the calibration set that is being analyzed */
    short m_totCalSets = 0; /**< Total number of calibration sets, as counted int defineHistos */


    // Delta T plots, slot-by-slot
    std::vector<TH1F*>
    m_slotAverageDeltaT[16]; /**< Average of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number*/
    std::vector<TH1F*>
    m_slotSigmaDeltaT[16]; /**< Standard deviation of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number*/
    std::vector<TH2F*>
    m_slotAverageDeltaTMap[16]; /**< Map of the average of the  DeltaT (time difference petween the calibraiton pulses) distribution */
    std::vector<TH2F*>
    m_slotSigmaDeltaTMap[16]; /**< Map of the Standard deviation of the  DeltaT (time difference petween the calibraiton pulses) distribution */

    // Average timing plots, all the detector at once
    std::vector<TH1F*>
    m_topAverageDeltaT; /**< Average of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number on the whole detector*/
    std::vector<TH1F*>
    m_topSigmaDeltaT; /**< Standard deviation of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number  on the whole detector*/


    // Occupancy plots, slot-by-slot
    std::vector<TH1F*>
    m_slotSampleOccupancy[16]; /**< Average number of calpulses per sample used in the minimization, as function of the channel number*/
    std::vector<TH1F*>
    m_slotEmptySamples[16]; /**< Number of (semi-)empty samples in each channel.*/
    std::vector<TH2F*>
    m_slotSampleOccupancyMap[16]; /**< Map of the average number of calpulses per sample used in the minimizat on */
    std::vector<TH2F*>
    m_slotEmptySamplesMap[16]; /**< Map of the number of (semi-)empty samples.*/

    // Occupancy plot, all the detector at once
    std::vector<TH1F*>
    m_topSampleOccupancy; /**< Average number of calpulses per sample used in the minimization, as function of the channel number on the whole detector*/


    // Delta T ratio plots, slot-by-slot
    std::vector<TH1F*>
    m_slotAverageDeltaTComparison[16]; /**< Ratio of the average of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number*/
    std::vector<TH1F*>
    m_slotSigmaDeltaTComparison[16]; /**< Ratio of the Standard deviation of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number*/
    std::vector<TH2F*>
    m_slotAverageDeltaTMapComparison[16]; /**< Map of the Ratio of the  average  DeltaT (time difference petween the calibraiton pulses) */
    std::vector<TH2F*>
    m_slotSigmaDeltaTMapComparison[16]; /**< Map of Ratio of the Standard deviation on  DeltaT (time difference petween the calibraiton pulses) */


    // Delta T ratio plots, whole detector
    std::vector<TH1F*>
    m_topAverageDeltaTComparison; /**< Average of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number on the whole detector*/
    std::vector<TH1F*>
    m_topSigmaDeltaTComparison; /**< Standard deviation of the DeltaT (time difference petween the calibraiton pulses) distribution, as function of the channel number  on the whole detector*/
    std::vector<TH1F*>
    m_topSampleOccupancyComparison; /**< Ratios of the average sample occupancy on the whole detector */





  };

} //namespace Belle2
