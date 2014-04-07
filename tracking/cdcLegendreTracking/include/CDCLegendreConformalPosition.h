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

#include <cdc/dataobjects/CDCHit.h>

namespace Belle2 {

  class CDCLegendreTrackCandidate;

  class CDCLegendreConformalPosition {
  public:

    CDCLegendreConformalPosition(int ninsTheta);


    static CDCLegendreConformalPosition& Instance(int ninsTheta);


  private:

    static CDCLegendreConformalPosition* s_cdcLegendreConformalPosition;

    int m_ninsTheta;


  };

}
