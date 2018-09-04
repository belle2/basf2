/**************************************************************************
1;4205;0c * BASF2 (Belle Analysis Framework 2)                                     *
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
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/ECLCluster.h>


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
     * Fill ROOT objects
     */
    virtual void collect();

    /*** if bad phi region removal is demanded */
    void setBadPhiRejection(bool value) {IsBadPhiRej = value;}

    /*** if bad phi region removal is demanded */
    void setEoverPCutValue(double value) {fSetEoverP = value;}


  private:

    bool m_cuts; /**< Boolean to apply cuts during collection */

    // required input
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array for CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array for Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array for TrackFitResults */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */

    // module params
    int m_maxNumHits; /**< maximum number of hits allowed */
    double fSetEoverP;
    // track level information
    double m_dedx = -1;  /**< dE/dx truncated mean */
    double m_costh = -1; /**< track cos(theta) */
    double m_p = -1; /**< track momentum */
    int m_charge = 0; /**< track momentum */
    int m_run = 1;/**run number */
    int m_nhits = -1;    /**< number of dE/dx hits on the track */

    // hit level information
    std::vector<int> m_wire;       /**< wire number for the hit */
    std::vector<int> m_layer;      /**< continuous layer number for the hit */
    std::vector<double> m_doca;    /**< distance of closest approach for the hit */
    std::vector<double> m_enta;    /**< entrance angle for the hit */
    std::vector<double> m_docaRS;    /**< rescaled distance of closest approach for the hit */
    std::vector<double> m_entaRS;    /**< rescaled entrance angle for the hit */
    std::vector<double> m_dedxhit; /**< dE/dx for the hit */


    bool Iscosth; /**< flag to write costh in tree  */
    bool Isp;/**< flag to write momentum in treet */
    bool Ischarge;/**< flag to write charge in treet */
    bool Isrun;/**< flag to write run number in tree */
    bool Iswire;/**< flag to write wire number in tree */
    bool Islayer;/**< flag to write layer number in tree */
    bool Isdoca;/**< flag to write doca in tree */
    bool Isenta;/**< flag to write enta in tree */
    bool IsdocaRS;/**< flag to write rescaled doca in tree */
    bool IsentaRS;/**< flag to write rescaled enta in tree */
    bool Isdedxhit;    /**< flag to write dedxhits in tree */
    bool IsBadPhiRej;/**< flag to remove bad phi region */

  };
}
