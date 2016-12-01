//+
// File : PantherInputModule.h
// Description : A module to read Panther record in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contirbutors: Anze Zupanc, Matic Lubej,
//-

#ifndef B2BII_CONVERT_MDST_H
#define B2BII_CONVERT_MDST_H

#include <framework/core/Module.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/panther/panther_group.h"

// Belle objects (Panther tables)
#include "belle_legacy/tables/belletdf.h"
#include "belle_legacy/tables/mdst.h"

#include "belle_legacy/helix/Helix.h"

// Belle2 objects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/dbobjects/BeamParameters.h>

#include <framework/datastore/StoreArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/Const.h>

#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Geometry/Point3D.h"

using namespace CLHEP;
using namespace HepGeom;

typedef Point3D<double> HepPoint3D;

#include <string>
#include <map>

//enable ACC conversion (needs externals >= v00-06-01)
#define HAVE_KID_ACC

//enable EID conversion (needs externals > v00-07-01)
#define HAVE_EID

// enable findKs (needs externals > v00-07-01)
#define HAVE_FINDKS

// enable nisKsFinder (needs externals > v00-07-01)
#define HAVE_NISKSFINDER

namespace Belle2 {

  /**
   * Module converts Belle MDST objects (Panther records) to Belle II MDST objects.
   *
   * The module performs conversion of the following Belle Panther records:
   *
   * o) Gen_hepevt -> MCParticle
   *
   * o) Mdst_ecl and Mdst_ecl_aux -> ECLCluster
   *
   *
   * The following relations are created:
   *
   * o) ECLCluster -> MCParticle
   */

  class B2BIIConvertMdstModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    B2BIIConvertMdstModule();
    virtual ~B2BIIConvertMdstModule() override;

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:

    /**
     * Initializes Belle II DataStore.
     */
    void initializeDataStore();

    //! flag that tells whether given data sample is for real data or MC
    // true = real data
    // false = mc
    bool m_realData;

    //! flag that tells which form of covariance matrix should be used in the conversion of charged tracks
    // true = use 6x6 (position, momentum) covariance matrix
    // false = use 5x5 (helix parameters) covaraince matrix
    bool m_use6x6CovarianceMatrix4Tracks;

    //! variable to tell us which IPProfile bin was active last time we looked
    int m_lastIPProfileBin{ -1};

    //-----------------------------------------------------------------------------
    // CONVERT TABLES
    //-----------------------------------------------------------------------------

    /**
     * Reads and converts all entries of Gen_hepevt Panther table to MCParticle dataobjects and adds them to StoreArray<MCParticle>.
     */
    void convertGenHepEvtTable();

    /**
     * Reads and converts all entries of Mdst_ecl(_aux) Panther table to ECLCluster dataobjects and adds them to StoreArray<ECLCluster>.
     */
    void convertMdstECLTable();

    /**
     * Reads and converts all entries of Mdst_klm_cluster Panther table to KLMCluster dataobjects and adds them to StoreArray<KLMCluster>.
     */
    void convertMdstKLMTable();

    /**
     * Reads and converts all entries of Mdst_charged (Mdst_trk and Mdst_trk_fit) Panther table to Track (TrackFitResult) dataobjects and adds them to StoreArray<Track> (StoreArray<TrackFitResult>).
     */
    void convertMdstChargedTable();

    /**
     * Reads all entries of Mdst_Gamma Panther table, creates a particle list 'gamma:mdst' and adds them to StoreArray<Particles>.
     */
    void convertMdstGammaTable();

    /**
     * Reads all entries of Mdst_Klong Panther table, creates a particle list 'K_L0:mdst' and adds them to StoreArray<Particles>.
     */
    void convertMdstKLongTable();

    /**
    * Reads all entries of Mdst_Pi0 Panther table, creates a particle list 'pi0:mdst' and adds them to StoreArray<Particles>.
    */
    void convertMdstPi0Table();

    /**
     * Reads and converts all entries of Mdst_vee2 Panther table to V0 dataobjects and adds them to StoreArray<V0>. V0 daughters are converted from Mdst_Vee_Daughters to
     * TrackFitResults, which are appended to the TrackFitResult StoreArray.
     */
    void convertMdstVee2Table();

    /** Stores beam parameters (energy, angles) in BeamParameters (currently in the DataStore). */
    void convertBeamEnergy();

    /** Stores the IPProfiles in BeamParameters (currently in DataStore) */
    void convertIPProfile(bool beginRun = false);

    //-----------------------------------------------------------------------------
    // CONVERT OBJECTS
    //-----------------------------------------------------------------------------

    /**
     * Converts Gen_hepevt record to MCParticleGraph object.
     */
    void convertGenHepevtObject(const Belle::Gen_hepevt& genHepevt, MCParticleGraph::GraphParticle* mcParticle);

    /**
     * Converts Mdst_ecl(_aux) record to ECLCluster object.
     * If running on MC, the ECLCluster -> MCParticle relation is set as well.
     */
    void convertMdstECLObject(const Belle::Mdst_ecl& ecl, const Belle::Mdst_ecl_aux& eclAux, ECLCluster* eclCluster);

    /**
    * Converts Mdst_klm_cluster record to KLMCluster object.
    * No MCRelation is set, since there was no matching in Belle.
    */
    void convertMdstKLMObject(const Belle::Mdst_klm_cluster& klm, KLMCluster* klmCluster);

    /**
     * Converts Mdst_charged (Mdst_trk(_fit)) record to Track (TrackFitResult) object.
     * If running on MC, the Track -> MCParticle relation is set as well.
     */
    void convertMdstChargedObject(const Belle::Mdst_charged& belleTrack, Track* track);

    /**
     * Creates TrackFitResult and fills it.
     */
    TrackFitResult createTrackFitResult(const HepLorentzVector& momentum,
                                        const HepPoint3D&              position,
                                        const HepSymMatrix&     error,
                                        const short int charge,
                                        const Const::ParticleType& pType,
                                        const float pValue,
                                        const uint64_t hitPatternCDCInitializer,
                                        const uint32_t hitPatternVXDInitializer);

    /**
     * Fills Helix parameters (converted to Belle II version), 5x5 error matrix, 4-momentum, position and 7x7 error matrix from Belle Helix stored in Mdst_trk_fit.
     */
    int getHelixParameters(const Belle::Mdst_trk_fit& trk_fit,
                           const double mass,
                           const HepPoint3D& newPivot,
                           std::vector<float>& helixParams,
                           HepSymMatrix& error5x5,
                           HepLorentzVector& momentum,
                           HepPoint3D& position,
                           HepSymMatrix& error7x7,
                           const double dPhi = 0.0);

    /**
     * Converts Belle's Helix parameters and it's covariance matrix to Belle II's version.
     */
    void convertHelix(Belle::Helix& helix, std::vector<float>& helixParams, HepSymMatrix& error5x5);

    /**
     * Converts Belle's Helix parameters and it's covariance matrix to Belle II's version.
     */
    void convertHelix(const Belle::Mdst_trk_fit& trk_fit, const HepPoint3D& newPivot, std::vector<float>& helixParams,
                      std::vector<float>& helixError);

    /**
     * Fills 4-momentum, position and 7x7 error matrix from Belle Helix stored in Mdst_trk_fit.
     */
    int belleHelixToCartesian(const Belle::Mdst_trk_fit& trk_fit, const double mass, const HepPoint3D& newPivot,
                              HepLorentzVector& momentum, HepPoint3D& position, HepSymMatrix& error, double dPhi = 0.0);

    /**
     * obtains the helix parameters from trk_fit and moves the pivot to 0,0,0
     */
    void belleHelixToHelix(const Belle::Mdst_trk_fit& trk_fit,
                           std::vector<float>& helixParam, std::vector<float>& helixError);

    /**
     * Fills 4-momentum, position and 7x7 error matrix from Belle Vee daughter.
     */
    void belleVeeDaughterToCartesian(const Belle::Mdst_vee2& vee, const int charge, const Const::ParticleType& pType,
                                     HepLorentzVector& momentum, HepPoint3D& position, HepSymMatrix& error);

    /**
     * obtains the helix parameters of the vee daughters
     */
    void belleVeeDaughterHelix(const Belle::Mdst_vee2& vee, const int charge, std::vector<float>& helixParam,
                               std::vector<float>& helixError);

    /** Get PID information for belleTrack and add it to PIDLikelihood (with relation from track). */
    void convertPIDData(const Belle::Mdst_charged& belleTrack, const Track* track);

    /** Number of Belle track hypotheses (see c_belleHyp_to_chargedStable). */
    const static int c_nHyp = 5;
    /** maps Belle hypotheses to Const::ChargedStable (from http://belle.kek.jp/secured/wiki/doku.php?id=software:atc_pid). */
    const static Const::ChargedStable c_belleHyp_to_chargedStable[c_nHyp];

    /** Add given Belle likelihoods (not log-likelihoods, in Belle hypothethis order) for given detector to pid. */
    void setLikelihoods(PIDLikelihood* pid, Const::EDetector det, double likelihoods[c_nHyp], bool discard_allzero = false);

#ifdef HAVE_KID_ACC
    /** Returns ACC likelihood for given hypothesis idp. Copied from atc_pid::acc_pid(). */
    static double acc_pid(const Belle::Mdst_charged& chg, int idp);
    /** Returns CDC likelihood for given hypothesis idp. Copied from atc_pid::cdd_pid(). */
    double cdc_pid(const Belle::Mdst_charged& chg, int idp);
#endif

    /* calcualtes atc_pid(3,1,5,sigHyp,bkgHyp).prob() from converted PIDLikelihood */
    double atcPID(const PIDLikelihood* pid, int sigHyp, int bkgHyp);

    //-----------------------------------------------------------------------------
    // RELATIONS
    //-----------------------------------------------------------------------------

    /**
     * Sets ECLCluster -> Track relations
     */
    void setECLClustersToTracksRelations();


    /**
     * Sets KLMCluster -> Track and ECLCluster relations
     */
    void setKLMClustersRelations();

    //-----------------------------------------------------------------------------
    // MISC
    //-----------------------------------------------------------------------------

    /**
     * Helper function to recover falsely set idhep info in GenHepEvt list
     */
    int recoverMoreThan24bitIDHEP(int id);

    /**
     * Checks if the reconstructed object (Track, ECLCluster, ...) was matched to the same MCParticle
     */
    void testMCRelation(const Belle::Gen_hepevt& belleMC, const MCParticle* mcP, std::string objectName);

    //! MCParticle Graph to build Belle2 MC Particles
    Belle2::MCParticleGraph m_particleGraph;

    //! map of Gen_hepevt Panther IDs and corresponding MCParticle StoreArray indices
    std::map<int, int> genHepevtToMCParticle;
    //! map of Mdst_ecl Panther IDs and corresponding ECLCluster StoreArray indices
    std::map<int, int> mdstEclToECLCluster;
    //! map of gamma Panther IDs and corresponding Particle StoreArray indices
    std::map<int, int> mdstGammaToParticle;
    //! map of Mdst_klm Panther IDs and corresponding KLMCluster StoreArray indices
    std::map<int, int> mdstKlmToKLMCluster;
    //! map of Klong Panther IDs and corresponding Particle StoreArray indices
    std::map<int, int> mdstKlongToParticle;

    /** output PIDLikelihood array. */
    StoreArray<PIDLikelihood> m_pidLikelihoods;

    /** BeamParameters */
    DBObjPtr<BeamParameters> m_beamParamsDB;
    BeamParameters m_beamParams;

    /** CONVERSION OF TRACK ERROR MATRIX ELEMENTS */
    /** Belle error matrix elements are in the following order
     *  px/py/pz/E/x/y/z or in indices are (0,1,2,3,4,5,6)
     *  Belle II error matrix elements are in the following order
     *  x/y/z/px/py/pz or in indices (0,1,2,3,4,5).
     * The conversion of indices is therefore:
     * 4 -> 0, 5 -> 1, 6 -> 2, 0 -> 3, 1 -> 4, 2 -> 5,
     * and 3 -> nothing.
     */
    const int ERRMCONV[7] = {3, 4, 5, -1, 0, 1, 2};

    /** B filed in TESLA */
    const double BFIELD = 1.5;

    /** Conversion factor for Kappa -> Omega helix parameters */
    const double KAPPA2OMEGA = 1.5 * TMath::C() * 1E-11;

  };

} // end namespace Belle2

#endif
