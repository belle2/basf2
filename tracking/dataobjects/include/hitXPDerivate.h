/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <vxd/geometry/SensorInfoBase.h>
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
    hitXPDerivate(const SVDTrueHit& hit, const SVDCluster& cluster, const MCParticle& particle, const VXD::SensorInfoBase& sensor);

    /** constructor for PXD hit
    * with arguments:  hit, particle, sensor info)
    */
    hitXPDerivate(const PXDTrueHit& hit, const MCParticle& particle, const VXD::SensorInfoBase& sensor);

    //! needed by root
    ClassDef(hitXPDerivate, 1);
  };
} /** end namespace Belle2 */
