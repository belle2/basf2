/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class TOPBackSplashFitResult : public RelationsObject {
  public:
    //TOPBackSplashFitResult() = default;
    //~TOPBackSplashFitResult() override = default;
    TOPBackSplashFitResult() :
      m_time{-1.0},
      m_chisqdof{-1.0},
      m_nPhotons{-1}
    {}

    void setTime(double t) { m_time = t; }
    double getTime() const { return m_time; }

    void setChisqdof(double chisqdof) { m_chisqdof = chisqdof; }
    double getChisqdof() const { return m_chisqdof; }

    void setNphotons(int nPhotons) { m_nPhotons = nPhotons; }
    int getNphotons() const { return m_nPhotons; }

  private:
    double m_time;
    double m_chisqdof;
    int m_nPhotons;

    ClassDef(TOPBackSplashFitResult, 1);
  };

}
