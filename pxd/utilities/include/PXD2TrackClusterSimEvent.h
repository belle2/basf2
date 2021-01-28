/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Qingyuan Liu                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <pxd/utilities/PXDTuple.h>
#include <pxd/utilities/PXD2TrackBase.h>


namespace Belle2 {
  namespace PXD {
    namespace Tuple {
      struct TrackClusterSim_t : TrackCluster_t {
        /** Default Constructor */
        TrackClusterSim_t(): TrackCluster_t(), simTol(0), simCluster() {}

        float simTol; /**< Thichness over length in cluster simulation. */
        Cluster_t simCluster; /**< Simulated cluster. */
      };

      typedef TrackBase_t<TrackClusterSim_t> TrackWithSim_t;

    } // end namespace Tuple
  } // end namespace PXD

  using namespace Belle2::PXD::Tuple;


  /** Class PXD2TrackClusterSimEvent: Event data container for performance and calibration studies.
   * This class holds all required variables for TTree creation and
   * the analysis, such that no relation lookup is needed.
   *
   * Caveat: this class is not placed in the dataobjects folder
   * to avoid dependency cycles.
   */
  class PXD2TrackClusterSimEvent : public PXD2TrackBase<TrackWithSim_t >  {

    ClassDef(PXD2TrackClusterSimEvent, 1);
  };
} // end namespace Belle2
