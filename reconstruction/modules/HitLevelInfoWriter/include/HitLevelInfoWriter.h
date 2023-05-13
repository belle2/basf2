/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>

#include <genfit/Track.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedxInjectionTime.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <reconstruction/dbobjects/CDCDedxInjectionTime.h>
#include <reconstruction/dbobjects/CDCDedxADCNonLinearity.h> //new in rel5
#include <reconstruction/dbobjects/CDCDedxCosineEdge.h> //new in rel5
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dataobjects/DedxConstants.h>

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/ReferenceFrame.h>

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

    /**
     * Function to recalculate the dedx with latest constants
     **/
    void recalculateDedx(CDCDedxTrack* dedxTrack, std::map<int, std::vector<double>>& l_var,
                         double (&cdcChi)[Const::ChargedStable::c_SetSize]);

    /**
     * Function to get the correction factor
     **/
    double GetCorrection(int& adc, int layer, int wireID, double doca, double enta, double costheta, double ring, double time) const;

    /**
     * Function to apply the hadron correction
     **/
    void HadronCorrection(double costheta, double& dedx) const;

    /**
     * hadron saturation parameterization part 2
     **/
    double D2I(const double cosTheta, const double D) const;

    /**
     * hadron saturation parameterization part 1
     **/
    double I2D(const double cosTheta, const double I) const;

    /** Save arithmetic and truncated mean for the 'dedx' values.
       *
       * @param mean              calculated arithmetic mean
       * @param truncatedMean     calculated truncated mean
       * @param truncatedMeanErr  error for truncatedMean
       * @param dedx              input values
       */
    void calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const;

    /** for all particles, save chi values into 'chi'
    * chi array of chi values to be modified
    **/
    void saveChiValue(double(&chi)[Const::ChargedStable::c_SetSize], CDCDedxTrack* dedxTrack, double dedx) const;

  private:

    std::string m_strOutputBaseName; /**< Base name for the output ROOT files */
    std::vector<std::string> m_strParticleList; /**< Vector of ParticleLists to write out */
    std::vector<std::string> m_filename; /**< full names of the output ROOT files */
    std::vector<TFile* > m_file;  /**< output ROOT files */
    std::vector<TTree* > m_tree;  /**< output ROOT trees */

    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array of CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array of input Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array of input TrackFitResults */
    StoreArray<ECLCluster> m_eclClusters; /**< Required array of input ECLClusters */
    StoreArray<KLMCluster> m_klmClusters; /**< Required array of input KLMClusters */
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo; /**< Store Object Ptr: EventLevelTriggerTimeInfo */

    /** Fill the TTree with the information from the track fit */
    void fillTrack(const TrackFitResult* fitResult);

    /** Fill the TTree with the information from a CDCDedxTrack object */
    void fillDedx(CDCDedxTrack* dedxTrack);

    /** Clear the arrays before filling an event */
    void clearEntries();

    // event level information (from emd)
    int m_expID{ -1};   /**< experiment in which this Track was found */
    int m_runID{ -1};   /**< run in which this Track was found */
    int m_eventID{ -1}; /**< event in which this Track was found */

    double m_injring{ -1.0}; /**< HER injection status */
    double m_injtime{ -1.0}; /**< time since last injection in micro seconds */

    // track level information (from tfr)
    double m_d0{0.};     /**< Signed distance to the POCA in the r-phi plane */
    double m_z0{0.};     /**< z coordinate of the POCA */
    double m_dz{ -1.};  /**< vertex or POCA in case of tracks z in respect to IPs */
    double m_dr{ -1.};  /**< track d0 relative to IP */
    double m_dphi{ -1.};  /**< POCA in degrees in respect to IP */
    double m_vx0{0.};    /**< X coordinate of track POCA to origin */
    double m_vy0{0.};    /**< Y coordinate of track POCA to origin */
    double m_vz0{0.};    /**< Z coordinate of track POCA to origin */
    double m_tanlambda{ -1.};  /**< Slope of the track in the r-z plane*/
    double m_phi0{ -1.};  /**< Angle of the transverse momentum in the r-phi plane */
    double m_chi2{ -1.}; /**< chi^2 from track fit */

    double m_nCDChits{ -1.};  /**< Number of CDC hits associated to the track */
    int m_inCDC{ -1}; /**< frack is CDC acceptance or not */
    int m_trackID{ -1};     /**< ID number of the Track */
    double m_length{ -1.};  /**< total path length of the Track */
    int m_charge{0};        /**< the charge for this Track */
    double m_cosTheta{ -2.}; /**< cos(theta) for the track */
    double m_pCDC{ -1.};       /**< momentum valid in CDC */
    double m_p{ -1.};       /**< momentum from tracking */
    double m_pt{ -1.};       /**< transverse momentum from tracking */
    double m_phi{ -1.};  /**< phi for the track */
    double m_ioasym{ -1.};  /**< asymmetry in increasing vs decreasing layer numbers per track */
    double m_theta{ -2.}; /**< cos(theta) for the track */

    // track level Mc
    double m_PDG{ -1.};     /**< MC PID */
    //    double m_motherPDG; /**< MC PID of mother particle */
    //    double m_pTrue;     /**< MC true momentum */
    //    double m_trackDist; /**< the total distance traveled by the track */

    // track level dE/dx measurements
    double m_mean{ -1.};       /**< dE/dx averaged */
    double m_trunc{ -1.};      /**< dE/dx averaged, truncated mean, with corrections */
    double m_truncNoSat{ -1.}; /**< dE/dx averaged, truncated mean, with corrections (not hadron) */
    double m_error{ -1.};      /**< standard deviation of the truncated mean */

    // other dec specific information
    double m_eop{ -1.};       /**< energy over momentum in the calorimeter */
    double m_e{ -1.};       /**< energy in the calorimeter */
    double m_e1_9{ -1.};       /**< ratio of energies of the central 1 crystal vs 3x3 crystals */
    double m_e9_21{ -1.};        /**< ratio of energies of the central 3x3 crystal vs 5x5 crystals */
    double m_klmLayers{ -1.}; /**< number of klm layers with hits */
    double m_eclsnHits{ -1.};        /**<# of clusters */

    // calibration constants
    double m_scale{ -1.};   /**< calibration scale factor */
    double m_cosCor{ -1.};  /**< calibration cosine correction */
    double m_cosEdgeCor{ -1.};  /**< calibration cosine edge correction */
    double m_runGain{ -1.}; /**< calibration run gain */
    double m_timeGain{ -1.}; /**< calibration injection time gain */
    double m_timeReso{ -1.}; /**< calibration injection time reso */

    // hadron cal and PID related variables
    double m_chieOld{ -1.};  /**< chi value for electron hypothesis */
    double m_chimuOld{ -1.}; /**< chi value for muon hypothesis */
    double m_chipiOld{ -1.}; /**< chi value for pion hypothesis */
    double m_chikOld{ -1.};  /**< chi value for kaon hypothesis */
    double m_chipOld{ -1.};  /**< chi value for proton hypothesis */
    double m_chidOld{ -1.};  /**< chi value for deuteron hypothesis */

    double m_chie{ -1.};  /**< modified chi value for electron hypothesis */
    double m_chimu{ -1.}; /**< modified chi value for muon hypothesis */
    double m_chipi{ -1.}; /**< modified chi value for pion hypothesis */
    double m_chik{ -1.};  /**< modified chi value for kaon hypothesis */
    double m_chip{ -1.};  /**< modified chi value for proton hypothesis */
    double m_chid{ -1.};  /**< modified chi value for deuteron hypothesis */

    double m_prese{ -1.};  /**< pred reso for electron hypothesis */
    double m_presmu{ -1.}; /**< pred reso for muon hypothesis */
    double m_prespi{ -1.}; /**< pred reso for pion hypothesis */
    double m_presk{ -1.};  /**< pred reso for kaon hypothesis */
    double m_presp{ -1.};  /**< pred reso for proton hypothesis */
    double m_presd{ -1.};  /**< pred reso for deuteron hypothesis */

    double m_pmeane{ -1.};  /**< pred mean for electron hypothesis */
    double m_pmeanmu{ -1.}; /**< pred mean for muon hypothesis */
    double m_pmeanpi{ -1.}; /**< pred mean for pion hypothesis */
    double m_pmeank{ -1.};  /**< pred mean for kaon hypothesis */
    double m_pmeanp{ -1.};  /**< pred mean for proton hypothesis */
    double m_pmeand{ -1.};  /**< pred mean for deuteron hypothesis */

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
    double h_adccorr[kMaxHits] = {}; /**< charge per hit corr by nonlinear ADC */
    double h_doca[kMaxHits] = {};   /**< distance of closest approach */
    double h_ndoca[kMaxHits] = {};  /**< normalized distance of closest approach */
    double h_ndocaRS[kMaxHits] = {}; /**< normalized +RS distance of closest approach */
    double h_enta[kMaxHits] = {};     /**< entrance angle */
    double h_entaRS[kMaxHits] = {};   /**< normalized + RS distance of entrance angle */
    double h_driftT[kMaxHits] = {};   /**< drift time */
    double h_driftD[kMaxHits] = {};   /**< drift distance */
    double h_facnladc[kMaxHits] = {}; /**< calibration hit gain */
    double h_wireGain[kMaxHits] = {}; /**< calibration hit gain */
    double h_twodCor[kMaxHits] = {};  /**< calibration 2D correction */
    double h_onedCor[kMaxHits] = {};  /**< calibration 1D cleanup correction */

    //Tracking variables for extra hits
    double h_WeightPionHypo[kMaxHits] = {}; /**< weight for pion hypothesis from KalmanFitterInfo*/
    double h_WeightKaonHypo[kMaxHits] = {}; /**< weight for kaon hypothesis from KalmanFitterInfo*/
    double h_WeightProtonHypo[kMaxHits] = {}; /**< weight for proton hypothesis from KalmanFitterInfo*/
    int h_foundByTrackFinder[kMaxHits] = {}; /**< the 'found by track finder' flag for the given hit */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains;      /**< Wire gain DB object */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain;         /**< Run gain DB object */
    DBObjPtr<CDCDedxInjectionTime> m_DBInjectTime; /**< time gain/reso DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor;     /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */
    DBObjPtr<CDCDedxADCNonLinearity> m_DBNonlADC; /**< hadron saturation non linearity */
    DBObjPtr<CDCDedxCosineEdge> m_DBCosEdgeCor; /**< cosine edge calibration */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< hadron saturation parameters */

    std::vector<double> m_hadronpars; /**< hadron saturation parameters */

    bool m_isDeadwire; /**< write only active wires */
    //Flag to enable and disable set of variables
    bool m_isHitLevel; /**< Flag to switch on/off hit level info */
    bool m_isExtraVar; /**< Flag to switch on/off extra level info and some available w/ release/5 only */
    bool m_isRelative; /**< Flag to switch on/off relative constants */
    bool m_isCorrection; /**< Flag to switch on/off corrections */
  };
} // Belle2 namespace
