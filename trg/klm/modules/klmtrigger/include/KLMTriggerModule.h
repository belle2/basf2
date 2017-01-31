/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Dmitri Liventsev                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef KLMTRIGGERMODULE_H
#define KLMTRIGGERMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  class KLMTriggerModule : public Module {
  public:

    // Constructor
    KLMTriggerModule();

    // Destructor
    virtual ~KLMTriggerModule() { };

    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun() { };
    virtual void terminate() { };

  private: // Parameters

    void fillHits();
    void fillTracks();
    void calcChisq();
    void geometryConverter(bool fwd, int sector, int layer, int phiStrip, int zStrip, double& x, double& y, double& z);

    const int nSectors = 8;
    const int nLayers = 15;

    // Debug level
    int _debugLevel;

    double m_maxChisq;
    double m_maxIP;
    int m_minFiredLayers;
  };

} // namespace Belle2

#endif // KLMTRIGGERMODULE_H
