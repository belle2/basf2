/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/datastore/StoreArray.h>

#include <pxd/dataobjects/PXDRawHit.h>

#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Corr Module */
  class PXDRawDQMCorrModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDRawDQMCorrModule();

  private:
    /** Initialize */
    void initialize() override final;
    /** Begin run */
    void beginRun() override final;
    /** Event */
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2()TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:

    std::string m_storeRawHitsName;             /**< PXDRawHits StoreArray name */
    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** Storearray for raw pixels   */
    StoreArray<PXDRawHit> m_storeRawHits;

    TH2F* m_CorrelationU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationV = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaV = nullptr;          /**< Correlation Sensor 1 vs 2 */

    TH2F* m_In1CorrelationU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH2F* m_In1CorrelationV = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_In1DeltaU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_In1DeltaV = nullptr;          /**< Correlation Sensor 1 vs 2 */

    TH2F* m_In2CorrelationU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH2F* m_In2CorrelationV = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_In2DeltaU = nullptr;          /**< Correlation Sensor 1 vs 2 */
    TH1F* m_In2DeltaV = nullptr;          /**< Correlation Sensor 1 vs 2 */

  };
}

