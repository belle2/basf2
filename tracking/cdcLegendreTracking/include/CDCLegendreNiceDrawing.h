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


  class CDCLegendreNiceDrawing {

  public:

    CDCLegendreNiceDrawing() {};

    CDCLegendreNiceDrawing(std::string& TrackCandColName, std::string& trackColName,
                           std::string& HitColName, std::string& StoreDirectory, bool drawMCSignal,
                           bool m_drawCands, std::string& m_mcParticlesColName);

    void initialize();

    void event();

    void initColorVec();

    void initWireString();

    TVector2 getWirePosition(int iLayer, int iWire);

    void drawCircle(std::stringstream& drawString, TVector2 position, double radius, std::string color, double linewidth = 1);

    void drawArc(std::stringstream& drawString, TVector2 position, TVector2 center, double radius, int charge, std::string color, double linewidth = 1);

    TVector3 translateCircle(TVector2 center, double radius);

    void initFig();

    void finalizeFile();

    void drawWires();

    void drawCDCHits();

    void drawCDCHit(std::stringstream& drawString, CDCHit* TrackHit, std::string hitColor);

    void drawAnyTrack(std::stringstream& drawString, TVector2 momentum, int charge, std::string trackColor, TVector2 position, int linewidth = 2);

    void drawTrackCands();

    void drawTrackCand(std::stringstream& drawString, genfit::TrackCand* TrackCand, std::string hitColor);

    void drawMCTracks();

    void drawMCTrack(std::stringstream& drawString, MCParticle* mcPart, std::string trackColor);

//    void drawTracks();

//    void drawTrack(std::stringstream& drawSting, genfit::Track* track, std::string color);

    std::string getColor(int i);

    std::pair<TVector2, TVector2> getIntersect(TVector2 center, TVector2 position);

  protected:

    std::string m_StoreDirectory;

    std::string m_TrackCandColName;

    std::string m_HitColName;

    std::string m_mcParticlesColName;

    std::string m_trackColName;

    bool m_drawMCSignal;

    double m_zReference;

    int m_eventCounter ;

    double m_max;

    double m_realMax;

    double m_rCDC;

    double m_scale;

    bool m_drawCands;

    std::stringstream m_wireString;

    std::vector<std::string> m_colorVec;

    std::ofstream m_fig;

    CDC::SimpleTDCCountTranslator m_driftTimeTranslator;

  };


}
