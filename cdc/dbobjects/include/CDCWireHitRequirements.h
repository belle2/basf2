/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <iostream>
#include <TObject.h>

namespace Belle2 {
  /**
   * Database object containing cut values to filter CDCWireHits
   */
  class CDCWireHitRequirements: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCWireHitRequirements() {}

    /**
     * Constructor
     */
    CDCWireHitRequirements(int minADC, int minTOT, int maxTOT, double minADCOverTOT) :
      m_minADC(minADC), m_minTOT(minTOT), m_maxTOT(maxTOT), m_minADCOverTOT(minADCOverTOT)
    {
    }

    /**
     * Destructor
     */
    ~CDCWireHitRequirements()
    {
    }

    /** Getter for ADC min threshold */
    int getMinADC() const
    {
      return m_minADC;
    }

    /** Getter for TOT min threshold */
    int getMinTOT() const
    {
      return m_minTOT;
    }

    /** Getter for TOT max threshold */
    int getMaxTOT() const
    {
      return m_maxTOT;
    }

    /** Getter for ADC/TOT min threshold */
    double getMinADCOverTOT() const
    {
      return m_minADCOverTOT;
    }


    /** Print content */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "CDCWireHit requirements" << std::endl;
      std::cout << "ADC > " << getMinADC() << std::endl;
      std::cout << "TOT > " << getMinTOT() << std::endl;
      std::cout << "TOT < " << getMaxTOT() << std::endl;
      std::cout << "ADC/TOT > " << getMinADCOverTOT() << std::endl;
    }

  private:
    int m_minADC = 0.0;          // Cut value for ADC
    int m_minTOT = 0.0;          // Cut value for TOT
    int m_maxTOT = 100;          // Cut value for TOT
    double m_minADCOverTOT = 0.0;      // Cut value for ADC/TOT

    ClassDef(CDCWireHitRequirements, 3); // ClassDef
  };

} // end namespace Belle2
