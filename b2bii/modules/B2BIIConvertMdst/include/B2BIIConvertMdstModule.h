//+
// File : PantherInputModule.h
// Description : A module to read Panther record in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contirbutors: Anze Zupanc, Matic Lubej
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

// Belle2 objects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>

#include <string>
#include <map>

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
    virtual ~B2BIIConvertMdstModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

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
     * Reads and converts all entries of Mdst_charged (Mdst_trk and Mdst_trk_fit) Panther table to Track (TrackFitResult) dataobjects and adds them to StoreArray<Track> (StoreArray<TrackFitResult>).
     */
    void convertMdstChargedTable();

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
     * Converts Mdst_charged (Mdst_trk(_fit)) record to Track (TrackFitResult) object.
     * If running on MC, the Track -> MCParticle relation is set as well.
     */
    void convertMdstChargedObject(const Belle::Mdst_charged& belleTrack, Track* track);

    /** Get PID information for belleTrack and add it to PIDLikelihood (with relation from track). */
    void convertPIDData(const Belle::Mdst_charged& belleTrack, const Track* track);

    //
    // RELATIONS
    //
    /**
     * Sets ECLCluster -> Track relations
     */
    void setECLClustersToTracksRelations();

    //-----------------------------------------------------------------------------
    // MISC
    //-----------------------------------------------------------------------------

    /**
     * Helper function to recover falsely set idhep info in GenHepEvt list
     */
    int recoverMoreThan24bitIDHEP(int id);

    /**
     * Compares the track momentum and position values for pion hypothesis.
     */
    void testTrackConversion(const Belle::Mdst_charged& belleTrack, const TrackFitResult* tfr);

    /**
     * Checks if the reconstructed object (Track, ECLCluster, ...) was matched to the same MCParticle
     */
    void testMCRelation(const Belle::Gen_hepevt& belleMC, const MCParticle* mcP, std::string objectName);

    /** Number of track hypotheses (see c_belleHyp_to_chargedStable). */
    const static int c_nHyp = 5;
    /** maps Belle hypotheses to Const::ChargedStable (from http://belle.kek.jp/secured/wiki/doku.php?id=software:atc_pid). */
    const static Const::ChargedStable c_belleHyp_to_chargedStable[c_nHyp];

    //! MCParticle Graph to build Belle2 MC Particles
    Belle2::MCParticleGraph m_particleGraph;

    //! map of Gen_hepevt Panther IDs and corresponing MCParticle StoreArray indices
    std::map<int, int> genHepevtToMCParticle;

    /** output PIDLikelihood array. */
    StoreArray<PIDLikelihood> m_pidLikelihoods;

  };

} // end namespace Belle2

#endif
