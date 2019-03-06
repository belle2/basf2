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
    CDCWireHitRequirements(double chargeCut) : m_chargeCut(chargeCut)
    {
    }

    /**
     * Destructor
     */
    ~CDCWireHitRequirements()
    {
    }

    /** Getter for FADC threshold */
    double getChargeCut() const
    {
      return m_chargeCut;
    }

    /** Setter for FADC threshold */
    void setChargeCut(double chargeCut)
    {
      m_chargeCut = chargeCut;
    }

    /** Print content */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "CDCWireHit requirements" << std::endl;
      std::cout << "Charge > " << getChargeCut() << std::endl;
    }

  private:
    double m_chargeCut = 0.0;          /**< Cut value for FADC */

    ClassDef(CDCWireHitRequirements, 2); /**< ClassDef */
  };

} // end namespace Belle2
