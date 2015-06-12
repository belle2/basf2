#ifndef COSMICS_VAL_H
#define COSMICS_VAL_H

#include <framework/core/Module.h>
class TH1;
class TH2;
class TNtuple;
class TTree;
class TFile;
#include <mdst/dataobjects/TrackFitResult.h>
#include <genfit/Track.h>

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
    virtual void initialize();

    /** Called once before a new run begins.
     *
     * This method gives you the chance to change run dependent constants like alignment parameters, etc.
     */
    virtual void beginRun();

    /** Called once for each event.
     *
     * This is most likely where your module will actually do anything.
     */
    virtual void event();

    /** Called once when a run ends.
     *
     *  Use this method to save run information, which you aggregated over the last run.
     */
    virtual void endRun();

    /** Clean up anything you created in initialize(). */
    virtual void terminate();

    /** Find trackfit results in for the corresponding track
    *
    */
    const TrackFitResult* findRelatedTrackFitResult(const genfit::Track* gfTrack);

  private:
    std::string m_gfTrackColName; /* m_gfTrackColName  */
    std::string m_outputFileName;  /* ouput filename string  */
    TFile* file; /* data file  */
    TTree* tree; /* data tree  */
    float t_p1;  /* momentum p1 */
    float t_p1MC; /* momentum  */
    float t_p2MC; /* momentum  */
    float t_p2; /* momentum  */
    float t_pt1; /* momentum  */
    float t_pt2; /* momentum  */
    float t_dz; /* member  */
    float t_dR; /* member  */

    float t_x1; /* member  */
    float t_x2; /* member  */
    float t_y1; /* member  */
    float t_y2; /* member  */
    float t_z1; /* member  */
    float t_z2; /* member  */

    float t_px1; /* member  */
    float t_px2; /* member  */
    float t_py1; /* member  */
    float t_py2; /* member  */
    float t_pz1;/* member  */
    float t_pz2; /* member  */

    float t_D01; /* member  */
    float t_Phi1; /* member  */
    float t_Omega1; /* member  */
    float t_Z01; /* member  */
    float t_cotTheta1; /* member  */

    float t_D02; /* member  */
    float t_Phi2; /* member  */
    float t_Omega2; /* member  */
    float t_Z02; /* member  */
    float t_cotTheta2; /* member  */

  };
}
#endif
