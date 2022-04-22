/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventT0.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <string>

namespace Belle2 {

  /** This module estimates the EventT0 as the average of cluster time of SVD clusters associated to tracks.
   *  The EventT0 is set to NaN if there are not RecoTracks or there are not SVD clusters associated to tracks or
   *  RecoTrack pt < m_pt OR RecoTrack pz < m_pz. The EventT0 estimated is added to the temporaryEventT0s to
   *  the StoreObjPtr as EventT0Component that cointains: eventT0, eventT0_error, detector=SVD, algorithm, quality.
   */

  class SVDEventT0EstimatorModule : public Module {
  public:

    /** Constructor defining the parameters */
    SVDEventT0EstimatorModule();

    /** default destructor*/
    virtual ~SVDEventT0EstimatorModule();

    /** Initialize the SVDEventT0Estimator.*/
    virtual void initialize() override;

    /** This method is the core of the SVDEventT0Estimator. */
    virtual void event() override;

  private:

    std::string m_eventT0Name = "EventT0"; /**< name of StoreObj EventT0  */
    std::string m_recoTracksName = "RecoTracks"; /**< name of RecoTracks StoreArray */
    StoreObjPtr<EventT0> m_eventT0; /**< EventT0 StoreObjPtr */
    StoreArray<RecoTrack> m_recoTracks; /**< RecoTracks StoreArray */
    double m_ptSelection = 0.25; /**< Cut on pt for RecoTrack selection */
    double m_absPzSelection = 0.; /**< Cut on abs(pz) for RecoTrack selection */
    std::string m_algorithm = "clsOnTrack_time_average"; /**< name of the algorithm used to evaluate SVD-eventT0 */

  };
}
