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
#include <string>
#include <vector>
#include <TObject.h>
#include <TH1S.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>
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
    ARICHHapdQA(): m_hitData2D(NULL)
    {
      m_leakCurrent.clear();
      m_noise.clear();
      m_hitCount.clear();
    };

    /**
     * Constructor
     */

    ARICHHapdQA(std::string serial, TTimeStamp measurementDate, std::vector<TGraph*> leakCurrent, TH2F* hitData2D,
                std::vector<TGraph*> noise, std::vector<TH1S*> hitCount): m_serial(serial), m_measurementDate(measurementDate),
      m_leakCurrent(leakCurrent),
      m_hitData2D(hitData2D), m_noise(noise), m_hitCount(hitCount)
    { };

    /**
     * Constructor
     */

    ARICHHapdQA(std::vector<TGraph*> leakCurrent, TH2F* hitData2D, std::vector<TGraph*> noise,
                std::vector<TH1S*> hitCount): m_leakCurrent(leakCurrent),
      m_hitData2D(hitData2D), m_noise(noise), m_hitCount(hitCount)
    { };


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
    TTimeStamp getHapdMeasurementDate() const {return m_measurementDate; }

    /** Set HAPD measurement date
     * @param HAPD measurement date
     */
    void setHapdMeasurementDate(TTimeStamp measurementDate) {m_measurementDate = measurementDate; }

    /**
     * Return Leakeage Current as a function of bias voltage
     * @return chip gain
     */
    TGraph* getHapdLeakCurrent(unsigned int i) const { if (i < m_leakCurrent.size()) return m_leakCurrent[i]; else return NULL;}

    /**
     * set Leakeage Current as a function of bias voltage
     * @param current Leakeage current
     */
    void setHapdLeakCurrent(std::vector<TGraph*> leakCurrent)
    {
      for (unsigned int i = 0; i < leakCurrent.size(); i++) m_leakCurrent.push_back(leakCurrent[i]);
    }

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
    TGraph* getHapdNoise(unsigned int i) const { if (i < m_noise.size()) return m_noise[i]; else return NULL;}

    /**
     * set Noise level vs. bias voltage for each channel
     * @param noise level
     */
    void setHapdNoise(std::vector<TGraph*> noise)
    {
      for (unsigned int i = 0; i < noise.size(); i++)  m_noise.push_back(noise[i]);
    }

    /**
     * Return Hit count vs. threshold
     * @return Hit count vs. threshold
     */
    TH1S* getHapdHitCount(unsigned int i) const { if (i < m_hitCount.size()) return m_hitCount[i]; else return NULL;}

    /**
     * set Hit count vs. threshold
     * @param Hit count vs. threshold
     */
    void setHapdHitCount(std::vector<TH1S*> hitCount)
    {
      for (unsigned int i = 0; i < hitCount.size(); i++)  m_hitCount.push_back(hitCount[i]);
    }

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
    TTimeStamp m_measurementDate;         /**< date of measurement */
    std::vector<TGraph*> m_leakCurrent;   /**< Leakege Current as a function of bias voltage */
    TH2F* m_hitData2D;                    /**< 2D hit data */
    std::vector<TGraph*> m_noise;         /**< Noise level vs. bias voltage for each channel */
    std::vector<TH1S*> m_hitCount;        /**< Hit count vs. threshold for sampling 5 channels/chip at 2 voltages */
    std::string m_comment;                /**< Optional comment */

    ClassDef(ARICHHapdQA, 1);  /**< ClassDef */
  };
} // end namespace Belle2
