#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <genfit/Track.h>

class TTree;
class TFile;

#include <string>

namespace Belle2 {
  /** Module to find Track correlation in cosmic events
   *
   *  One cosmic track is fitted as two tracks from the IP and the impact and
   *  track parameters are compared.
   *
   *  The module generates the file cosmic.root
   */
  class CosmicsAlignmentValidationModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    CosmicsAlignmentValidationModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~CosmicsAlignmentValidationModule();

    /** Use this to initialize resources or memory your module needs.
     *
     *  Also register any outputs of your module (StoreArrays, RelationArrays,
     *  StoreObjPtrs) here, see the respective class documentation for details.
     */
    virtual void initialize() override;

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun() override;

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event() override;

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun() override;

    /** Clean up anything you created in initialize(). */
    virtual void terminate() override;

    /** Find trackfit results in for the corresponding track
    *
    */
    const TrackFitResult* findRelatedTrackFitResult(const genfit::Track* gfTrack);

  private:
    std::string m_gfTrackColName; ///< m_gfTrackColName
    std::string m_outputFileName; ///< ouput filename string

    /** Genfit tracks. */
    StoreArray<genfit::Track> m_GenfitTracks;

    /** MC particles. */
    StoreArray<MCParticle> m_MCParticles;

    TFile* file;    /**< data file  */
    TTree* tree;    /**< data tree  */

    float t_p1 = 0;     /**< momentum p1 */
    float t_p1MC = 0;     /**< momentum in MC */
    float t_p2MC = 0;     /**< momentum in MC */
    float t_p2 = 0;     /**< momentum  */
    float t_pt1 = 0;    /**< momentum  */
    float t_pt2 = 0;    /**< momentum  */
    float t_dz = 0;     /**< member  */
    float t_dR = 0;     /**< member  */

    float t_x1 = 0;     /**< member  */
    float t_x2 = 0;     /**< member  */
    float t_y1 = 0;     /**< member  */
    float t_y2 = 0;     /**< member  */
    float t_z1 = 0;     /**< member  */
    float t_z2 = 0;     /**< member  */

    float t_px1 = 0;    /**< member  */
    float t_px2 = 0;    /**< member  */
    float t_py1 = 0;    /**< member  */
    float t_py2 = 0;    /**< member  */
    float t_pz1 = 0;    /**< member  */
    float t_pz2 = 0;    /**< member  */

    float t_D01 = 0;    /**< member  */
    float t_Phi1 = 0;     /**< member  */
    float t_Omega1 = 0;   /**< member  */
    float t_Z01 = 0;    /**< member  */
    float t_cotTheta1 = 0;  /**< member  */

    float t_D02 = 0;    /**< member  */
    float t_Phi2 = 0;     /**< member  */
    float t_Omega2 = 0;   /**< member  */
    float t_Z02 = 0;    /**< member  */
    float t_cotTheta2 = 0;  /**< member  */

  };
}
