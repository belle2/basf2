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
#include <TObject.h>
#include <TH2F.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  /**
  *   Contains manufacturer data of the photo sensor - HAPD
  */
  class ARICHHapdQE: public TObject {
  public:

    /**
     * Default constructor
     */
    ARICHHapdQE(): m_serial(""), m_quantumEfficiency2D(NULL) {};

    /**
     * Constructor
     */

    ARICHHapdQE(const std::string& serial, TH2F* qe): m_serial(serial), m_quantumEfficiency2D(qe) { };

    /**
     * Destructor
     */
    ~ARICHHapdQE() {};

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

    /**
     * Return 2D quantum efficiency
     * @return 2D quantum efficiency
     */
    TH2F* getQuantumEfficiency2D() const {return m_quantumEfficiency2D;}

    /**
     * Set 2D quantum efficiency
     * @param quantum efficiency 2D
     */
    void setQuantumEfficiency2D(TH2F* qe) { m_quantumEfficiency2D = qe;}

  private:
    std::string m_serial;         /**< HAPD serial number */
    TH2F* m_quantumEfficiency2D;  /**< 2D quantum efficiency */

    ClassDef(ARICHHapdQE, 1);  /**< ClassDef */
  };
} // end namespace Belle2
