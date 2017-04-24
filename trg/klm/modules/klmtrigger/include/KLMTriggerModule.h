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
    virtual void endRun();
    virtual void terminate() { };

  private: // Parameters

    //! Create 2D hits from KLM Digits
    void fillHits();

    //! Create tracks from 2D hits
    void fillTracks();

    //! Calculate track parameters, chi squared, impact parameter and number of fired layers
    void calcChisq();

    //! Convert sector, layer and channel numbers to x, y, z coordinates
    //! @param fwd detector end (forward=true and backward=false)
    //! @param sector sector number
    //! @param layer layer number
    //! @param phiStrip phi strip number
    //! @param zStrip z strip number
    //! @param x x coordinate of the hit
    //! @param y y coordinate of the hit
    //! @param z x coordinate of the hit
    void geometryConverter(bool fwd, int sector, int layer, int phiStrip, int zStrip, int& x, int& y, int& z);

    //! Total number of sectors (eight, a constant)
    static const int c_TotalSectors = 8;
    //! Total number of layers (fifteen, a constant)
    static const int c_TotalLayers = 15;

    //! Trigger threshold on chisq
    double m_maxChisq;
    //! Trigger threshold on the impact parameter
    double m_maxIP;
    //! Trigger threshold on the number of fired layers
    int m_minLayers;

    //! Total number of processed events
    int m_nEvents;
    //! Total number of found tracks
    int m_nTracks;

    /**name of StoreArray Tracks*/
    std::string m_klmtrackCollectionName;
    /**name of StoreArray Hits*/
    std::string m_klmhitCollectionName;
  };
} // namespace Belle2

#endif // KLMTRIGGERMODULE_H

