/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <framework/core/Module.h>

#include <svd/calibration/SVDClustering.h>
#include <svd/calibration/SVDHitTimeSelection.h>
#include <svd/calibration/SVDCoGOnlyErrorScaleFactors.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded calibration constants
   */
  class SVDClusterCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDClusterCalibrationsMonitorModule();

    /** initialize the TTrees and check validities of payloads*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /** print the payloads uniqueID and write tree and histograms to the rootfile  */
    virtual void endRun() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the mean and RMS of calibration constants */

    //branches
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_ladder = nullptr; /**< ladder number*/
    TBranch* b_layer = nullptr; /**< layer number*/
    TBranch* b_sensor = nullptr; /**< sensor number*/
    TBranch* b_side = nullptr; /**< sensor side */
    TBranch* b_clsSNR = nullptr; /**< cluster SNR */
    TBranch* b_clsSeedSNR = nullptr; /**< cluster seed SNR */
    TBranch* b_clsAdjSNR = nullptr; /**< cluster adj SNR */
    TBranch* b_clsScaleErr1 = nullptr; /**< cluster size 1 pos err scale factor SNR */
    TBranch* b_clsScaleErr2 = nullptr; /**< cluster size 2 pos err scale factor SNR */
    TBranch* b_clsScaleErr3 = nullptr; /**< cluster size 3 pos err scale factor SNR */
    TBranch* b_clsTimeFunc = nullptr; /**< cluster cut time function ID */
    TBranch* b_clsTimeMin = nullptr; /**< cluster cut min time */


    //branch variables
    int m_run = -1; /**< run number*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    float m_clsSNR = -1; /**< cluster SNR */
    float m_clsSeedSNR = -1; /**< cluster seed SNR */
    float m_clsAdjSNR = -1; /**< cluster adj SNR */
    float m_clsScaleErr1 = -1; /**< cluster size 1 pos err scale factor SNR */
    float m_clsScaleErr2 = -1; /**< cluster size 2 pos err scale factor SNR */
    float m_clsScaleErr3 = -1; /**< cluster size 3 pos err scale factor SNR */
    int m_clsTimeFunc = -1; /**< cluster cut time function version */
    float m_clsTimeMin = -1; /**< cluster cut min time */

  private:

    SVDClustering m_ClusterCal; /**< cluster calibrations payload */
    SVDHitTimeSelection m_HitTimeCut; /**< hit time cuts payload */
    SVDCoGOnlyErrorScaleFactors m_CoGOnlySF;/**< scale factors for the CoGOnly algorithm*/

    //CLUSTERS SNR CUTS
    SVDHistograms<TH1F>* m_hClsSNR = nullptr; /**< cluster SNR histo */
    SVDHistograms<TH1F>* m_hClsSeedSNR = nullptr; /**< cluster seed SNR histo */
    SVDHistograms<TH1F>* m_hClsAdjSNR = nullptr; /**< cluster adj SNR histo */

    //CLUSTER POSITION ERROR SCALE FACTORS
    SVDHistograms<TH1F>* m_hClsScaleErr1 = nullptr; /**< cluster size 1, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsScaleErr2 = nullptr; /**< cluster size 2, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsScaleErr3 = nullptr; /**< cluster size 3, position error scale factor histo */

    //CLUSTER TIME CUTS
    SVDHistograms<TH1F>* m_hClsTimeFuncVersion = nullptr; /**< cluster cut time function version histo */
    SVDHistograms<TH1F>* m_hClsTimeMin = nullptr; /**< cluster cut minimum time histo */

  };
}

