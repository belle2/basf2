/*
 * CDCNiceDrawingModule.h
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#pragma once


#include "cdc/translators/SimpleTDCCountTranslator.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "mdst/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"

#include "genfit/Track.h"
#include "genfit/TrackCand.h"
#include "framework/datastore/StoreArray.h"
#include "framework/gearbox/Const.h"

#include <boost/foreach.hpp>
#include "TVector2.h"

#include <fstream>
#include <iomanip>
#include <cmath>

namespace Belle2 {

  namespace TrackFindingCDC {

    /** Class for perform svg drawing of track candidates */
    class NiceDrawing {

    public:

      enum DrawStatus {
        draw_tracks = 0,
        draw_not_reconstructed = 1,
        draw_track_candidates = 2,
        draw_mc_tracks = 3,
        draw_not_reconstructed_hits = 4
      };

      NiceDrawing() {};

      /**
       * @brief Constructor
       * @param TrackCandColName Name of track candidates in StoreArray
       * @param trackColName Name of tracks in StoreArray
       * @param HitColName Name of CDCHits in StoreArray
       * @param StoreDirectory Directory to store pictures
       * @param drawMCSignal Sets whether MC particles should be drawn
       * @param drawCands Sets whether track candidates should be drawn
       * @param mcParticlesColName Name of MC particles in StoreArray
       */
      NiceDrawing(std::string& TrackCandColName,
                  std::string& trackColName,
                  std::string& HitColName,
                  std::string& StoreDirectory,
                  bool drawMCSignal,
                  bool drawCands,
                  std::string& mcParticlesColName);

      /**
       * @brief Constructor
       * @param TrackCandColName Name of track candidates in StoreArray
       * @param mcTrackCandColName Name of MCTrack candidates in StoreArray
       * @param trackColName Name of tracks in StoreArray
       * @param HitColName Name of CDCHits in StoreArray
       * @param StoreDirectory Directory to store pictures
       * @param drawMCSignal Sets whether MC particles should be drawn
       * @param drawCands Sets whether track candidates should be drawn
       * @param mcParticlesColName Name of MC particles in StoreArray
       */
      NiceDrawing(std::string& TrackCandColName,
                  std::string& mcTrackCandColName,
                  std::string& trackColName,
                  std::string& HitColName,
                  std::string& StoreDirectory,
                  bool drawMCSignal,
                  bool drawCands,
                  std::string& mcParticlesColName);

      /** Prepare variables and CDC structure */
      void initialize();

      /** Process event */
      void event(bool drawAlsoDifference = false);

      /** Process Event by drawing only one file (Both MC and Candidates, only Candidates minus MC or only MC minus Candidates) */
      void drawOneFileForEvent(DrawStatus drawStatus);

      /** Initialization of colors */
      void initColorVec();

      /** Initialize wires */
      void initWireString();

      /** Get position of wire */
      TVector2 getWirePosition(int iLayer, int iWire);

      /** Circle drawing */
      void drawCircle(std::stringstream& drawString,
                      TVector2 position,
                      double radius,
                      std::string color,
                      double linewidth = 1,
                      double opacity = 0.75);

      /** Arc drawing */
      void drawArc(std::stringstream& drawString,
                   TVector2 position,
                   TVector2 center,
                   double radius,
                   int charge,
                   std::string color,
                   double linewidth = 1,
                   double opacity = 0.75);

      /** Translate coordinates from normal space to svg space */
      TVector3 translateCircle(TVector2 center, double radius);

      /** Initialize svg file */
      void initFig(DrawStatus drawStatus = DrawStatus::draw_tracks);

      /** Finalize svg file */
      void finalizeFile();

      /** Draw wires in svg file */
      void drawWires();

      /** Draw hits in svg file */
      void drawCDCHits();

      /** Draw hits in a svg file, that are related to a MCTrackCandidate but not to a PTTrackCandidate */
      void drawNotReconstructedCDCHits();

      /** Draw given hit */
      void drawCDCHit(std::stringstream& drawString,
                      const CDCHit* TrackHit,
                      std::string hitColor);

      /** Track drawing function */
      void drawAnyTrack(std::stringstream& drawString,
                        TVector2 momentum,
                        int charge,
                        std::string trackColor,
                        TVector2 position,
                        int linewidth = 2);

      /**Taking information about tracks from trackCandColName and draw them */
      void drawTrackCands(std::string& trackCandColName);

      /**Taking information about tracks from pattern track candidates and draw them */
      void drawTrackCands() { drawTrackCands(m_TrackCandColName); }

      /** Draws track candidate */
      void drawTrackCand(std::stringstream& drawString,
                         const genfit::TrackCand* TrackCand,
                         std::string hitColor);

      /** Taking information about MC tracks and draw them */
      void drawMCTracks();

      /** Draws MC track*/
      void drawMCTrack(std::stringstream& drawString,
                       MCParticle* mcPart,
                       std::string trackColor);

      /** Draw only those TrackCandidates that do not have a related MCTrack = Fake Tracks */
      void drawTrackCandidatesMinusMCTracks();

      /** Draw only those MCTracks that do not have a related TrackCandidate = Not Recontructed Tracks */
      void drawMCTracksMinusTrackCandidates();

      /** Get color by index */
      std::string getColor(int i);

      /** Returns position of track end */
      std::pair<TVector2, TVector2> getIntersect(TVector2 center, TVector2 position);

    protected:

      std::string m_TrackCandColName; /**< Name of track candidates collection in StoreArray */

      std::string m_mcTrackCandColName; /**< Name of MCTrack candidates collection in StoreArray */

      std::string m_trackColName; /**< Name of tracks collection in StoreArray */

      std::string m_HitColName; /**< Name of hits collection in StoreArray */

      std::string m_StoreDirectory; /**< Directory to store drawings */

      bool m_drawMCSignal; /**< Sets whether MC particles will be drawn */

      bool m_drawCands; /**< Sets whether "genfit" tracks will be drawn */

      std::string m_mcParticlesColName; /**< Name of MC particles collection in StoreArray */

      double m_zReference; /**< Z-reference point */

      int m_eventCounter ; /**< Event counter */

      double m_max;

      double m_realMax;

      double m_rCDC; /**< Radius of CDC */

      double m_scale;

      std::stringstream m_wireString; /**< String for wire drawing */

      std::vector<std::string> m_colorVec; /**< Vector which holds colors used in drawing */

      std::ofstream m_fig; /**< Output file */

      CDC::SimpleTDCCountTranslator m_driftTimeTranslator; /**< Drift time translator */

    private:
      /** Estimate the track quality based on hitpattern and so on. Just for testing purposes */
      std::string getQualityEstimationOfPTTrackCandidate(const genfit::TrackCand* TrackCand);

      /** Draw the quality estimation as a describing text in the same color as the track. */
      void drawDescribingQualityText(std::stringstream& drawString, std::string& qualityEstimation, std::string& trackColor, double yPosition);

    };
  }

}
