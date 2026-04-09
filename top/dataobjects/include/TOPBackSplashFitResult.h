#pragma once

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  class TOPBackSplashFitResult : public RelationsObject {
  public:
    //TOPBackSplashFitResult() = default;
    //~TOPBackSplashFitResult() override = default;
    TOPBackSplashFitResult() :
      m_time{0.0},
      m_chisqdof{0.0},
      m_nPhotons{0}
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
