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
//#include <top/dataobjects/TOPDigit.h>
//#include <framework/datastore/StoreObjPtr.h>
//#include <framework/datastore/StoreArray.h>
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
//#include <top/modules/TOPDataQualityOnline/TOPDQStat.h>

namespace Belle2 {
  /**
   * Module for the comparison of different sets of time base correction (TBC) constants and to produce monitoring plots out of a given set.
   * The input constants are given in the histogram format provided by the TOPTimeCalibrator module.
   * The user can specify several calibration sets that are going to be compared each other indicating the folder in which the root files with the histograms are stored.
   * The modules i sensitive to the name format of the input files. They must follow the standard naming tbcSlotXX_Y-scrodZZZ.root, where XX is ste slot number, Y is the BS number, and ZZZ is the scrodID.
   * The list of calibration sets has to be given in a separate txt file, in the format
   *
   * CalibrationSetFolder  Label
   *
   * Where CalibrationSetFolder is the absolute path to the folder containing the histograms of the calibraiton set, and Label is a string defined
   * by the used to identify the set in the output histograms.
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
     * End-of-run action
     */
    void endRun();

    /**
     * Termination action
     */
    void terminate();

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
     * Analyzes the calibrations stored in the file m_calSetFile. This is the main function in which the analysy and the hisogram filling takes place.
     * Returns an error if m_slotID, m_boardstackID or m_scrodID are not properly initialized
     */
    int analyzeCalFile();

    /**
     * Last function to be called, compared the histograms of different datasets filled by analyzeCalFile()
     */
    int makeComparisons();


  private:
    // steering parameters
    std::string m_inputDirectoryList =
      ""; /**< List of the directories (one per IOV) in which the files with the calibration constants of the SCODS are stored (i.e. the output of the TBC production module)*/
    bool m_compareToPreviousSet =
      true; /**< Determines if the reverence set for the ratio is the first CalSet of the list (if false) or if each CalSet is compared to the previous one (if true) */
    std::string m_outputFile = ""; /**< File in which the output histograms are stored */
    short m_minCalPulses = 200; /** < Minimum number of calpulses to declare a sample as non-empty> */
    short m_numSamples = 256; /**< Number of samples that have been calibrated*/


    // utilities
    std::string m_calSetDirectory; /**< Label to be used to indetify the histograms of a the calibration set*/
    TFile*  m_calSetFile =
      nullptr; /**< File containing the calibration constants of the SCROD being analyzed.*/
    std::string m_calSetLabel; /**< Label to be used to identify the histograms of a the calibration set. */
    short m_slotID = -1; /**< ID of the slot whose calibrations are being analyzed*/
    short m_boardstackID = -1; /**< ID of the slot whose calibrations are being analyzed*/
    short m_scrodID = -1; /**< ID of the scrod  whose calibrations are being analyzed*/
    short m_calSetID = 0; /** < Internal ID of the calibration set that is being analyzed */
    short m_totCalSets = 0; /** < Total number of calibration sets, as counted int defineHistos */


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



  };

} //namespace Belle2
