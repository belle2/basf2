/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: jvbennett                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationCollectorModule.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>

#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxMomentumCor.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>

#include <vector>

namespace Belle2 {
  /**
   * A collector module for CDC dE/dx electron calibrations
   *
   */
  class CDCDedxElectronCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    CDCDedxElectronCollectorModule();

    /**
     * Create and book ROOT objects
     */
    virtual void prepare();

    /**
     * Create and book DB objects
     */
    virtual void startRun();

    /**
     * Fill ROOT objects
     */
    virtual void collect();


  private:

    // required input
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array for CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array for Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array for TrackFitResults */

    bool m_cuts; /**< Whether to apply cleanup cuts */
    bool m_momCor; /**< Whether to apply momentum correction */
    bool m_useDBMomCor; /**< Whether to use momentum correction in DB */
    bool m_scaleCor; /**< Whether to apply scale correction */
    bool m_cosineCor; /**< Whether to apply cosine correction */

    // db objects
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxMomentumCor> m_DBMomentumCor; /**< Momentum correction for cosmics */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGains; /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */

    CDCDedxMomentumCor m_MomentumCor; /**< Momentum correction for cosmics */
    CDCDedxWireGain m_WireGains; /**< Wire gain DB object */
    CDCDedxRunGain m_RunGains; /**< Run gain DB object */
    CDCDedxCosineCor m_CosineCor; /**< Electron saturation correction DB object */
    CDCDedx2DCell m_2DCell; /**< 2D correction DB object */
    CDCDedx1DCell m_1DCell; /**< 1D correction DB object */

    // module params
    int m_maxNumHits; /**< maximum number of hits allowed */

    // track level information
    double m_dedx = -1;  /**< dE/dx truncated mean */
    double m_costh = -1; /**< track cos(theta) */
    double m_p = -1; /**< track momentum */
    int m_nhits = -1;    /**< number of dE/dx hits on the track */

    // hit level information
    std::vector<int> m_wire;       /**< wire number for the hit */
    std::vector<int> m_layer;      /**< continuous layer number for the hit */
    std::vector<double> m_doca;    /**< distance of closest approach for the hit */
    std::vector<double> m_enta;    /**< entrance angle for the hit */
    std::vector<double> m_dedxhit; /**< dE/dx for the hit */
  };
}
