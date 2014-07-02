/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {

  class CDCLegendreWireCenter {
  public:
    CDCLegendreWireCenter();

    ~CDCLegendreWireCenter();

    /**
     * Get static instance of the class
     */
    static CDCLegendreWireCenter& Instance();

    /**
     * Get center of wire
     */
    inline double getCenter(int iCLayer) const {return m_wireCentersArray[iCLayer];};

  private:

    double m_wireCentersArray[56];
    static CDCLegendreWireCenter* s_cdcLegendreWireCenter;
  };
}
