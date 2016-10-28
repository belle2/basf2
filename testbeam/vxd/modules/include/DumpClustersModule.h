/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DUMPCLUSTERSMODULE_H
#define DUMPCLUSTERSMODULE_H

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/spacePointCreation/SpacePoint.h>

// load some parts of the PXD
#include <pxd/dataobjects/PXDDigit.h>
#include <svd/dataobjects/SVDDigit.h>
#include <testbeam/vxd/dataobjects/TelDigit.h>

#include <TTree.h>

namespace Belle2 {
  /**
   * Dumps information on SVD, PXD and Tel spacepoints into a root file which can be used for analysis outside of basf2
   *
   *    *
   */
  class DumpClustersModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    DumpClustersModule();

    /** Destructor  */
    virtual ~DumpClustersModule();

    /** initialize  */
    virtual void initialize();

    /** begin run: currently empty  */
    virtual void beginRun();

    /** main function  */
    virtual void event();

    /** end run  */
    virtual void endRun();

    /** currently empty */
    virtual void terminate();

    /** defines the tree used for output */
    void defineHisto();

  private:
    //pointer to the tree
    TTree* m_tree;
    //the Cluster variables:
    std::vector<double> m_posu, m_posv; //position local u and v
    std::vector<double> m_erru, m_errv; //error on u and v position
    std::vector<double> m_rho; //correlation between u and v error
    std::vector<int> m_id; //sensor ids
    std::vector<int> m_isU; // -1 for pxd cluster; 1 if svd U cluster; 0 if svd V cluster
    int m_tlunumber; // event number from the TLU
    Long64_t m_time; // store the event time
    int m_runnumber, m_eventnumber, m_subrunnumber, m_experimentnumber, m_productionnumber; // further event metadata
    // PXD/Tel Digit variables
    std::vector<int> m_col, m_row; //local column and row values
    std::vector<double> m_value; //local column and row values
    std::vector<int> m_id2; //pxd ids

    // SVD Digit variables
    std::vector<bool> m_IsUStrip; //Strip ID determines whether this is a u strip (true) or a v strip(false)
    std::vector<int> m_cell_ID_SVD; //Local cell ID, either rows or columns, denpending on isUStrip
    std::vector<int> m_charge_SVD; // Charge of the SVD digit
    std::vector<int> m_id3; //Sensor ID of the SVD

    std::string m_svdclusters; /* name of the storearray with svd space points */
    std::string m_pxdclusters; /* name of the storearray with pxd space points */
    std::string m_telclusters; /* name of the storearray with pxd space points */

    std::string m_pxddigits; /* name of the storearray with pxd space points */
    std::string m_svddigits; /* name of the storearray with svd space points */
    std::string m_teldigits; /* name of the storearray with svd space points */

  };
}

#endif /* DumpClustersMODULE_H */
