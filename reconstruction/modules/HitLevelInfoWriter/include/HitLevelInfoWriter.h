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

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

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

    /** Fill the TTree with the information from the track fit */
    void fillTrack(const TrackFitResult* fitResult);

    /** Fill the TTree with the information from a CDCDedxTrack object */
    void fillDedx(CDCDedxTrack* dedxMeas);

    /** Clear the arrays before filling an event */
    void clearEntries();

    std::string m_filename; /**< name of output ROOT file */

    TFile* m_file; /**< output ROOT file */
    TTree* m_tree; /**< output ROOT tree */

    // event level information
    int m_runID; /**< event in which this Track was found */
    int m_eventID; /**< event in which this Track was found */
    int m_numTracks; /**< number of tracks for this event */

    // track level information
    int m_trackID; /**< ID number of the Track */
    int m_nlhits; /**< the total number of layer hits for this Track */
    int m_nlhitsused; /**< the total number of layer hits used for this Track */
    int m_nhits; /**< the number of good hits for this Track */

    //    double m_trackDist; /**< the total distance traveled by the track */

    double m_mean;  /**< dE/dx averaged */
    double m_trunc; /**< dE/dx averaged, truncated mean */
    double m_error; /**< standard deviation of the truncated mean */
    double m_chipi; /**< chi value for pion hypothesis */

    double m_vx0; /**< X coordinate of track POCA to origin */
    double m_vy0; /**< Y coordinate of track POCA to origin */
    double m_vz0; /**< Z coordinate of track POCA to origin */

    double m_d0; /**< DOCA in r-phi for track at origin */
    double m_z0; /**< z position of track at origin */
    double m_chi2; /**< chi^2 from track fit */

    double m_eopst; /**< energy over momentum in the calorimeter */

    double m_p;        /**< momentum at point of closest approach to origin */
    double m_phi;      /**< phi for the track */
    double m_cosTheta; /**< cos(theta) for the track */

    double m_PDG;        /**< MC PID */
    //    double m_motherPDG; /**< MC PID of mother particle */
    //    double m_pTrue;     /**< MC true momentum */

    // hit level information (references on nhits)
    static const int kMaxHits = 100; /**< default hit level index */

    int m_hitlayer[kMaxHits]; /**< layer number */
    int m_wire[kMaxHits];  /**< sense wire ID */

    double m_adcraw[kMaxHits]; /**< charge per hit */
    double m_path[kMaxHits];   /**< path length in cell */
    double m_dedx[kMaxHits];   /**< charge per path length */
    double m_doca[kMaxHits];   /**< distance of closest approach */
    double m_enta[kMaxHits];   /**< entrance angle */
    double m_driftT[kMaxHits];   /**< drift time */

    // layer level information
    double m_layer[kMaxHits];     /**< layer number */
    double m_layerdx[kMaxHits];   /**< distance travelled in this layer */
    double m_layerdedx[kMaxHits]; /**< dE/dx for this layer */

  };
} // Belle2 namespace
#endif
