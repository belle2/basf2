//+
// File : PantherInputModule.h
// Description : A module to read Panther record in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//-

#ifndef PANTHER_INPUT_H
#define PANTHER_INPUT_H

#include <framework/core/Module.h>

#define BELLE_TARGET_H "belle-x86_64-unknown-linux-gnu-g++.h"
#include "panther/panther.h"
#include "panther/panther_group.h"

// Belle objects (Panther tables)
#include "panther/belletdf.h"
#include "panther/mdst.h"

// Belle2 objects
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/MCParticle.h>
#include "mdst/dataobjects/MCParticleGraph.h"
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <string>

#include <sys/time.h>
#include <map>

namespace Belle2 {

  /**
   * Module reads Belle MDST files and converts Belle MDST objects (Panther records)
   * to Belle II MDST objects.
   *
   * The module performs conversion of the following Belle Panther records:
   *
   * o) Gen_hepevt -> MCParticle
   *
   */

  class PantherInputModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    PantherInputModule();
    virtual ~PantherInputModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    //! Panther -> basf2 conversion function
    void Convert();


    // Data members
  private:

    /**
     * Test panther database connection
     * export BELLE_POSTGRES_SERVER=${BELLE_POSTGRES_SERVER=ekpbelle}
     * export BELLE_INDEX_POSTGRES_SERVER=${BELLE_INDEX_POSTGRES_SERVER=al19}
     */
    void test_pntdb();

    /**
     * Initializes Belle II DataStore.
     */
    void initializeDataStore();

    /**
     * Converts Belle_event Panther record to Belle II EventMetaData StoreObjectPointer.
     */
    bool convertBelleEventObject();

    /**
     * Reads and converts all entries of Gen_hepevt Panther table to MCParticle dataobjects and adds them to StoreArray<MCParticle>.
     */
    void convertGenHepEvtTable();

    /**
     * Converts gen_hepevt record to MCParticleGraph object.
     */
    void convertGenHepevtObject(const Belle::Gen_hepevt& genHepevt, MCParticleGraph::GraphParticle* mcParticle);

    /**
     * Helper function to recover falsely set idhep info in GenHepEvt list
     */
    int recoverMoreThan24bitIDHEP(int id);

    //! Input MDST file name
    std::string m_inputFileName;

    //! PantherFile
    Belle::Panther_FileIO* m_fd;

    //! Event counter
    int m_nevt;

    // MCParticle Graph to build Belle2 MC Particles
    Belle2::MCParticleGraph m_particleGraph;

    // map of Gen_hepevt Panther IDs and corresponing MCParticle StoreArray indices
    std::map<int, int> genHepevtToMCParticle;

  };

} // end namespace Belle2

#endif
