/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <calibration/CalibrationCollectorModule.h>
#include <framework/core/ModuleParam.templateDetails.h>
#include <framework/geometry/B2Vector3.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/GblTrajectory.h>
#include <genfit/Track.h>
#include <genfit/MeasuredStateOnPlane.h>

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
     * @param addVertexPoint flag for adding the vertex point
     */
    std::vector<genfit::Track*> getParticlesTracks(std::vector<Particle*> particles, bool addVertexPoint = true);

    /** Fit given RecoTrack with GBL
     *
     * @param recoTrack A RecoTrack object to be fitted
     * @param particle Pointer to reconstructed daughter particle updated by vertex fit OR nullptr for single track
     *
     * @return true for success, false when some problems occured (or track too much down-weighted by previous DAF fit)
     */
    bool fitRecoTrack(RecoTrack& recoTrack, Particle* particle = nullptr);

    /** Compute the transformation matrix d(q/p,u',v',u,v)/d(x,y,z,px,py,pz) from state at first track point (vertex)
     * @param msop MeasuredStateOnPlane - linearization point (track state @ plane) at which the transformation should be computed
     */
    TMatrixD getGlobalToLocalTransform(const genfit::MeasuredStateOnPlane& msop);

    /** Compute the transformation matrix d(x,y,z,px,py,pz)/d(q/p,u',v',u,v) from state at first track point (vertex)
     * @param msop MeasuredStateOnPlane - linearization point (track state @ plane) at which the transformation should be computed
     */
    TMatrixD getLocalToGlobalTransform(const genfit::MeasuredStateOnPlane& msop);

    ///  Compute the transformation matrices d(q/p,u'v',u,v)/d(vx,vy,vz,px,py,pz,theta,phi,M) = dq/d(v,z) for
    ///  both particles in pair. Only for decays of type V0(*)->f+f- (same mass for f)
    ///  @param mother The mother Belle2::Particle with two daughters, its 4-momenta should already be updated by a previous vertex fit done
    ///  by modularAnalysis.
    ///  @param motherMass This function expect the assumed invariant mass of the pair. This is to allow to set artifical values
    ///  (e.g. for e+e- -> mu+mu-)
    ///  @return a pair of 5x9 matrices {dq+/d(v,z), dq-/d(v,z)}. One for each particle in list (in list order).
    ///  NOTE: The signs DO NOT refer to charges of the particles! If you want to know: (+) particle is that one which goes *along* the mother
    ///  momentum in CM system
    ///
    ///  The transformation is from local measurement system at 1st (GBL) point of each track in pair (virtual measurement
    ///  (see fitRecoTrack(..., particle) and addVertexPoint parameter of getParticlesTracks(...)) to the common parameters which
    ///  staticaly and kinematicaly describe the two-body decay:
    ///
    /// - Position of the common vertex (vy,vy,vz)
    /// - Total momentum of the pair (particles are back-to-back in their CM) (px,py,pz) and the invariant mass (M) of the decay
    /// - 2 angles describing the orientation of the decay particles in the system of the mother (CM)
    ///
    std::pair<TMatrixD, TMatrixD> getTwoBodyToLocalTransform(Particle& mother, double motherMass);

    /** Write down a GBL trajectory (to TTree or binary file) */
    void storeTrajectory(gbl::GblTrajectory& trajectory);

    /** Get the primary vertex position estimation and its size
     *  from BeamSpot
     @return tuple<B2Vector3D, TMatrixDSym> tuple with position and size as covariance matrix
     */
    std::tuple<B2Vector3D, TMatrixDSym> getPrimaryVertexAndCov() const;


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
    /** Number of external iterations of GBL fitter */
    int m_externalIterations;
    /** String defining internal GBL iterations for outlier down-weighting */
    std::string m_internalIterations;
    /** Up to which external iteration propagation Jacobians should be re-calculated */
    int m_recalcJacobians;
    /** Add local parameter for track T0 fit in GBL (local derivative) **/
    bool m_fitTrackT0;
    /** Update L/R weights from previous DAF fit result? **/
    bool m_updateCDCWeights;
    /** Minimum CDC hit weight **/
    double m_minCDCHitWeight;
    /** Minimum CDC used hit fraction **/
    double m_minUsedCDCHitFraction;
    /** Type of alignment hierarchy (for VXD only for now): 0 = None, 1 = Flat (only constraints,
         no new global parameters/derivatives), 2 = Half-Shells + sensors (no ladders), 3 = Full **/
    int m_hierarchyType;
    /** enable PXD hierarchy **/
    bool m_enablePXDHierarchy;
    /** enable SVD hierarchy **/
    bool m_enableSVDHierarchy;
    /** Enable global derivatives for wire-by-wire alignment **/
    bool m_enableWireByWireAlignment;
    /** Enable global derivatives for wire sagging **/
    bool m_enableWireSagging;

    /** List of event meta data entries at which payloads can change for timedep calibration */
    std::vector<std::tuple<int, int, int>> m_eventNumbers{};

    /** Config for time dependence: list( tuple( list( param1, param2, ... ), list( (ev, run, exp), ... )), ... */
    std::vector< std::tuple< std::vector< int >, std::vector< std::tuple< int, int, int > > > > m_timedepConfig;

    /** Map of list_name -> (mass, width) for custom mass and width setting */
    std::map<std::string,  std::tuple<double, double>> m_customMassConfig;

    /** Current vector of GBL data from trajectory to be stored in a tree */
    std::vector<gbl::GblData> m_currentGblData{};

    /** Update mass and width of the particle (mother in list) with user custom-defined values */
    void updateMassWidthIfSet(std::string listName, double& mass, double& width);
  };
}
