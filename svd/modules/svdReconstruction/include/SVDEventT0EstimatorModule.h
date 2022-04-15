/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/core/Environment.h>

#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>

#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <genfit/TrackPoint.h>
#include <TVector3.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <string>

namespace Belle2 {

  /**
   *
   */

  class SVDEventT0EstimatorModule : public Module {
  public:

    /** Constructor defining the parameters */
    SVDEventT0EstimatorModule();

    /** default destructor*/
    virtual ~SVDEventT0EstimatorModule();

    /** Initialize the SVDEventT0Estimator.*/
    virtual void initialize() override;

    /** Called when entering a new run.     */
    virtual void beginRun() override;

    /** This method is the core of the SVDEventT0Estimator. */
    virtual void event() override;

    /** This method is called if the current run ends. */
    virtual void endRun() override;

    /** This method is called at the end of the event processing.   */
    virtual void terminate() override;


  private:

    std::string m_eventT0; /**< name of StoreObj EventT0  */
    std::string m_recoTracks; /**< name of RecoTracks StoreArray */

  protected:

    /** Estimate the SVD eventT0 as the average of time of clusters
     *  clusters associated to tracks
     */
    double eventT0Estimator(const std::vector<SVDCluster* > svdClusters);


  };
}



























