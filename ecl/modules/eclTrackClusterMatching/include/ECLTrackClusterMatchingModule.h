/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLTRACKCLUSTERMATCHING_H
#define ECLTRACKCLUSTERMATCHING_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

#include "TFile.h"
#include "TTree.h"

namespace Belle2 {

  /** The module creates and saves a Relation between Tracks and ECLCluster in
   *  the DataStore. It uses the existing Relation between Tracks and ExtHit as well
   *  as the Relation between ECLCluster and ExtHit.
   */
  class ECLTrackClusterMatchingModule : public Module {
  public:
    /** Constructor, for setting module description and parameters. */
    ECLTrackClusterMatchingModule();

    /** Use to clean up anything you created in the constructor. */
    virtual ~ECLTrackClusterMatchingModule();

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

  private:

    /** Check if extrapolated hit is inside ECL and matches one of the desired categories. */
    bool isECLHit(const ExtHit& extHit) const;

    double clusterQuality(const ExtHit& extHit, double deltaPhi, double deltaTheta) const;

    /** members of ECLReconstructor Module */

    TFile* m_rootFilePtr; /**< pointer at root file used for storing info */
    std::string m_rootFileName; /**< name of the root file */
    bool m_writeToRoot; /**< if true, a rootFile named by m_rootFileName will be filled with info */
    TTree* m_tree; /**< Root tree and file for saving the output */

    // variables
    int m_iExperiment; /**< Experiment number */
    int m_iRun; /**< Run number */
    int m_iEvent; /**< Event number */

    int m_trackNo;
    double m_trackMomentum;
    double m_deltaPhi;
    double m_phiCluster;
    double m_errorPhi_ECLNEAR;
    double m_errorPhi_ECLCROSS;
    double m_errorPhi_ECLDL;
    double m_deltaTheta;
    double m_thetaCluster;
    double m_errorTheta_ECLNEAR;
    double m_errorTheta_ECLCROSS;
    double m_errorTheta_ECLDL;
    double m_quality;
    double m_quality_best; /**< Best quality among all extrapolated hits of one track */
    int m_hitstatus_best;

  };

} //Belle2
#endif
