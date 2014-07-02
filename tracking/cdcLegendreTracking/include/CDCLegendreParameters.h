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

#include <framework/logging/Logger.h>

namespace Belle2 {

  class CDCLegendreParameters {

  public:
    CDCLegendreParameters(int minTrackHits, bool reconstructCurler, bool fitTracks, bool mergeTracks, bool appendHits) {
      s_minTrackHits = minTrackHits;
      s_reconstructCurler = reconstructCurler;
      s_fitTracks = fitTracks;
      s_mergeTracks = mergeTracks;
      s_appendHits = appendHits;
    };

    ~CDCLegendreParameters();

    static CDCLegendreParameters& getInstance() {
      if (!s_cdcLegendreParameters) B2FATAL("CDCLegendreParameters instance is not initialized!");
      return *s_cdcLegendreParameters;
    }

    static void createInstance(int minTrackHits, bool reconstructCurler, bool fitTracks, bool mergeTracks, bool appendHits) {
      if (!s_cdcLegendreParameters) {
        s_cdcLegendreParameters = new CDCLegendreParameters(minTrackHits, reconstructCurler, fitTracks, mergeTracks, appendHits);
      }
    }

    int getMinTrackHits() const {return s_minTrackHits;}

    bool getReconstructCurler() const {return s_reconstructCurler;}

    bool getFitTracks() const {return s_fitTracks;}

    bool getMergeTracks() const {return s_mergeTracks;}

    bool getAppendHits() const {return s_appendHits;}

  private:

    static int s_minTrackHits;
    static bool s_reconstructCurler;
    static bool s_fitTracks;
    static bool s_mergeTracks;
    static bool s_appendHits;

    static CDCLegendreParameters* s_cdcLegendreParameters;


  };

}
