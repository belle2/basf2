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
  namespace TrackFinderCDCLegendre {

    class TrackCandidate;
    class TrackHit;

    /** Class for in-module drawing (step-by-step drawing). For testing purposes only; contains hardcoded values and pathes*/
    class TrackDrawer : public NiceDrawing {

    public:

      /**
       * @brief Constructor
       * @param drawCandInfo Sets whether conformal and legendre transformation should be drawn
       * @param drawCandidates Sets whether drawing while basf2 running should be done
       */
      TrackDrawer(bool drawCandInfo, bool drawCandidates);

      virtual ~TrackDrawer() {};

      /**
       * Prepare variables and CDC stucture
       */
      void initialize();

      /**
       * Process event
       */
      void event();

      /**
       * Draw given track candidate
       */
      void drawTrackCand(TrackCandidate* TrackCand);

      /**
       * Finalize svg file
       */
      void finalizeFile();

      /**
       * Reopen file
       */
      void openFileAgain();

      /**
       * Show picture during basf2 run
       */
      void showPicture();


      /**
       * Draw cdc hits in conformal space
       */
      void drawConformalHits(std::vector<TrackHit*> trackHitList, int ntrack, bool do_print);

      /**
       * Draw cdc hits in legendre space
       */
      void drawLegendreHits(std::vector<TrackHit*> trackHitList, int ntrack, bool do_print);

      /**
       * Get color by index (ROOT drawings)
       */
      Color_t getRootColor(int i);

      /**
       * Initialize ROOT colors vector
       */
      void initRootColorVec();

      /**
       * Draw hits in legendre and conformal space using ROOT
       */
      void drawHitsROOT(std::vector<TrackHit*>& hits_vector); /**<Method for drawing cdchits in TCanvas (in legendre and conformal spaces)*/

      /**
       * Finalize ROOT drawing
       */
      void finalizeROOTFile(std::vector<TrackHit*>& hits_vector);

    protected:

      bool m_drawCandInfo; /**< Checks whether conformal and legendre transformation should be drawn */
      bool m_drawCandidates;  /**< Checks whether drawing while basf2 running should be done */
      int m_iTrack; /**< Track counter */
//    FILE* pipeout_fp; /**< Chacks whether conformal and legendre transformation should be drawn */
      int m_trackCounter; /**< Track counter; used for saving track finding progress */

      //for ROOT drawing:
      std::vector<TF1*> m_hitsLegendre; /**< Holds cdc hits transformation into legendre space */
      std::vector<TEllipse*> m_hitsConformal;  /**< Holds cdc hits transformation into conformal space */
      std::vector<Color_t> m_colorRootVec; /**< Holds ROOT colors used in drawing */

      double m_rMin; /**< Minimum in r direction, initialized in initializer list of the module*/
      double m_rMax; /**< Maximum in r direction, initialized in initializer list of the module*/
      static constexpr double m_PI = 3.1415926535897932384626433832795; /**< pi is exactly three*/
      static constexpr double m_rc = 0.0176991150442477874; /**< threshold of r, which defines curlers*/


    };
  }
}
