/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HITLEVELINFOWRITERMODULE_H
#define HITLEVELINFOWRITERMODULE_H

#include <reconstruction/dataobjects/DedxConstants.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/Track.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/core/Module.h>

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

  private:

    StoreArray<CDCDedxTrack> m_dedxTracks; /**< Required array of CDCDedxTracks */
    StoreArray<Track> m_tracks; /**< Required array of input Tracks */
    StoreArray<TrackFitResult> m_trackFitResults; /**< Required array of input TrackFitResults */

    /** Fill the TTree with the information from the track fit */
    void fillTrack(const TrackFitResult* fitResult);

    /** Fill the TTree with the information from a CDCDedxTrack object */
    void fillDedx(CDCDedxTrack* dedxMeas);

    /** Clear the arrays before filling an event */
    void clearEntries();

    std::string m_filename; /**< name of output ROOT file */
    bool m_correct; /**< name of output ROOT file */

    TFile* m_file; /**< output ROOT file */
    TTree* m_tree; /**< output ROOT tree */

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
    double m_eopst; /**< energy over momentum in the calorimeter */
    double m_PDG;        /**< MC PID */
    //    double m_motherPDG; /**< MC PID of mother particle */
    //    double m_pTrue;     /**< MC true momentum */
    //    double m_trackDist; /**< the total distance traveled by the track */

    // calibration constants
    double m_coscor;    /**< calibration cosine correction */
    double m_coscorext;    /**< calibration cosine correction */
    double m_rungain;   /**< calibration run gain */

    // track level dE/dx measurements
    double m_mean;  /**< dE/dx averaged */
    double m_trunc; /**< dE/dx averaged, truncated mean, with corrections */
    double m_truncorig; /**< dE/dx averaged, truncated mean */
    double m_error; /**< standard deviation of the truncated mean */
    double m_chipi; /**< chi value for pion hypothesis */

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
    double h_twodcor[kMaxHits];   /**< calibration 2D correction */
    double h_onedcor[kMaxHits];   /**< calibration 1D cleanup correction */
  };
} // Belle2 namespace
#endif
