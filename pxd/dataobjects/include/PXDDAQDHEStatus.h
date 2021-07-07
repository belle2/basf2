/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vxd/dataobjects/VxdID.h>
#include <pxd/dataobjects/PXDDAQDHPStatus.h>
#include <pxd/dataobjects/PXDErrorFlags.h>


namespace Belle2 {

  /** tuple of Chip ID (2 bit), Row (10 bit), Common Mode (6 bit) */
  typedef std::tuple<uint8_t, uint16_t, uint8_t> PXDDAQDHPComMode;
  using Belle2::PXD::PXDError::PXDErrorFlags;
  using Belle2::PXD::PXDError::EPXDErrNr;

  /**
   * The PXD DAQ DHE Status class
   *
   * This is a small class that records information about PXD DAQ DHE status
   * It will record if the data of this sensor (readout by this DHE) is useable.
   *
   */
  class PXDDAQDHEStatus final {
  public:

    /** Default constructor for the ROOT IO. */
    PXDDAQDHEStatus() : m_errorMask(0), m_critErrorMask(0), m_usable(true), m_sensorID(0), m_dheID(0), m_triggerGate(0), m_frameNr(0),
      m_dhp_found_mask(0),
      m_rawCount(0), m_redCount(0), m_errorinfo(0) {}

    /** constructor setting the error mask, dheid, raw and reduced data counters, ...
     * @param id VxdID of sensor
     * @param dheid DHEID of sensor
     * @param mask Error mask
     * @param tg Trigger Gate (Start Row, Trigger Offset)
     * @param fn (absolute) Readout Frame Number, lower bits only
     */
    PXDDAQDHEStatus(VxdID id, int dheid, const PXDErrorFlags& mask, unsigned short tg,
                    unsigned short fn) : m_errorMask(mask), m_critErrorMask(0), m_usable(true), m_sensorID(id), m_dheID(dheid),
      m_triggerGate(tg), m_frameNr(fn), m_dhp_found_mask(0), m_rawCount(0), m_redCount(0), m_errorinfo(0)
    {}

    /** destructor */
    virtual ~PXDDAQDHEStatus() {};

    /** Return Usability of data
     * @return conclusion if data is useable
     */
    bool isUsable() const { return m_usable; }

    /** Mark Data in DHE as Unusable
     */
    void markUnusable() { m_usable = false; m_errorMask[EPXDErrNr::c_nrUNUSABLE_DATA] = true;}

    /** Set Error bit mask
     * @param mask Bit Mask to set
     */
    void setErrorMask(const PXDErrorFlags& mask) { m_errorMask = mask; }
    /** Return Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getErrorMask(void) const { return m_errorMask; }

    /** Set Critical Error bit mask
     * @param mask Bit Mask to set
     */
    void setCritErrorMask(const PXDErrorFlags& mask) { m_critErrorMask = mask; }

    /** Return Critical Error bit mask
     * @return bit mask
     */
    PXDErrorFlags getCritErrorMask(void) const { return m_critErrorMask; }

    /** Calculate the usability decision
     * If any of the critical bits is set in the error mask
     * the PXD data from this DHE is not usable for analysis
     * TODO Maybe this decision needs improvement.
     */
    void Decide(void) {m_usable = (m_errorMask & m_critErrorMask) == PXDErrorFlags(0) && !m_errorMask[EPXDErrNr::c_nrUNUSABLE_DATA];}

    /** Set VxdID and DHE ID of sensor */
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
    /** set Trigger Gate */
    void setTriggerGate(unsigned int tg) { m_triggerGate = tg;};
    /** get Trigger Gate */
    unsigned short getTriggerGate(void) const { return  m_triggerGate;};
    /** get Trigger Start Row */
    unsigned short getStartRow(void) const { return  m_triggerGate * 4;};
    /** set Readout Frame number */
    void setFrameNr(unsigned int fn) { m_frameNr = fn;};
    /** get Readout Frame number */
    unsigned short getFrameNr(void) const { return  m_frameNr;};


    /** get Mask for found DHPs with valid data */
    unsigned short getDHPFoundMask(void) { return m_dhp_found_mask;};
    /** set Mask for found DHPs with valid data */
    void setDHPFoundMask(unsigned short dhpmask) { m_dhp_found_mask = dhpmask;};

    /** set erroinfo from the DHE END **/
    void setEndErrorInfo(uint32_t e) { m_errorinfo = e;};
    /** get erroinfo from the DHE END **/
    uint32_t getEndErrorInfo(void) const { return m_errorinfo;};

    /** Add DHP information
     * @param daqdhp DHP Status Object
     */
    void addDHP(PXDDAQDHPStatus& daqdhp) { m_pxdDHP.push_back(daqdhp);};

    /** New DHP information
     * @param params constructor parameter
     * @return new DHP Status Object
     */
    template<class ...Args> PXDDAQDHPStatus&  newDHP(Args&& ... params)
    {
      /*return*/ m_pxdDHP.emplace_back(std::forward<Args>(params)...);
      return m_pxdDHP.back();
    }

    /** iterator-based iteration for DHPs */
    std::vector<PXDDAQDHPStatus>::iterator begin()  { return m_pxdDHP.begin(); };
    /** iterator-based iteration for DHPs */
    std::vector<PXDDAQDHPStatus>::iterator end()  { return m_pxdDHP.end(); };

    /** const iterator-based iteration for DHPs */
    std::vector<PXDDAQDHPStatus>::const_iterator cbegin() const { return m_pxdDHP.cbegin(); };
    /** const iterator-based iteration for DHPs */
    std::vector<PXDDAQDHPStatus>::const_iterator cend() const { return m_pxdDHP.cend(); };
    /** Returns PXDDAQDHPStatus for the last DHP */
    PXDDAQDHPStatus& dhp_back()  { return m_pxdDHP.back(); };
    /** Returns number of DHPs */
    size_t dhp_size() const { return m_pxdDHP.size(); };

    /** Add Common Mode information
     * @param daqcm DHP Common Mode object
     */
    auto addCM(PXDDAQDHPComMode& daqcm) { return m_commode.push_back(daqcm);};

    /** iterator-based iteration for Common Mode */
    std::vector<PXDDAQDHPComMode>::iterator cm_begin()  { return m_commode.begin(); };
    /** iterator-based iteration for Common Mode */
    std::vector<PXDDAQDHPComMode>::iterator cm_end()  { return m_commode.end(); };
    /** Returns last Common Mode for this event */
    PXDDAQDHPComMode& cm_back()  { return m_commode.back(); };
    /** Returns number of Common Mode blocks in this event */
    size_t cm_size() const { return m_commode.size(); };
    /** Returns CM DHP ID at position i */
    int get_cm_dhp(int i) const { return std::get<0>(m_commode[i]); };
    /** Returns CM row at position i */
    int get_cm_row(int i) const { return std::get<1>(m_commode[i]); };
    /** Returns CM value at position i */
    int get_cm_value(int i) const { return std::get<2>(m_commode[i]); };

  private:
    PXDErrorFlags m_errorMask; /**< errors found in this DHE/sensor */
    PXDErrorFlags m_critErrorMask; /**< critical error mask */
    bool m_usable; /**< data is useable.*/

    unsigned short m_sensorID;/**< Sensor ID.*/
    unsigned short m_dheID;/**< DHE ID as delivered by DAQ.*/
    unsigned short m_triggerGate; /**< Trigger Gate ("Startrow") from DHE header */
    unsigned short m_frameNr; /**< Frame number (low bits) from DHE header */
    unsigned short m_dhp_found_mask; /**< Mask for DHP with valid data */
    uint32_t m_rawCount; /**< raw byte count for monitoring */
    uint32_t m_redCount; /**< reduced byte count for monitoring */
    uint32_t m_errorinfo; /**< erroinfo from the DHE END **/

    /** Vector of DHP informations belonging to this event */
    std::vector< PXDDAQDHPStatus> m_pxdDHP;

    /** Vector of Common Mode informations belonging to this event */
    std::vector < PXDDAQDHPComMode> m_commode;

    /** necessary for ROOT */
    ClassDef(PXDDAQDHEStatus, 8);

  }; // class PXDDAQDHEStatus

} // end namespace Belle2
