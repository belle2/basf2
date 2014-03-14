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

#include "tracking/cdcLegendreTracking/CDCLegendreNiceDrawing.h"
#include <framework/core/Module.h>

#include "cdc/geometry/CDCGeometryPar.h"
#include "cdc/translators/SimpleTDCCountTranslator.h"

#include <fstream>
#include "genfit/Track.h"
#include "genfit/TrackCand.h"

#include "TF1.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TAxis.h"
#include "TEllipse.h"


namespace Belle2 {

  class CDCLegendreTrackCandidate;
  class CDCLegendreTrackHit;

  class CDCLegendreTrackDrawer : public CDCLegendreNiceDrawing {

  public:

    CDCLegendreTrackDrawer(bool drawCandInfo, bool drawCandidates);

    virtual ~CDCLegendreTrackDrawer() {};

    virtual void initialize();

    virtual void beginRun() {};

    virtual void event();

    virtual void endRun() {};

    virtual void terminate() {};

    void drawTrackCand(CDCLegendreTrackCandidate* TrackCand);

    void finalizeFile();

    void openFileAgain();

    void showPicture();


    void drawConformalHits(std::vector<CDCLegendreTrackHit*> trackHitList, int ntrack, bool do_print);
    void drawLegendreHits(std::vector<CDCLegendreTrackHit*> trackHitList, int ntrack, bool do_print);
    Color_t getRootColor(int i);
    void initRootColorVec();
    void drawHitsROOT(std::vector<CDCLegendreTrackHit*>& hits_vector); /**<Method for drawing cdchits in TCanvas (in legendre and conformal spaces)*/
    void finalizeROOTFile(std::vector<CDCLegendreTrackHit*>& hits_vector);
  protected:

    bool m_drawCandidates;
    bool m_drawCandInfo;
    int m_iTrack;
    FILE* pipeout_fp;

    //for ROOT drawing:
    std::vector<TF1*> m_hitsLegendre;
    std::vector<TEllipse*> m_hitsConformal;
    std::vector<Color_t> m_colorRootVec;

    double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
    double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/
    static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
    static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/


  };
}
