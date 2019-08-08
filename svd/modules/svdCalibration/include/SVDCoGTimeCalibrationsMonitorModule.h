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
#include <vxd/dataobjects/VxdID.h>

#include <svd/calibration/SVDCoGTimeCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TList.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded calibration constants
   */
  class SVDCoGTimeCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDCoGTimeCalibrationsMonitorModule();

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
    TBranch* b_triggerBin = nullptr; /**< trigger bin */
    TBranch* b_alpha = nullptr; /**< alpha */
    TBranch* b_beta = nullptr; /**< beta */


    //branch variables
    int m_run = -1; /**< run number*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    int m_triggerBin = -1; /**< trigger bin*/
    float m_alpha = -1; /**< alpha */
    float m_beta = -1; /**< beta */

  private:

    SVDCoGTimeCalibrations m_CoGTimeCal; /**< CoG time calibrations payload */


  };
}

