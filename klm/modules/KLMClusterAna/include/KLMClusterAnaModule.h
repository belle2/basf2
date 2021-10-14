//#pragma once
//useful documentation for vector--> TVector https://root-forum.cern.ch/t/vector-into-tvector/7775


/* Belle 2 headers. */
#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>

/* KLM headers. */
#include <mdst/dataobjects/KLMCluster.h>
#include <klm/dataobjects/bklm/BKLMHit2d.h>
#include <klm/dataobjects/eklm/EKLMHit2d.h>


#include <klm/dataobjects/KLMClusterShape.h>

namespace Belle2 {

  /**
   * Module for KLM cluster reconstruction efficiency studies.
   */
  class KLMClusterAnaModule : public Module {

  public:

    /**
     * Constructor.
     */
    KLMClusterAnaModule();

    /**
     * Destructor.
     */
    ~KLMClusterAnaModule();

    /**
     * Initializer.
     */
    void initialize() override;

    /**
     * Called when entering a new run.
     */
    void beginRun() override;

    /**
     * This method is called for each event.
     */
    void event() override;




  private:


    /** KLM clusters. */
    StoreArray<KLMCluster> m_KLMClusters;

    /** Output per cluster. */
    StoreArray<KLMClusterShape> m_KLMClusterShape; /**< array of output information */

    /** BKLMhits. */
    StoreArray<BKLMHit2d> bklmHit2ds;

    /** EKLMhits. */
    StoreArray<EKLMHit2d> eklmHit2ds;

    /** Running main calculations on KLMCluster  */
    void runClusterAna(KLMCluster& klmcluster);


  };

}

