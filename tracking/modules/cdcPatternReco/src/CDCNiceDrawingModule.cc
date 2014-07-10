/*
 * CDCNiceDrawingModule.cc
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#include "tracking/modules/cdcPatternReco/CDCNiceDrawingModule.h"

#include "tracking/cdcLegendreTracking/CDCLegendreNiceDrawing.h"

#include "framework/datastore/StoreArray.h"
#include "cdc/dataobjects/CDCHit.h"
#include "mdst/dataobjects/MCParticle.h"
#include "framework/gearbox/Const.h"
#include "genfit/Track.h"
#include "genfit/TrackCand.h"
#include <boost/foreach.hpp>

#include "TVector2.h"

#include <cmath>
#include <iomanip>

using namespace Belle2;
using namespace CDC;
using namespace TrackFinderCDCLegendre;

REG_MODULE(CDCNiceDrawing)

CDCNiceDrawingModule::CDCNiceDrawingModule() : Module()
{

  setDescription("Nice Tracking Drawing Module");

  addParam("TrackCandColName", m_TrackCandColName, "Genfit Track candidate collection",
           std::string("TrackCands"));

  addParam("trackColName", m_trackColName, "Track collection",
           std::string("Tracks"));

  addParam("HitColName", m_HitColName, "CDCHit collection", std::string("CDCHits"));

  addParam("StoreDirectory", m_StoreDirectory,
           std::string("Directory where pictures are stored"));

  addParam("DrawMCSignal", m_drawMCSignal,
           "Controls, whether MC signal is draw", false);

  addParam("DrawCands", m_drawCands,
           "Controls, whether Track Candidates or Tracks are drawn", false);

  addParam("MCParticlesColName", m_mcParticlesColName,
           "MCParticles collection name", std::string("MCParticles"));

}

void CDCNiceDrawingModule::initialize()
{
  m_cdcLegendreNiceDrawing = new NiceDrawing(m_TrackCandColName, m_trackColName, m_HitColName, m_StoreDirectory, m_drawMCSignal, m_drawCands, m_mcParticlesColName);

  m_cdcLegendreNiceDrawing->initialize();

}

void CDCNiceDrawingModule::event()
{

  m_cdcLegendreNiceDrawing->event();

}

CDCNiceDrawingModule::~CDCNiceDrawingModule()
{
}
