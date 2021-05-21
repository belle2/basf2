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

#include <svd/calibration/SVDCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleCoGTimeCalibrations.h>
#include <svd/calibration/SVD3SampleELSTimeCalibrations.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded calibration constants
   */
  class SVDTimeCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDTimeCalibrationsMonitorModule();

    /** initialize the TTrees and check validities of payloads*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /** print the payloads uniqueID and write tree and histograms to the rootfile  */
    virtual void endRun() override;

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */
    std::string m_timeAlgo;   /**< time algorithms */

    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the mean and RMS of calibration constants */

    //branches
    TBranch* b_exp = nullptr; /**< exp number*/
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_ladder = nullptr; /**< ladder number*/
    TBranch* b_layer = nullptr; /**< layer number*/
    TBranch* b_sensor = nullptr; /**< sensor number*/
    TBranch* b_side = nullptr; /**< sensor side */
    TBranch* b_triggerBin = nullptr; /**< trigger bin */
    TBranch* b_c0 = nullptr; /**< calibration function c0 */
    TBranch* b_c1 = nullptr; /**< calibration function c1 */
    TBranch* b_c2 = nullptr; /**< calibration function c2 */
    TBranch* b_c3 = nullptr; /**< calibration function c3 */

    //branch variables
    int m_run = -1; /**< run number*/
    int m_exp = -1; /**< exp number*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    int m_triggerBin = -1; /**< trigger bin*/
    float m_c0 = -1; /**< calibration function c0 */
    float m_c1 = -1; /**< calibration function c1 */
    float m_c2 = -1; /**< calibration function c2 */
    float m_c3 = -1; /**< calibration function c3 */


  private:

    SVDCoGTimeCalibrations m_CoG6TimeCal; /**< CoG6 time calibrations payload */
    SVD3SampleCoGTimeCalibrations m_CoG3TimeCal; /**< CoG3 time calibrations payload */
    SVD3SampleELSTimeCalibrations m_ELS3TimeCal; /**< ELS3 time calibrations payload */


  };
}

