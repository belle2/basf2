/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/logging/Logger.h>

namespace Belle2 {

  /**
   * The PXD Gated Info Class
   *
   * Here we store the information for all gates of all modules
   * if the read out value is useable for reco.
   * if the sensor was completely gated, no gate contains any
   * signal information
   * if the sensor was gates during readout, a few gates do not
   * contain useful info, as the gating process messes up the adc values
   *
   */

  class PXDGatedModeInfo : public TObject {
  public:

    /** Default constructor for the ROOT IO. */
    PXDGatedModeInfo() :  m_fullGated(false), m_readoutGated(false) {}


    /** Set flag that we triggered while detector was gated
     * @param b gated
     */
    void setFullGated(bool b) { m_fullGated = b; }

    /** Return if data was recorded while detector was gated
     * @return detector was gated
     */
    bool isFullGated() const { return m_fullGated; }

    /** Set flag that  we triggered while detector was gated
     * @param b gated
     */
    void setReadoutGated(bool b) { m_readoutGated = b; }

    /** Return if data was readout while detector was gated
     * @return detector was gated
     */
    bool isReadoutGated() const { return m_readoutGated; }

    /** Return if gate is ok
     * @return gate is ok
     */
    bool isGateOK(VxdID id, int gate) const
    {
      if (m_fullGated) {
        return false;
      } else {
        if (m_readoutGated) {
          if (id.getLayerNumber() == 1) {
            gate = 192 - 1 - gate;
          }
          if (gate < 0 || gate >= 192) {
            B2ERROR("Invalid Gate");
            return false;
          }
          return !m_gateStatus[gate];
        } else {
          return true;
        }
      }
    }

    /** is the VCELL valid*/
    bool isVCellOK(VxdID id, int vCellId) const { return isGateOK(id, vCellId / 4); }

    /** set gate status of specific gate */
    void setGateGatedL2(int gate, bool gated)
    {
      if (gate < 0 || gate >= 192) {
        B2ERROR("Invalid Gate");
      } else {
        m_gateStatus[gate] = gated;
      }
    }

  private:
    bool m_fullGated; /**< Sensor was in gated mode while triggered */
    bool m_readoutGated; /**< Sensor switched to gated mode during readout */

    std::vector <bool>
    m_gateStatus; /**< Status of Gates (true=gated), the gates are numbered as (VCellID/4) for Layer 2 (as rolling shutter runs in V direction), and invers for Layer 1 */

    /** necessary for ROOT */
    ClassDef(PXDGatedModeInfo, 1)

  }; // class PXDGatedModeInfo

} // end namespace Belle2
