/*
 * CDCNiceDrawingModule.h
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#pragma once

#include "cdc/geometry/CDCGeometryPar.h"
#include "cdc/translators/SimpleTDCCountTranslator.h"

#include <fstream>
#include "genfit/Track.h"
#include "genfit/TrackCand.h"

class TVector2;
//class TrackCand;
//class Track;

namespace Belle2 {
  class CDCHit;
  class MCParticle;

  namespace TrackFinderCDCLegendre {

    /** Class for perform svg drawing of track candidates */
    class NiceDrawing {

    public:

      NiceDrawing()
//  :   m_TrackCandColName("TrackCands"), m_trackColName("Tracks"), m_HitColName("CDCHits"), m_StoreDirectory("tmp/"),
//      m_drawMCSignal(true), m_drawCands(true), m_mcParticlesColName("MCParticles"), m_zReference(0), m_rCDC(113), m_scale(0),
//      m_realMax(0), m_max(0), m_eventCounter(0)
      {};

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
      NiceDrawing(std::string& TrackCandColName, std::string& trackColName,
                  std::string& HitColName, std::string& StoreDirectory, bool drawMCSignal,
                  bool drawCands, std::string& mcParticlesColName);

      /**
       * Prepare variables and CDC stucture
       */
      void initialize();

      /**
       * Process event
       */
      void event();

      /**
       * Initialization of colors
       */
      void initColorVec();

      /**
       * Initialize wires
       */
      void initWireString();

      /**
       * Get position of wire
       */
      TVector2 getWirePosition(int iLayer, int iWire);

      /**
       * Circle drawing
       */
      void drawCircle(std::stringstream& drawString, TVector2 position, double radius, std::string color, double linewidth = 1);

      /**
       * Arc drawing
       */
      void drawArc(std::stringstream& drawString, TVector2 position, TVector2 center, double radius, int charge, std::string color, double linewidth = 1);

      /**
       * Translate coordinates from normal space to svg space
       */
      TVector3 translateCircle(TVector2 center, double radius);

      /**
       * Initialize svg file
       */
      void initFig();

      /**
       * Finalize svg file
       */
      void finalizeFile();

      /**
       * Draw wires in svg file
       */
      void drawWires();

      /**
       * Draw hits in svg file
       */
      void drawCDCHits();

      /**
       * Draw given hit
       */
      void drawCDCHit(std::stringstream& drawString, CDCHit* TrackHit, std::string hitColor);

      /**
       * Track drawing function
       */
      void drawAnyTrack(std::stringstream& drawString, TVector2 momentum, int charge, std::string trackColor, TVector2 position, int linewidth = 2);

      /**
       * Taking information about tracks and draw them
       */
      void drawTrackCands();

      /**
       * Draws track candidate
       */
      void drawTrackCand(std::stringstream& drawString, genfit::TrackCand* TrackCand, std::string hitColor);

      /**
       * Taking information about MC tracks and draw them
       */
      void drawMCTracks();

      /**
       * Draws MC track
       */
      void drawMCTrack(std::stringstream& drawString, MCParticle* mcPart, std::string trackColor);

//    void drawTracks();

//    void drawTrack(std::stringstream& drawSting, genfit::Track* track, std::string color);

      /**
       * Get color by index
       */
      std::string getColor(int i);

      /**
       * Returns position of track end
       */
      std::pair<TVector2, TVector2> getIntersect(TVector2 center, TVector2 position);

    protected:

      std::string m_TrackCandColName; /**< Name of track candidates collection in StoreArray */

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

    };
  }

}
