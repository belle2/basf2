/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MILLEPEDECOLLECTORMODULE_H
#define MILLEPEDECOLLECTORMODULE_H

#include <calibration/CalibrationCollectorModule.h>
#include <genfit/Track.h>
#include <genfit/StateOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <analysis/dataobjects/Particle.h>
#include <genfit/GblTrajectory.h>

#include <tracking/dataobjects/RecoTrack.h>

namespace Belle2 {
  /**
   * Calibration data collector for Millepede Algorithm
   *
   * Collects data from GBL-fitted tracks and produces binary files for Millepede
   *
   */
  class MillepedeCollectorModule : public CalibrationCollectorModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    MillepedeCollectorModule();

    /** Prepration */
    virtual void prepare() override;

    /** Data collection */
    virtual void collect() override;

    /** Only for closing mille binaries after each run */
    virtual void closeRun() override;

    /** Register mille binaries in file catalog */
    virtual void finish() override;

    /** Make a name for mille binary (encodes module name + starting exp, run and event + process id) */
    std::string getUniqueMilleName();

    /**
     * Get all useable tracks for particles
     *
     * @param particles vector of Belle2::Particles to be changed in vector of genfit::Tracks
     */
    std::vector<genfit::Track*> getParticlesTracks(std::vector<Particle*> particles, bool addVertexPoint = true);

    /** Fit given RecoTrack with GBL
     *
     * @param particles vector of Belle2::Particles to be changed in vector of genfit::Tracks
     * @param particle Pointer to reconstructed daughter particle updated by vertex fit OR nullptr for single track
     *
     * @return true for success, false when some problems occured (or track too much down-weighted by previous DAF fit)
     */
    bool fitRecoTrack(RecoTrack& recoTrack, Particle* particle = nullptr);

    /** Compute the transformation matrix d(q/p,u',v',u,v)/d(x,y,z,px,py,pz) from state at first track point (vertex) */
    TMatrixD getGlobalToLocalTransform(const genfit::MeasuredStateOnPlane& msop);

    /** Compute the transformation matrix d(x,y,z,px,py,pz)/d(q/p,u',v',u,v) from state at first track point (vertex) */
    TMatrixD getLocalToGlobalTransform(const genfit::MeasuredStateOnPlane& msop);

    /** Write down a GBL trajectory (to TTree or binary file) */
    void storeTrajectory(gbl::GblTrajectory& trajectory);

    /** d(Px,Py,Pz)/d(vx,vy,vz,px,py,pz,theta,phi,M) **/
    std::pair<TMatrixD, TMatrixD> getLocalToCommonTwoBodyExtParametersTransform(Particle& mother, double motherMass);


  private:
    /** Names of arrays with single RecoTracks fitted by GBL */
    std::vector<std::string> m_tracks;
    /** Names of particle list with single particles */
    std::vector<std::string> m_particles;
    /** Name of particle list with mothers of daughters to be used with vertex constraint in calibration */
    std::vector<std::string> m_vertices;
    /** Name of particle list with mothers of daughters to be used with vertex + IP profile (+ optional calibration) constraint in calibration */
    std::vector<std::string> m_primaryVertices;
    /** Name of particle list with mothers of daughters to be used with vertex + mass constraint in calibration */
    std::vector<std::string> m_twoBodyDecays;
    /** Name of particle list with mothers of daughters to be used with vertex + IP profile (+ optional calibration) + IP kinematics (+ optional calibration) constraint in calibration */
    std::vector<std::string> m_primaryTwoBodyDecays;
    /** Name of particle list with mothers of daughters to be used with vertex + IP profile + mass constraint in calibration */
    std::vector<std::string> m_primaryMassTwoBodyDecays;
    /** Name of particle list with mothers of daughters to be used with vertex + IP profile + mass constraint in calibration */
    std::vector<std::string> m_primaryMassVertexTwoBodyDecays;

    /** Width (in GeV/c/c) to use for invariant mass constraint for 'stable' particles (like K short). Temporary until proper solution is found */
    double m_stableParticleWidth;

    /** Use double (instead of single/float) precision for binary files */
    bool m_doublePrecision;
    /** Add derivatives for beam spot vertex calibration for primary vertices */
    bool m_calibrateVertex;
    /** Add derivatives for beam spot kinematics calibration for primary vertices */
    bool m_calibrateKinematics = true;
    /** Minimum p.value for output */
    double m_minPValue;
    /** Whether to use TTree to accumulate GBL data instead of binary files*/
    bool m_useGblTree;
    /** Use absolute path to locate binary files in MilleData */
    bool m_absFilePaths;
    /** Whether to use VXD alignment hierarchy*/
    std::vector<std::string> m_components{};

    /** Current vector of GBL data from trajectory to be stored in a tree */
    std::vector<gbl::GblData> m_currentGblData{};

    /** Number of external iterations of GBL fitter */
    int m_externalIterations;
    /** String defining internal GBL iterations for outlier down-weighting */
    std::string m_internalIterations;
    /** Up to which external iteration propagation Jacobians should be re-calculated */
    int m_recalcJacobians;

    /** Add local parameter for event T0 fit in GBL **/
    bool m_fitEventT0;
    /** Update L/R weights from previous DAF fit result? **/
    bool m_updateCDCWeights;
    /** Minimum CDC hit weight **/
    double m_minCDCHitWeight;
    /** Minimum CDC used hit fraction **/
    double m_minUsedCDCHitFraction;

  };
}

#endif /* MILLEPEDECOLLECTORMODULE_H */