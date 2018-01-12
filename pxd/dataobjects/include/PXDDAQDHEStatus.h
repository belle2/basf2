/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 * Created: Dez 2017                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDErrorFlags.h>
/// TObject is not used in this class, but the include is needed for the ClassDef
#include <TObject.h>

using namespace Belle2::PXD::PXDError;

namespace Belle2 {

  /**
   * The PXD DAQ DHE Status class
   *
   * This is a small class that records information about PXD DAQ DHE status
   * It will record if the data of this sensor (readout by this DHE) is useable.
   *
   */
  class PXDDAQDHEStatus {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHEStatus() : m_usable(true), m_errorMask(0), m_critErrorMask(0), m_startRow(0), m_frameNr(0), m_rawCount(0), m_redCount(0) {}

    /** constructor setting the error mask, dhcid, raw and reduced data counters, ...
     * @param id VxdID of sensor
     * @param dheid DHEID of sensor
     * @param mask Error mask
     * @param raw raw data counter
     * @param red reduced data counter
     * @param sr Trigger Start Row (Trigger Offset)
     * @param fn (absolute) Readout Frame Number, lower bits only
     */
    PXDDAQDHEStatus(VxdID id, int dheid, PXDErrorFlags mask, uint32_t raw, uint32_t red, unsigned short sr,
                    unsigned short fn) : m_sensorID(id), m_dheID(dheid),
      m_usable(true), m_errorMask(mask), m_critErrorMask(0), m_startRow(sr), m_frameNr(fn), m_rawCount(raw), m_redCount(red) {}

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

    /** Set Error bit mask
     * @param m Bit Mask to set
     */
    void setErrorMask(PXDErrorFlags m) { m_errorMask = m; }

    /** Return Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getErrorMask(void) const { return m_errorMask; }

    /** Set Critical Error bit mask
     * @param m Bit Mask to set
     */
    void setCritErrorMask(PXDErrorFlags m) { m_critErrorMask = m; }

    /** Return Critical Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getCritErrorMask(void) const { return m_critErrorMask; }

    /** Calculate the usability decision
     * If any of the critical bits is set in the error mask
     * the PXD data from this DHE is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == 0;}

    /** Set VxdID and DHC ID of sensor */
    void setDHEID(VxdID id, int dheid) { m_sensorID = id; m_dheID = dheid;};
    /** Get DHE ID of sensor*/
    unsigned short getDHEID(void) const { return  m_dheID;};
    /** Get VxdID of sensor */
    VxdID getSensorID(void) const { return m_sensorID;};

    /** Set Data counters for reduction calculation */
    void setCounters(uint32_t raw, uint32_t red) {m_rawCount = raw; m_redCount = red;};
    /** Get Data counters for reduction calculation */
    void getCounters(uint32_t& raw, uint32_t& red) const { raw = m_rawCount; red = m_redCount;};
    /** Set Raw Data counter for reduction calculation */
    uint32_t getRawCnt(void) const { return m_rawCount;};
    /** Set Reduced Data counter for reduction calculation */
    uint32_t getRedCnt(void) const { return m_redCount;};
    /** set Trigger Start Row */
    void setStartRow(unsigned int sr) { m_startRow = sr;};
    /** get Trigger Start Row */
    unsigned short getStartRow(void) const { return  m_startRow;};
    /** set Readout Frame number */
    void setFrameNr(unsigned int fn) { m_frameNr = fn;};
    /** get Readout Frame number */
    unsigned short getFrameNr(void) const { return  m_frameNr;};

  private:

    VxdID m_sensorID;/**< Sensor ID.*/
    unsigned short m_dheID;/**< DHE ID as delivered by DAQ.*/
    bool m_usable; /**< data is useable.*/
    PXDErrorFlags m_errorMask; /**< errors found in this DHE/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */

    unsigned short m_startRow; /**< Startrow from DHE header */
    unsigned short m_frameNr; /**< Frame number (low bits) from DHE header */
    uint32_t m_rawCount; /**< raw byte count for monitoring */
    uint32_t m_redCount; /**< reduced byte count for monitoring */

    ClassDef(PXDDAQDHEStatus, 1);

  }; // class PXDDAQDHEStatus

} // end namespace Belle2
