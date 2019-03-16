/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Jitendra Kumar
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
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** End of the event processing. */
    virtual void terminate() override;

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
    int m_expID{ -1};   /**< experiment in which this Track was found */
    int m_runID{ -1};   /**< event in which this Track was found */
    int m_eventID{ -1}; /**< event in which this Track was found */

    // track level information (from tfr)
    double m_phi{ -1.};  /**< phi for the track */
    double m_vx0{0.};    /**< X coordinate of track POCA to origin */
    double m_vy0{0.};    /**< Y coordinate of track POCA to origin */
    double m_vz0{0.};    /**< Z coordinate of track POCA to origin */
    double m_d0{0.};     /**< DOCA in r-phi for track at origin */
    double m_z0{0.};     /**< z position of track at origin */
    double m_chi2{ -1.}; /**< chi^2 from track fit */

    // track level information (from cdt)
    int m_trackID{ -1};     /**< ID number of the Track */
    double m_length{ -1.};  /**< total path length of the Track */
    int m_charge{0};        /**< the charge for this Track */
    double m_cosTheta{ -2.}; /**< cos(theta) for the track */
    double m_pIP{ -1.};     /**< IP momentum */
    double m_p{ -1.};       /**< momentum valid in CDC */
    double m_PDG{ -1.};     /**< MC PID */
    //    double m_motherPDG; /**< MC PID of mother particle */
    //    double m_pTrue;     /**< MC true momentum */
    //    double m_trackDist; /**< the total distance traveled by the track */
    double m_ioasym{ -1.};  /**< asymmetry in increasing vs decreasing layer numbers per track */

    // other track level information
    double m_eop{ -1.};       /**< energy over momentum in the calorimeter */
    double m_klmLayers{ -1.}; /**< number of klm layers with hits */

    // calibration constants
    double m_scale{ -1.};   /**< calibration scale factor */
    double m_cosCor{ -1.};  /**< calibration cosine correction */
    double m_runGain{ -1.}; /**< calibration run gain */

    // track level dE/dx measurements
    double m_mean{ -1.};       /**< dE/dx averaged */
    double m_trunc{ -1.};      /**< dE/dx averaged, truncated mean, with corrections */
    double m_truncNoSat{ -1.}; /**< dE/dx averaged, truncated mean, with corrections (not hadron) */
    double m_error{ -1.};      /**< standard deviation of the truncated mean */

    // PID values
    double m_chie{ -1.};  /**< chi value for electron hypothesis */
    double m_chimu{ -1.}; /**< chi value for muon hypothesis */
    double m_chipi{ -1.}; /**< chi value for pion hypothesis */
    double m_chik{ -1.};  /**< chi value for kaon hypothesis */
    double m_chip{ -1.};  /**< chi value for proton hypothesis */
    double m_chid{ -1.};  /**< chi value for deuteron hypothesis */

    static const int kMaxHits = 200; /**< default hit level index */

    // layer level information
    int l_nhits{ -1};                    /**< the total number of layer hits for this Track */
    int l_nhitsused{ -1};                /**< the total number of layer hits used for this Track */
    int l_nhitscombined[kMaxHits] = {};  /**< the number of hits combined this layer */
    int l_wirelongesthit[kMaxHits] = {}; /**< the wire number of longest hit in this layer */
    int l_layer[kMaxHits] = {};          /**< layer number */
    double l_path[kMaxHits] = {};        /**< distance travelled in this layer */
    double l_dedx[kMaxHits] = {};        /**< dE/dx for this layer */

    // hit level information (references on nhits)
    int h_nhits{ -1};           /**< the number of good hits for this Track */
    int h_lwire[kMaxHits] = {}; /**< sense wire within layer */
    int h_wire[kMaxHits] = {};  /**< sense wire ID */
    int h_layer[kMaxHits] = {}; /**< layer number */

    double h_path[kMaxHits] = {};   /**< path length in cell */
    double h_dedx[kMaxHits] = {};   /**< charge per path length */
    double h_adcraw[kMaxHits] = {}; /**< charge per hit */
    double h_doca[kMaxHits] = {};   /**< distance of closest approach */
    double h_ndoca[kMaxHits] = {};  /**< normalized distance of closest approach */
    double h_docaRS[kMaxHits] = {}; /**< normalized +RS distance of closest approach */
    double h_enta[kMaxHits] = {};     /**< entrance angle */
    double h_entaRS[kMaxHits] = {};   /**< normalized + RS distance of entrance angle */
    double h_driftT[kMaxHits] = {};   /**< drift time */
    double h_wireGain[kMaxHits] = {}; /**< calibration hit gain */
    double h_twodCor[kMaxHits] = {};  /**< calibration 2D correction */
    double h_onedCor[kMaxHits] = {};  /**< calibration 1D cleanup correction */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains;      /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain;         /**< Run gain DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor;     /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */

    bool IsHitLevel; /**< Flag to switch on/off hit level information */

  };
} // Belle2 namespace
