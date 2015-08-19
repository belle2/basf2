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
#include <TH1F.h>
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
     * Various constants
     */
    enum {c_NumOfChips = 4,      /**< number of HAPD Chips on the sensor */
          c_NumOfChannels = 144, /**< number of channels on the sensors */
          c_Measurement = 3,      /**< number of measurements */
          c_Voltage = 2,          /**< measurements at 2 different voltages */
         };

    /**
     * Default constructor
     */
    ARICHHapdQA(): m_hitData2D(NULL)
    {
      for (unsigned i = 0; i < c_Measurement; i++) {
        for (unsigned j = 0; j < c_NumOfChips; j++) {
          m_leakCurrent[i][j] = NULL;
        }
      }
      m_noise = {};
      m_hitCount = {};
    };

    ARICHHapdQA(TGraph** * leakCurrent, TH2F* hitData2D, std::vector<TGraph*> noise, std::vector<TH1F*> hitCount)
    {
      for (unsigned i = 0; i < c_Measurement; i++) {
        for (unsigned j = 0; j < c_NumOfChips; j++) {
          m_leakCurrent[i][j] = leakCurrent[i][j];
        }
      }
      m_hitData2D = hitData2D;
      m_noise = noise;
      m_hitCount = hitCount;
    };

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
    TGraph* getHapdLeakCurrent(unsigned int j, unsigned int i) { if (j < c_Measurement && i < c_NumOfChips) return m_leakCurrent[j][i]; else return NULL;}

    /**
     * set Leakeage Current as a function of bias voltage
     * @param current Leakeage current
     */
    void setHapdLeakCurrent(unsigned int j, unsigned int i,  TGraph* leakCurrent) { if (j < c_Measurement && i < c_NumOfChips) m_leakCurrent[j][i] = leakCurrent; }

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
    TGraph* getHapdNoise(unsigned int i) { if (i < m_noise.size()) return m_noise[i]; else return NULL;}

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
    TH1F* getHapdHitCount(unsigned int i) { if (i < m_hitCount.size()) return m_hitCount[i]; else return NULL;}

    /**
     * set Hit count vs. threshold
     * @param Hit count vs. threshold
     */
    void setHapdHitCount(std::vector<TH1F*> hitCount)
    {
      for (unsigned int i = 0; i < hitCount.size(); i++)  m_hitCount.push_back(hitCount[i]);
    }

    /** Return comment
     * @return comment
     */
    std::string getHapdComment();

    /** Set comment
     * @param comment
     */
    void setHapdComment();


  private:
    std::string m_serial;                                /**< serial number of the sensor */
    TTimeStamp m_measurementDate;                        /**< date of measurement */
    TGraph* m_leakCurrent[c_Measurement][c_NumOfChips];  /**< Leakege Current as a function of bias voltage */
    TH2F* m_hitData2D;                                   /**< 2D hit data */
    std::vector<TGraph*> m_noise;                        /**< Noise level vs. bias voltage for each channel */
    std::vector<TH1F*> m_hitCount;                       /**< Hit count vs. threshold for sampling 5 channels/chip at 2 voltages */
    std::string m_comment;                               /**< Optional comment */

    ClassDef(ARICHHapdQA, 1);  /**< ClassDef */
  };
} // end namespace Belle2






