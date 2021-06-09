/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once


#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>

#include <svd/dataobjects/SVDHistograms.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>

namespace Belle2 {
  /**
   * Module that produces a localdb with position error scale factors
   * for different position algoritms
   */
  class SVDPositionErrorScaleFactorImporterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDPositionErrorScaleFactorImporterModule();

    /** check presence of data objects*/
    virtual void initialize() override;

    /** initialize the TTrees and create histograms*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /**
     * create cumulative histograms, compute scale factors,
     * create payloads and import it to a localdb,
     * and write tree and histograms to the rootfile
     */
    virtual void endRun() override;

  private:

    /* user-defined parameters */
    std::string m_rootFileName;   /**< root file name */

    std::string m_posAlgorithm = "CoGOnly"; /**< position algorithm */

    bool m_noOutliers = false; /**< if True removes outliers from scale factor computation*/

    float m_min = -10; /**< min of the pulls histograms*/
    float m_max = 10; /**< max of the pulls histograms*/
    float m_nBins = 100; /**< number of bins of the pulls histograms*/
    std::string m_uniqueID = "PositionErrorScaleFactors_TEST"; /**< payload uniqueID*/

    StoreArray<SVDCluster> m_clusters; /**< SVDClusters StoreArray */
    StoreArray<SVDTrueHit> m_truehits; /**< SVDTrueHits StoreArray */

    //* ROOT file related parameters
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the mean and RMS of calibration constants */

    //tree variables
    int m_exp = -1; /**< exp number*/
    int m_run = -1; /**< run number*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    int m_size = -1; /**< cluster size*/
    float m_clsCharge = -1; /**< cluster charge */
    float m_clsTime = -1; /**< cluster time */
    float m_clsPos = -1; /**< cluster position error*/
    float m_clsErr = -1; /**< cluster position error*/
    float m_clsResid = -1; /**< cluster positionresidual*/
    float m_clsPull = -1; /**< cluster position pull */

    /** computes the scale factor by requiring that 68%
    of the pull distribution is between ±1
    */
    double oneSigma(TH1F*);

    //CLUSTER PULLS
    SVDHistograms<TH1F>* m_hClsPullSize1 = nullptr; /**< cluster size 1, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsPullSize2 = nullptr; /**< cluster size 2, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsPullSize3 = nullptr; /**< cluster size 3, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsPullSize4 = nullptr; /**< cluster size 4, position error scale factor histo */
    SVDHistograms<TH1F>* m_hClsPullSize5 = nullptr; /**< cluster size 5, position error scale factor histo */

    const int maxSize = 5; /**< maximum cluster size*/
    TH1F* m_hL3Pulls[5][2] = {}; /**< cumulative L3 pulls histograms [size][side]*/
    TH1F* m_hFWPulls[5][2] = {}; /**< cumulative FW pulls histograms [size][side]*/
    TH1F* m_hBWPulls[5][2] = {}; /**< cumulative BW pulls histograms [size][side]*/
    TH1F* m_hORPulls[5][2] = {}; /**< cumulative ORIGAMI pulls histograms [size][side]*/

  };
}

