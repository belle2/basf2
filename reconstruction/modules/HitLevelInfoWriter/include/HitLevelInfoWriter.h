/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/DedxConstants.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <genfit/Track.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBArray.h>
#include <framework/core/Module.h>

#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <string>
#include <vector>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>

class TH2F;

namespace Belle2 {

  class CDCDedxTrack;

  /** Extracts dE/dx information for calibration testing. Writes a ROOT file.
   */
  class HitLevelInfoWriterModule : public Module {

  public:

    /** Default constructor */
    HitLevelInfoWriterModule();

    /** Destructor */
    virtual ~HitLevelInfoWriterModule();

    /** Initialize the module */
    virtual void initialize();

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event();

    /** End of the event processing. */
    virtual void terminate();

    /** Create the output TFiles and TTrees. */
    void bookOutput(std::string filename);

  private:

    std::string m_strOutputBaseName; /**< Base name for the output ROOT files */
    std::vector<std::string> m_strParticleList; /**< Vector of ParticleLists to write out */
    std::vector<std::string> m_filename; /**< full names of the output ROOT files */
    std::vector<TFile*> m_file; /**< output ROOT files */
    std::vector<TTree*> m_tree; /**< output ROOT trees */

    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array of CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array of input Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array of input TrackFitResults */
    StoreArray<ECLCluster> m_eclClusters; /**< Required array of input ECLClusters */
    StoreArray<KLMCluster> m_klmClusters; /**< Required array of input KLMClusters */

    /** Fill the TTree with the information from the track fit */
    void fillTrack(const TrackFitResult* fitResult);

    /** Fill the TTree with the information from a CDCDedxTrack object */
    void fillDedx(CDCDedxTrack* dedxMeas);

    /** Clear the arrays before filling an event */
    void clearEntries();

    // event level information (from emd)
    int m_expID; /**< experiment in which this Track was found */
    int m_runID; /**< event in which this Track was found */
    int m_eventID; /**< event in which this Track was found */

    // track level information (from tfr)
    double m_phi;      /**< phi for the track */
    double m_vx0; /**< X coordinate of track POCA to origin */
    double m_vy0; /**< Y coordinate of track POCA to origin */
    double m_vz0; /**< Z coordinate of track POCA to origin */
    double m_d0; /**< DOCA in r-phi for track at origin */
    double m_z0; /**< z position of track at origin */
    double m_chi2; /**< chi^2 from track fit */

    // track level information (from cdt)
    int m_trackID; /**< ID number of the Track */
    double m_length; /**< total path length of the Track */
    int m_charge; /**< the charge for this Track */
    double m_cosTheta; /**< cos(theta) for the track */
    double m_p;        /**< momentum valid in CDC */
    double m_PDG;        /**< MC PID */
    //    double m_motherPDG; /**< MC PID of mother particle */
    //    double m_pTrue;     /**< MC true momentum */
    //    double m_trackDist; /**< the total distance traveled by the track */
    double m_ioasym; /**< asymmetry in increasing vs decreasing layer numbers per track */

    // other track level information
    double m_eop; /**< energy over momentum in the calorimeter */
    double m_klmLayers; /**< number of klm layers with hits */

    // calibration constants
    double m_scale;    /**< calibration scale factor */
    double m_cosCor;    /**< calibration cosine correction */
    double m_runGain;   /**< calibration run gain */

    // track level dE/dx measurements
    double m_mean;  /**< dE/dx averaged */
    double m_trunc; /**< dE/dx averaged, truncated mean, with corrections */
    double m_error; /**< standard deviation of the truncated mean */

    // PID values
    double m_chie; /**< chi value for electron hypothesis */
    double m_chimu; /**< chi value for muon hypothesis */
    double m_chipi; /**< chi value for pion hypothesis */
    double m_chik; /**< chi value for kaon hypothesis */
    double m_chip; /**< chi value for proton hypothesis */
    double m_chid; /**< chi value for deuteron hypothesis */

    static const int kMaxHits = 200; /**< default hit level index */

    // layer level information
    int l_nhits; /**< the total number of layer hits for this Track */
    int l_nhitsused; /**< the total number of layer hits used for this Track */
    int l_nhitscombined[kMaxHits]; /**< the number of hits combined this layer */
    int l_wirelongesthit[kMaxHits]; /**< the wire number of longest hit in this layer */
    int l_layer[kMaxHits];     /**< layer number */
    double l_path[kMaxHits];   /**< distance travelled in this layer */
    double l_dedx[kMaxHits]; /**< dE/dx for this layer */

    // hit level information (references on nhits)
    int h_nhits; /**< the number of good hits for this Track */
    int h_lwire[kMaxHits];  /**< sense wire within layer */
    int h_wire[kMaxHits];  /**< sense wire ID */
    int h_layer[kMaxHits]; /**< layer number */

    double h_path[kMaxHits];   /**< path length in cell */
    double h_dedx[kMaxHits];   /**< charge per path length */
    double h_adcraw[kMaxHits]; /**< charge per hit */
    double h_doca[kMaxHits];   /**< distance of closest approach */
    double h_enta[kMaxHits];   /**< entrance angle */
    double h_driftT[kMaxHits];   /**< drift time */
    double h_wireGain[kMaxHits];   /**< calibration hit gain */
    double h_twodCor[kMaxHits];   /**< calibration 2D correction */
    double h_onedCor[kMaxHits];   /**< calibration 1D cleanup correction */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */
  };
} // Belle2 namespace
