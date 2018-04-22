/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <TFile.h>
#include <vxd/dataobjects/VxdID.h>
#include <tracking/dataobjects/MCParticleInfo.h>
#include <tracking/dataobjects/hitXP.h>



namespace Belle2 {

  /**  This class is the derivate of HitXP, and complete it with a constructor that use
  * all other complex types (classes) of basf2. It is necessary to buld a hitXP object.
  */

  class hitXPDerivate: public hitXP {

  public:

    /** empy constructor */
    hitXPDerivate() {}

    /** constructor for SVD hit
    * with arguments:  hit, cluster, particle, sensor info)
    */
    hitXPDerivate(const SVDTrueHit& hit, const SVDCluster cluster, const MCParticle& particle, const VXD::SensorInfoBase& sensor);

    /** constructor for PXD hit
    * with arguments:  hit, particle, sensor info)
    */
    hitXPDerivate(const PXDTrueHit& hit, const MCParticle& particle, const VXD::SensorInfoBase& sensor);

    //! needed by root
    ClassDef(hitXPDerivate, 1);
  };
} /** end namespace Belle2 */
