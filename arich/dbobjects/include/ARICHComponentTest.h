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
#include <TTimeStamp.h>

namespace Belle2 {
  /**
  *   Test of the ARICH component
  */

  class ARICHComponentTest: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHComponentTest(): m_id(0), m_componentID(""), m_timeStamp(), m_type(0), m_data(NULL), m_comment("") {};

    /**
     * Constructor
     */
    ARICHComponentTest(int id, TTimeStamp timeStamp, int type, TObject* data): m_id(id), m_componentID(""), m_timeStamp(timeStamp),
      m_type(type), m_data(data), m_comment("") {};

    /**
     * Destructor
     */
    ~ARICHComponentTest() {};


    /** Return Test Identifier
     * @return Test Identifier
     */
    int getTestID() const {return m_id; }

    /**
     * Set Test Identifier
     * @param Test Identifier
     */
    void setTestID(int id) { m_id = id;}

    /** Return Component Identifier
     * @return Component Identifier
     */
    std::string getComponentID();

    /** Set Component Identifier
     * @param Component Identifier
     */
    void setComponentID(const std::string& componentID) {m_componentID = componentID; }

    /** Return Date of test measurement
     * @return Date of test measurement
     */
    TTimeStamp getTestTimeStamp() const {return m_timeStamp; }

    /** Set Date of test measurement
     * @param Date of test measurement
     */
    void setTestTimeStamp(TTimeStamp timeStamp) {m_timeStamp = timeStamp; }

    /** Return measurement type
     * @return measurement type
     */
    int getMeasurementType() const {return m_type; }

    /**
     * Set measurement type
     * @param mwasurement type
     */
    void setMeasurementType(int type) { m_type = type;}

    /**
     * Return Measurement data
     * @return Measurement data
     */
    TObject* getMeasurementData() const {return m_data;}

    /**
     * Set Measurement data
     * @param Measurement data
     */
    void setMeasurementData(TObject* data) {m_data = data;}


    /** Return comment
     * @return comment
     */
    std::string getComponentTestComment();

    /** Set comment
     * @param comment
     */
    void setComponentTestComment(const std::string& comment) {m_comment = comment; }

  private:
    int m_id;                    /**< Test Identfier */
    std::string m_componentID;   /**< Component identifier */
    TTimeStamp  m_timeStamp;     /**< Date of test measurement */
    int m_type;                  /**< Measurement type */
    TObject* m_data;             /**< Measurement data */
    std::string m_comment;       /**< optional comment */

    ClassDef(ARICHComponentTest, 1); /**< ClassDef */
  };
} // end namespace Belle2
