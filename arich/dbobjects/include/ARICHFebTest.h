/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Manca Mrvar                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <string>
#include <cmath>
#include <vector>
#include <utility>
#include <TTimeStamp.h>
#include <TH2F.h>
#include <TH3F.h>

#include <TROOT.h>
#include <TClass.h>

namespace Belle2 {

  /**
   * The Class for ARICH Aerogel Parameters.
   */

  class ARICHFebTest: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHFebTest(): m_serial(0), m_dna(""), m_runSlowC(""), m_timeSlowC(), m_tmon0(0.0), m_tmon1(0.0), m_vdd(0.0), m_v2p(0.0),
      m_v2n(0.0), m_vss(0.0), m_vth1(0.0), m_vth2(0.0), m_vcc12(0.0), m_vcc15(0.0), m_vcc25(0.0), m_v38p(0.0), m_runLV(""), m_timeLV(),
      m_currentV20p(0.0), m_currentV21n(0.0), m_currentV38p(0.0), m_runHV(""), m_timeHV(), m_currentV99p(0.0), m_deadChannel(),
      m_testPulse(NULL), m_offsetRough(NULL),  m_offsetFine(NULL),  m_slopesRough(), m_slopesFine(), m_fwhm(), m_comment("") {};


    /**
     * Constructor
     */
    explicit ARICHFebTest(int serial): m_serial(serial), m_dna(""), m_runSlowC(""), m_timeSlowC(), m_tmon0(0.0), m_tmon1(0.0),
      m_vdd(0.0), m_v2p(0.0), m_v2n(0.0), m_vss(0.0), m_vth1(0.0), m_vth2(0.0), m_vcc12(0.0), m_vcc15(0.0), m_vcc25(0.0), m_v38p(0.0),
      m_runLV(""), m_timeLV(), m_currentV20p(0.0), m_currentV21n(0.0), m_currentV38p(0.0), m_runHV(""), m_timeHV(), m_currentV99p(0.0),
      m_deadChannel(), m_testPulse(NULL), m_offsetRough(NULL), m_offsetFine(NULL), m_slopesRough(), m_slopesFine(), m_fwhm(),
      m_comment("") {};

    /**
     * Destructor
     */
    ~ARICHFebTest() {};

    /** Return FEB serial number
     * @return FEB serial number
     */

    int getFebSerial() const {return m_serial; }

    /** Set FEB serial number
     * @param FEB serial number
     */
    void setFebSerial(int serial) {m_serial = serial; }

    /** Return FEB dna number
     * @return FEB dna number
     */
    std::string getFebDna() const {return m_dna; }

    /** Set slow control run number
     * @param slow control run number
     */
    void setRunSlowC(const std::string& runSlowC) {m_runSlowC = runSlowC; }

    /** Return slow control number
     * @return slow control number
     */
    std::string getRunSlowC() const {return m_runSlowC; }

    /** Set FEB dna number
     * @param FEB dna number
     */
    void setFebDna(const std::string& dna) {m_dna = dna; }

    /** Return Test date (slow control measurement)
     * @return Test date (slow control measurement)
     */
    TTimeStamp getTimeSlowC() const {return m_timeSlowC; }

    /** Set Test date (slow control measurement)
     * @param Test date (slow control measurement)
     */
    void setTimeSlowC(TTimeStamp timeSlowC) {m_timeSlowC = timeSlowC; }

    /** Return Temperature 0
     * @return Temperature 0
     */
    float getTemperature0() const {return m_tmon0; }

    /** Set Temperature 0
     * @param Temperature 0
     */
    void setTemperature0(float tmon0) {m_tmon0 = tmon0; }

    /** Return Temperature 1
     * @return Temperature 1
     */
    float getTemperature1() const {return m_tmon1; }

    /** Set Temperature 1
     * @param Temperature 1
     */
    void setTemperature1(float tmon1) {m_tmon1 = tmon1; }

    /** Return Voltage - positive supply voltage
     * @return Voltage - positive supply voltage
     */
    float getVdd() const {return m_vdd; }

    /** Set Voltage - positive supply voltage
     * @param Voltage - positive supply voltage
     */
    void setVdd(float vdd) {m_vdd = vdd; }

    /** Return Voltage - (+2.0) V
     * @return Voltage - (+2.0) V
     */
    float getV2p() const {return m_v2p; }

    /** Set Voltage - (+2.0) V
     * @param Voltage - (+2.0) V
     */
    void setV2p(float v2p) {m_v2p = v2p; }

    /** Return Voltage - (-2.0) V
     * @return Voltage - (-2.0) V
     */
    float getV2n() const {return m_v2n; }

    /** Set Voltage - (-2.0) V
     * @param Voltage - (-2.0) V
     */
    void setV2n(float v2n) {m_v2n = v2n; }

    /** Return Voltage - negative supply voltage
     * @return Voltage - negative supply voltage
     */
    float getVss() const {return m_vss; }

    /** Set Voltage - negative supply voltage
     * @param Voltage - negative supply voltage
     */
    void setVss(float vss) {m_vss = vss; }

    /** Return Voltage - threshold voltage 1
     * @return Voltage - threshold voltage 1
     */
    float getVth1() const {return m_vth1; }

    /** Set Voltage - threshold voltage 1
     * @param Voltage - threshold voltage 1
     */
    void setVth1(float vth1) {m_vth1 = vth1; }

    /** Return Voltage - threshold voltage 1
     * @return Voltage - threshold voltage 1
     */
    float getVth2() const {return m_vth2; }

    /** Set Voltage - threshold voltage 1
     * @param Voltage - threshold voltage 1
     */
    void setVth2(float vth2) {m_vth2 = vth2; }

    /** Return Voltage - supply voltage 1.2 V
     * @return Voltage - supply voltage 1.2 V
     */
    float getVcc12() const {return m_vcc12; }

    /** Set Voltage - supply voltage 1.2 V
     * @param Voltage - supply voltage 1.2 V
     */
    void setVcc12(float vcc12) {m_vcc12 = vcc12; }

    /** Return Voltage - supply voltage 1.5 V
     * @return Voltage - supply voltage 1.5 V
     */
    float getVcc15() const {return m_vcc15; }

    /** Set Voltage - supply voltage 1.5 V
     * @param Voltage - supply voltage 1.5 V
     */
    void setVcc15(float vcc15) {m_vcc15 = vcc15; }

    /** Return Voltage - supply voltage 2.5 V
     * @return Voltage - supply voltage 2.5 V
     */
    float getVcc25() const {return m_vcc25; }

    /** Set Voltage - supply voltage 2.5 V
     * @param Voltage - supply voltage 2.5 V
     */
    void setVcc25(float vcc25) {m_vcc25 = vcc25; }

    /** Return Voltage - (+3.8) V
     * @return Voltage - (+3.8) V
     */
    float getV38p() const {return m_v38p; }

    /** Set Voltage - (+3.8) V
     * @param Voltage - (+3.8) V
     */
    void setV38p(float v38p) {m_v38p = v38p; }

    /** Set LV test run number
     * @param LV test run number
     */
    void setRunLV(const std::string& runLV) {m_runLV = runLV; }

    /** Return LV test number
     * @return LV test number
     */
    std::string getRunLV() const {return m_runLV; }

    /** Return Test date (LV test)
     * @return Test date (LV test)
     */
    TTimeStamp getTimeLV() const {return m_timeLV; }

    /** Set Test date (LV test)
     * @param Test date (LV test)
     */
    void setTimeLV(TTimeStamp timeLV) {m_timeLV = timeLV; }

    /** Return current at voltage (+2.0) V
     * @return current at voltage (+2.0) V
     */
    float getCurrentV20p() const {return m_currentV20p; }

    /** Set current at voltage (+2.0) V
     * @param current at voltage (+2.0) V
     */
    void setCurrentV20p(float currentV20p) {m_currentV20p = currentV20p; }

    /** Return current at voltage (-2.0) V
     * @return current at voltage (-2.0) V
     */
    float getCurrentV21n() const {return m_currentV21n; }

    /** Set current at voltage (-2.0) V
     * @param current at voltage (-2.0) V
     */
    void setCurrentV21n(float currentV21n) {m_currentV21n = currentV21n; }

    /** Return current at voltage (+3.8) V
     * @return current at voltage (+3.8) V
     */
    float getCurrentV38p() const {return m_currentV38p; }

    /** Set current at voltage (+3.8) V
     * @param current at voltage (+3.8) V
     */
    void setCurrentV38p(float currentV38p) {m_currentV38p = currentV38p; }

    /** Set HV test run number
     * @param HV test run number
     */
    void setRunHV(const std::string& runHV) {m_runHV = runHV; }

    /** Return HV test number
     * @return HV test number
     */
    std::string getRunHV() const {return m_runHV; }

    /** Return Test date (HV test)
     * @return Test date (HV test)
     */
    TTimeStamp getTimeHV() const {return m_timeHV; }

    /** Set Test date (HV test)
     * @param Test date (HV test)
     */
    void setTimeHV(TTimeStamp timeHV) {m_timeHV = timeHV; }

    /** Return current at voltage (+99) V
     * @return current at voltage (+99) V
     */
    float getCurrentV99p() const {return m_currentV99p; }

    /** Set current at voltage (+99) V
     * @param current at voltage (+99) V
     */
    void setCurrentV99p(float currentV99p) {m_currentV99p = currentV99p; }

    /**
     * Return a channel number from the list of dead channels
     * @param i index of the element in the list
     * @return channel id
     */
    int getDeadChannel(unsigned int i) const;

    /**
     * Add a channel number to the list of dead channels
     * @param channel FEB channel id
     */
    void appendDeadChannel(int channel) {m_deadChannel.push_back(channel); }

    /**
     * Set vector of dead channel numbers
     * @param channel FEB channel id
     */
    void setDeadChannels(const std::vector<int>& deadChannels) {m_deadChannel = deadChannels; }

    /**
     * Return size of the list of dead channels
     * @return size
     */
    int getDeadChannelsSize() const {return m_deadChannel.size();}

    /**
     * Return test pulse
     * @return 2D test pulse
     */
    TH2F* getTestPulse2D() const {return m_testPulse;}

    /**
     * Set test pulse
     * @param 2D test pulse
     */
    void setTestPulse2D(TH2F* testPulse) { m_testPulse = testPulse;}

    /**
     * Return threshold scans with rough offset settings
     * @return 3D threshold scans with rough offset settings
     */
    TH3F* getOffsetRough3D() const {return m_offsetRough;}

    /**
     * Set threshold scans with rough offset settings
     * @param 3D threshold scans with rough offset settings
     */
    void setOffsetRough3D(TH3F* offsetRough) { m_offsetRough = offsetRough;}

    /**
     * Return threshold scans with fine offset settings
     * @return 3D threshold scans with fine offset settings
     */
    TH3F* getOffsetFine3D() const {return m_offsetFine;}

    /**
     * Set threshold scans with fine offset settings
     * @param 3D threshold scans with fine offset settings
     */
    void setOffsetFine3D(TH3F* offsetFine) { m_offsetFine = offsetFine;}

    /**
     * Return slope[mV/offset step] for i-th channel (rough settings)
     * @param i FEB channel number
     * @return slope[mV/offset step]
     */
    float getSlopeRough(unsigned int i) const;

    /**
     * Add slope[mV/offset step] for i-th channel (rough settings)
     * @param slope[mV/offset step]
     */
    void appendSlopeRough(float slope) {m_slopesRough.push_back(slope); }

    /**
     * Set vector of slopes (rough settings)
     * @param vector of slopes[mV/offset step]
     */
    void setSlopesRough(const std::vector<float>& slopesRough) {m_slopesRough = slopesRough; }

    /**
     * Return size of the list of slopes (rough settings)
     * @return size
     */
    int getSlopesRoughSize() const {return m_slopesRough.size();}

    /**
     * Return slope[mV/offset step] for i-th channel (fine settings)
     * @param i FEB channel number
     * @return slope[mV/offset step]
     */
    float getSlopeFine(unsigned int i) const;

    /**
     * Add slope[mV/offset step] for i-th channel (fine settings)
     * @param slope[mV/offset step]
     */
    void appendSlopeFine(float slope) {m_slopesFine.push_back(slope); }

    /**
     * Set vector of slopes (fine settings)
     * @param vector of slopes[mV/offset step]
     */
    void setSlopesFine(const std::vector<float>& slopesFine) {m_slopesFine = slopesFine; }

    /**
     * Return size of the list of slopes (fine settings)
     * @return size
     */
    int getSlopesFineSize() const {return m_slopesFine.size();}

    /**
     * Return FWHM value&sigma for i-th channel
     * @param i FEB channel number
     * @return FWHM values&sigma
     */
    std::pair<float, float> getFWHM(unsigned int i) const;

    /**
     * Add FWHM value&sigma for i-th channel
     * @param FWHM value&sigma
     */
    void appendFWHM(const std::pair<float, float>& fwhm) {m_fwhm.push_back(fwhm); }

    /**
     * Set vector of FWHM values&sigma
     * @param vector of FWHM values&sigma
     */
    void setFWHM(const std::vector<std::pair<float, float>>& fwhm) {m_fwhm = fwhm; }

    /**
     * Return size of the list of FWHM
     * @return size
     */
    int getFWHMSize() const {return m_fwhm.size();}

    /**
     * Return FWHM value for i-th channel
     * @param i FEB channel number
     * @return FWHM value
     */
    float getFWHMvalue(unsigned int i) const;

    /**
     * Return FWHM sigma for i-th channel
     * @param i FEB channel number
     * @return FWHM sigma
     */
    float getFWHMsigma(unsigned int i) const;

    /** Set comment
     * @param comment
     */
    void setComment(const std::string& comment) {m_comment = comment; }

    /** Return comment
     * @return comment
     */
    std::string getComment() const {return m_comment; }

  private:

    int m_serial;                    /**< FEB serial number */
    std::string m_dna;               /**< FEB dna */
    std::string m_runSlowC;          /**< Run number of FEB slow control measurements */
    TTimeStamp m_timeSlowC;          /**< Test Date of FEB slow control measurements */
    float m_tmon0;                   /**< Temperature 0 */
    float m_tmon1;                   /**< Temperature 1 */
    float m_vdd;                     /**< Voltage - positive supply voltage */
    float m_v2p;                     /**< Voltage - (+2.0) V */
    float m_v2n;                     /**< Voltage - (-2.0) V */
    float m_vss;                     /**< Voltage - negative supply voltage */
    float m_vth1;                    /**< Voltage - threshold voltage 1 */
    float m_vth2;                    /**< Voltage - threshold voltage 2 */
    float m_vcc12;                   /**< Voltage - supply voltage 1.2 V */
    float m_vcc15;                   /**< Voltage - supply voltage 1.5 V */
    float m_vcc25;                   /**< Voltage - supply voltage 2.5 V */
    float m_v38p;                    /**< Voltage - (+3.8) V */
    std::string m_runLV;             /**< Run number of FEB low voltage test */
    TTimeStamp m_timeLV;             /**< Test Date of FEB low voltage test */
    float m_currentV20p;             /**< Current at 2.0 V */
    float m_currentV21n;             /**< Current at -2.1 V */
    float m_currentV38p;             /**< Current at 3.8 V */
    std::string m_runHV;             /**< Run number of FEB high voltage test */
    TTimeStamp m_timeHV;             /**< Test Date of FEB high voltage test */
    float m_currentV99p;             /**< Current at 99 V */
    std::vector<int> m_deadChannel;  /**< List of dead channels on the FEB */
    TH2F* m_testPulse;               /**< Test pulse scan */
    TH3F* m_offsetRough;             /**< Threshold scans with rough offset settings */
    TH3F* m_offsetFine;              /**< Threshold scans with fine offset settings */
    std::vector<float> m_slopesRough;           /**< Slopes for each channel (rough settings) [mV/step] */
    std::vector<float> m_slopesFine;            /**< Slopes for each channel (fine settings) [mV/step] */
    std::vector<std::pair<float, float>> m_fwhm; /**< FWHM and sigma for each channel */
    std::string m_comment;           /**< Optional comment */

    ClassDef(ARICHFebTest, 2);  /**< ClassDef */
  };
} // end namespace Belle2
