/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <string>
#include <TGraph.h>
#include <arich/dbobjects/ARICHHapdChipInfo.h>

namespace Belle2 {
  /**
  *   Contains manufacturer data of the photo sensor - HAPD
  */
  class ARICHHapdInfo: public TObject {
  public:

    /**
     * Various constants
     */
    enum {c_NumberOfChips = 4, /**< number of HAPD Chips on the sensor */
         };

    /**
     * Default constructor
     */
    ARICHHapdInfo(): m_serial(""), m_qe400(0.0), m_highVoltage(0.0), m_guardBias(0), m_current(0.0), m_quantumEfficiency(NULL),
      m_pulseHeight(NULL)
    {
      for (unsigned i = 0; i < 4; i++) m_HAPDChipInfo[i] = NULL;
    }


    /**
     * Constructor
     */
    ARICHHapdInfo(const std::string& serial, float qe400, float hv, int gb, float I, ARICHHapdChipInfo** HAPDChipInfo, TGraph* qe,
                  TGraph* pulseHeight): m_serial(serial), m_qe400(qe400), m_highVoltage(hv), m_guardBias(gb), m_current(I), m_quantumEfficiency(qe),
      m_pulseHeight(pulseHeight)
    {
      for (unsigned i = 0; i < 4; i++) m_HAPDChipInfo[i] = HAPDChipInfo[i];
    }

    /**
     * Destructor
     */
    ~ARICHHapdInfo() {};

    /**
     * Return HAPD Serial Number
     * @return serial number
     */
    std::string getSerialNumber() const {return m_serial;}

    /**
     * Set HAPD Serial Number
     * @param serial serial number
     */
    void setSerialNumber(const std::string& serial) {m_serial = serial; }

    /**
     * Return quantum efficiency at 400 nm
     * @return quantum efficiency
     */
    float getQuantumEfficiency400() const {return m_qe400;}

    /**
     * Set quantum efficiency at 400 nm
     * @param qe quantum efficiency
     */
    void setQuantumEfficiency400(float qe) {m_qe400 = qe;}

    /**
     * Return operational High Voltage
     * @return voltage
     */
    float getHighVoltage() const {return m_highVoltage;}

    /**
     * Set operational High Voltage
     * @param hv voltage
     */
    void setHighVoltage(float hv) {m_highVoltage = hv;}

    /**
     * Return operational Guard Bias
     * @return guards bias
     */
    int getGuardBias() const {return m_guardBias;}

    /**
     * Set operational Guard Bias
     * @param guard bias
     */
    void setGuardBias(int gb) {m_guardBias = gb;}

    /**
     * Return Operational current
     * @return current
     */
    float getCurrent() const {return m_current;}

    /**
     * Set Operational current
     * @param I current
     */
    void setCurrent(float I) {m_current = I;}

    /**
     * Return HapdChipInfo of the chip i
     * @param i index of the chip
     * @return HapdChipInfo
     */
    ARICHHapdChipInfo* getHapdChipInfo(unsigned int i) const;

    /**
     * Set HapdChipInfo of the chip i
     * @param i index of the chip
     * @param chipInfo ARICHHapdChipInfo
     */
    void setHapdChipInfo(unsigned int i, ARICHHapdChipInfo* chipInfo);

    /**
     * Return Quantum Efficiency as a function of wavelength
     * @return quantum efficiency
     */
    TGraph* getQuantumEfficiency() const {return m_quantumEfficiency;}

    /**
     * Set Quantum Efficiency as a function of wavelength
     * @param quantum efficiency
     */
    void setQuantumEfficiency(TGraph* qe) {m_quantumEfficiency = qe;}

    /**
     * Return pulse height distribution of one of the HAPD channels
     * @return pulse height distribution
     */
    TGraph* getPulseHeightDistribution() const {return m_pulseHeight;}

    /**
     * Set pulse height distribution of one of the HAPD channels
     * @param adc pulse height distribution
     */
    void setPulseHeightDistribution(TGraph* adc) {m_pulseHeight = adc;}


  private:
    std::string m_serial;                   /**< serial number of the sensor */
    float   m_qe400;                        /**< quantum efficiency at 400 nm */
    float   m_highVoltage;                  /**< Operational high voltage */
    int     m_guardBias;                    /**< guard HV (V) */
    float   m_current;                      /**< Current (A)  */
    ARICHHapdChipInfo* m_HAPDChipInfo[4];   /**< HapdChipInfo id of the i-th chip in the sensor */
    TGraph*  m_quantumEfficiency;           /**< Quantum Efficiency as a function of wavelength */
    TGraph*  m_pulseHeight;                 /**< Pulse height distribution */

    ClassDef(ARICHHapdInfo, 2); /**< ClassDef */
  };
} // end namespace Belle2

