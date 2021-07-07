/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/geometry/GeometryPar.h>
#include <klm/eklm/geometry/GeometryData.h>

namespace Belle2::KLM {

  /**
   * Wrapper class around bklm::GeometryPar and EKLM::GeometryData.
   * It only returns an instance to the barrel or endcap geometry.
   *
   * This class has a ROOT dictionary, therefore it can be used in python modules:
   *
   * @code{.py}
   * import basf2
   * import ROOT
   *
   * class KLMGeometryPrinter(basf2.Module):
   *   def beginRun(self):
   *     bklm = ROOT.Belle2.KLMGeometryPar.BarrelInstance()
   *     if bklm:
   *       print(bklm.getLayerInnerRadius(5))
   *     eklm = ROOT.Belle2.KLMGeometryPar.EndcapInstance()
   *     if eklm:
   *       print(eklm.eklm.getStripLength(21))
   * @endcode
   *
   * @note In order to return something meaningful, the python module
   *       has to be added to a path after the Gearbox and the Geometry modules.
   */
  class KLMGeometryPar {

  public:

    /**
     * Return a pointer to the bklm::GeometryPar instance.
     */
    static const bklm::GeometryPar* BarrelInstance()
    {
      return bklm::GeometryPar::instance();
    }

    /**
     * Return a pointer to the EKLM::GeometryData instance.
     */
    static const EKLM::GeometryData* EndcapInstance()
    {
      return &(EKLM::GeometryData::Instance());
    }

  private:

    /** Private constructor. */
    KLMGeometryPar();

    /** Private destructor. */
    ~KLMGeometryPar();

  };

}
