/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: 11.04.2018                                                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <vxd/dataobjects/VxdID.h>
#include <framework/logging/Logger.h>
#include <memory>


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

    bool isGateOK(VxdID id, int gate) const
    {
      if (m_fullGated) {
        return false;
      } else {
        if (m_readoutGated) {
          if (id.getLayerNumber() == 2) {
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

    bool isVCellOK(VxdID id, int vCellId) const { return isGateOK(id, vCellId / 4); }

    void setGateGatedL1(int gate, bool gated)
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
    m_gateStatus; /**< Status of Gates (true=gated), the gates are numbered as (VCellID/4) for Layer 1, and invers for Layer2 */

    ClassDef(PXDGatedModeInfo, 1)

  }; // class PXDGatedModeInfo

} // end namespace Belle2
