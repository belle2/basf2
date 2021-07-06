/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <TObject.h>
#include <TH1S.h>
#include <TH2F.h>
#include <TTimeStamp.h>
#include <TGraph.h>

namespace Belle2 {
  /**
  *   Contains manufacturer data of the photo sensor - HAPD
  */
  class ARICHHapdQA: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHHapdQA(): m_serial(""), m_arrivalDate(), m_leakCurrent(), m_hitData2D(NULL), m_noise(), m_hitCount(),
      m_comment("") {};

    /**
     * Constructor
     */

    ARICHHapdQA(const std::string& serial, TTimeStamp arrivalDate, std::vector<TGraph*>& leakCurrent, TH2F* hitData2D,
                std::vector<TGraph*>& noise, std::vector<TH1S*>& hitCount): m_serial(serial), m_arrivalDate(arrivalDate),
      m_leakCurrent(leakCurrent), m_hitData2D(hitData2D), m_noise(noise), m_hitCount(hitCount), m_comment("") { };


    /**
     * Destructor
     */
    ~ARICHHapdQA() {};

    /**
     * Return HAPD Serial Number
     * @return serial number
     */
    std::string getHapdSerialNumber() const {return m_serial;}

    /**
     * Set HAPD Serial Number
     * @param serial serial number
     */
    void setHapdSerialNumber(const std::string& serial) {m_serial = serial; }

    /** Return HAPD measurement date
     * @return HAPD measurement date
     */
    TTimeStamp getHapdArrivalDate() const {return m_arrivalDate; }

    /** Set HAPD measurement date
     * @param HAPD measurement date
     */
    void setHapdArrivalDate(TTimeStamp arrivalDate) {m_arrivalDate = arrivalDate; }

    /**
     * Return Leakeage Current as a function of bias voltage
     * @return chip gain
     */
    TGraph* getHapdLeakCurrent(unsigned int i) const;

    /**
     * set Leakeage Current as a function of bias voltage
     * @param current Leakeage current
     */
    void setHapdLeakCurrent(std::vector<TGraph*> leakCurrent);

    /**
     * Return 2D hit data
     * @return 2D hit data
     */
    TH2F* getHapdHitData2D() const {return m_hitData2D;}

    /**
     * Set 2D hit data
     * @param hit data 2D
     */
    void setHapdHitData2D(TH2F* hitData2D) { m_hitData2D = hitData2D;}

    /**
     * Return Noise level vs. bias voltage for each channel
     * @return noise level
     */
    TGraph* getHapdNoise(unsigned int i) const;

    /**
     * set Noise level vs. bias voltage for each channel
     * @param noise level
     */
    void setHapdNoise(std::vector<TGraph*> noise);

    /**
     * Return Hit count vs. threshold
     * @return Hit count vs. threshold
     */
    TH1S* getHapdHitCount(unsigned int i) const;

    /**
     * set Hit count vs. threshold
     * @param Hit count vs. threshold
     */
    void setHapdHitCount(std::vector<TH1S*> hitCount);

    /** Return comment
     * @return comment
     */
    std::string getHapdComment() const {return m_comment; }

    /** Set comment
     * @param comment
     */
    void setHapdComment(const std::string& comment) {m_comment = comment; }


  private:
    std::string m_serial;                 /**< serial number of the sensor */
    TTimeStamp m_arrivalDate;             /**< date of HAPD's arrival */
    std::vector<TGraph*> m_leakCurrent;   /**< Leakege Current as a function of bias voltage */
    TH2F* m_hitData2D;                    /**< 2D hit data */
    std::vector<TGraph*> m_noise;         /**< Noise level vs. bias voltage for each channel */
    std::vector<TH1S*> m_hitCount;        /**< Hit count vs. threshold for sampling 5 channels/chip at 2 voltages */
    std::string m_comment;                /**< Optional comment */

    ClassDef(ARICHHapdQA, 2);  /**< ClassDef */
  };
} // end namespace Belle2
