/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Rok Pestotnik, Manca Mrvar                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <TH1F.h>
#include <string>
#include <TGraph.h>
#include <arich/database/ARICHHapdChipInfo.h>
//class ARICHHapdChipInfo;
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
    ARICHHapdInfo(): m_qe400(0), m_highVoltage(0), m_guardVoltage(0), m_current(0), m_quantumEfficiency(NULL), m_pulseHeight(NULL)
    {
      for (unsigned ii = 0; ii < 4; ii++) m_HAPDChipInfo[ii] = NULL;
    };

    /**
     * Constructor
     */
    ARICHHapdInfo(float qe400, float hv, float gv, float I, ARICHHapdChipInfo** HAPDChipInfo, TGraph* quantumEfficiency,
                  TH1F* pulseHeight)
    {
      m_qe400 = qe400;
      m_highVoltage = hv;
      m_guardVoltage = gv;
      m_current = I;
      for (unsigned ii = 0; ii < 4; ii++) m_HAPDChipInfo[ii] = HAPDChipInfo[ii];
      m_quantumEfficiency = quantumEfficiency;
      m_pulseHeight = pulseHeight;
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
     * Return operational Guard voltage
     * @return voltage
     */
    float getGuardVoltage() const {return m_guardVoltage;}

    /**
     * Set operational Guard voltage
     * @param hv voltage
     */
    void setGuardVoltage(float gv) {m_guardVoltage = gv;}


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
     * @return current
     */
    ARICHHapdChipInfo* getHapdChipInfo(unsigned int i) { if (i < c_NumberOfChips) return m_HAPDChipInfo[i]; else return NULL;  }

    /**
     * Set HapdChipInfo of the chip i
     * @param i index of the chip
     * @param chipInfo ARICHHapdChipInfo
     */
    void setHapdChipInfo(unsigned int i, ARICHHapdChipInfo* chipInfo) { if (i < c_NumberOfChips) m_HAPDChipInfo[i] = chipInfo;}

    /**
     * Return Quantum Efficiency as a function of wavelength
     * @return current
     */
    TGraph* getQuantumEfficiency() const {return m_quantumEfficiency;}

    /**
     * Set Quantum Efficiency as a function of wavelength
     * @param current
     */
    void setQuantumEfficiency(TGraph* qEfficiency) {m_quantumEfficiency = qEfficiency;}

    /**
     * Return pulse height distribution of one of the HAPD channels
     * @return pulse height distribution
     */
    TH1F* getPulseHeightDistribution() const {return m_pulseHeight;}

    /**
     * Set pulse height distribution of one of the HAPD channels
     * @param adc pulse height distribution
     */
    void setPulseHeightDistribution(TH1F* adc) {m_pulseHeight = adc;}


  private:
    std::string m_serial;                   /**< serial number of the sensor */
    float   m_qe400;                        /**< quantum efficiency at 400 nm */
    float   m_highVoltage;                  /**< Operational high voltage */
    float   m_guardVoltage;                 /**< guard HV (V) */
    float   m_current;                      /**< Current (A)  */
    ARICHHapdChipInfo* m_HAPDChipInfo[4];   /**< HapdChipInfo id of the i-th chip in the sensor */
    TGraph* m_quantumEfficiency;            /**< Quantum Efficiency as a function of wavelength */
    TH1F*   m_pulseHeight;                  /**< Pulse height distribution */

    ClassDef(ARICHHapdInfo, 1); /**< ClassDef */
  };
} // end namespace Belle2

