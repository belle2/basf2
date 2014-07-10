/*
 * CDCNiceDrawingModule.cc
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#include "tracking/cdcLegendreTracking/CDCLegendreNiceDrawing.h"
#include <tracking/cdcLegendreTracking/CDCLegendreWireCenter.h>


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

NiceDrawing::NiceDrawing(std::string& TrackCandColName, std::string& trackColName,
                         std::string& HitColName, std::string& StoreDirectory, bool drawMCSignal,
                         bool drawCands, std::string& mcParticlesColName):
  m_TrackCandColName(TrackCandColName), m_trackColName(trackColName), m_HitColName(HitColName), m_StoreDirectory(StoreDirectory),
  m_drawMCSignal(drawMCSignal), m_drawCands(drawCands), m_mcParticlesColName(mcParticlesColName)
{

}

void NiceDrawing::initialize()
{
//  m_zReference = 25.852;

  m_eventCounter = 1;

  m_max = 800;

  m_realMax = 250;

  m_rCDC = 113;

  m_scale = m_max / float(m_realMax);

  initColorVec();

  initWireString();

  if (m_drawCands)
    StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  else
    StoreArray<genfit::Track>::required(m_trackColName);

  StoreArray<CDCHit>::required(m_HitColName);

  if (m_drawMCSignal)
    StoreArray<MCParticle>::required(m_mcParticlesColName);

}

void NiceDrawing::event()
{
  initFig();

  if (m_drawMCSignal)
    drawMCTracks();

  drawWires();

  drawCDCHits();

//  if (m_drawCands)
  drawTrackCands();
//  else
//    drawTracks();

  m_eventCounter += 1;

  finalizeFile();
}

void NiceDrawing::initFig()
{
  std::stringstream ss;
  ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << "_cdc.svg";

  m_fig.open(ss.str().c_str());

  m_fig << "<?xml version=\"1.0\" ?> \n<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"" << m_max << "pt\" height=\"" << m_max << "pt\" viewBox=\"0 0 " << m_max << " " << m_max << "\" version=\"1.1\">\n";
}

void NiceDrawing::drawWires()
{
  m_fig << m_wireString.str();
}

void NiceDrawing::initWireString()
{
  double rWire = 0.00025 * m_max;

  for (unsigned int iLayer = 0; iLayer < CDC::CDCGeometryPar::Instance().nWireLayers(); ++iLayer) {
    for (unsigned int iWire = 0; iWire < CDC::CDCGeometryPar::Instance().nWiresInLayer(iLayer); ++iWire) {
      TVector2 wirePos = getWirePosition(iLayer, iWire);
      drawCircle(m_wireString, wirePos, rWire, "gray");
    }
  }

  drawCircle(m_wireString, TVector2(0, 0) , m_rCDC, "gray", 2);
}

std::string NiceDrawing::getColor(int i)
{
  int iColor = i % m_colorVec.size();

  return m_colorVec[iColor];
}

void NiceDrawing::initColorVec()
{
  m_colorVec.push_back("#0343df"); //blue
  m_colorVec.push_back("#15b01a"); //green
  m_colorVec.push_back("#e50000"); //red
  m_colorVec.push_back("#f97306"); //orange
  m_colorVec.push_back("#6e750e"); //olive
  m_colorVec.push_back("#650021"); //maroon
  m_colorVec.push_back("#01ff07"); //bright green
  m_colorVec.push_back("#0504aa"); //royal blue
  m_colorVec.push_back("#840000"); //dark red
  m_colorVec.push_back("#c20078"); //magenta
  m_colorVec.push_back("#9a0eea"); //violet
  m_colorVec.push_back("#033500"); //dark green
  m_colorVec.push_back("#00ffff"); //cyan
  m_colorVec.push_back("#cb416b"); //dark pink
  m_colorVec.push_back("#01153e"); //navy
  m_colorVec.push_back("#014d4e"); //dark teal
  m_colorVec.push_back("#9aae07"); //puke green
  m_colorVec.push_back("#ff796c"); //salmon
  m_colorVec.push_back("#c0fb2d"); //yellow green
  m_colorVec.push_back("#045c5a"); //dark turquoise
  m_colorVec.push_back("#1e488f"); //cobalt
}

TVector2 NiceDrawing::getWirePosition(int iLayer, int iWire)
{
  TVector3 wireBegin = CDC::CDCGeometryPar::Instance().wireForwardPosition(iLayer, iWire);
  TVector3 wireEnd = CDC::CDCGeometryPar::Instance().wireBackwardPosition(iLayer, iWire);

  m_zReference = WireCenter::Instance().getCenter(iLayer);
//  B2INFO("Z position: " << m_zReference << "; Layer: " << iLayer);

  double fraction;

  fraction = (m_zReference - wireBegin.z()) / (wireBegin.z() - wireEnd.z());
//  B2INFO("Fraction: " << fraction);
  double WireX = (wireBegin.x() + fraction * (wireBegin.x() - wireEnd.x()));
  double WireY = (wireBegin.y() + fraction * (wireBegin.y() - wireEnd.y()));

  return TVector2(WireX, WireY);
}

void NiceDrawing::drawArc(std::stringstream& drawString, TVector2 position, TVector2 center, double radius, int charge, std::string color, double linewidth)
{
  std::pair<TVector2, TVector2> intersect_both = getIntersect(center, position);


  int sweep_flag = 0;
  TVector2 intersect = intersect_both.first;
  if (charge > 0) {
    sweep_flag = 1;
    intersect = intersect_both.second;
  }

  TVector3 origin_new = translateCircle(position, radius);
  TVector3 arc_new = translateCircle(intersect, radius);

  drawString << "<path d=\"M " << origin_new[0] << " " << origin_new[1] << " ";

  drawString << "A " << arc_new[2] << " " << arc_new[2] << " ";

  drawString << "0 0 "  << sweep_flag << " ";

  drawString << arc_new[0] << " " << arc_new[1] << "\" ";

  drawString << "fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << linewidth << "\" />\n";
}

void NiceDrawing::drawCircle(std::stringstream& drawString, TVector2 position, double radius, std::string color, double linewidth)
{
  TVector3 circle_new = translateCircle(position, radius);

  drawString << "<circle cx=\"" << circle_new[0] << "\" cy=\"" << circle_new[1] << "\" r=\"" << circle_new[2] << "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << linewidth << "\"  />\n";
}

TVector3 NiceDrawing::translateCircle(TVector2 center, double radius)
{
  double retX =      center.X() * m_scale + (m_max / 2);
  double retY = -1 * center.Y() * m_scale + (m_max / 2);
  double retR = radius * m_scale;

  return TVector3(retX, retY, retR);
}

void NiceDrawing::finalizeFile()
{
  m_fig << "</svg>\n";
  m_fig.close();
}

void NiceDrawing::drawCDCHits()
{
  StoreArray<CDCHit> HitArray(m_HitColName);

  if (HitArray.getEntries() == 0)
    return;

  std::stringstream ss;

  for (int iHit = 0; iHit < HitArray.getEntries(); ++iHit) {
    CDCHit* cdcHit = HitArray[iHit];

    drawCDCHit(ss, cdcHit, "black");
  }

  m_fig << ss.str();
}

void NiceDrawing::drawCDCHit(std::stringstream& drawString, CDCHit* cdcHit, std::string hitColor)
{
  double driftTime = m_driftTimeTranslator.getDriftLength(
                       cdcHit->getTDCCount(), WireID(cdcHit->getID()));

  int layerID = 0;

  if (cdcHit->getISuperLayer() == 0)
    layerID = cdcHit->getILayer();
  else
    layerID = cdcHit->getILayer() + cdcHit->getISuperLayer() * 6 + 2;

  TVector2 position = getWirePosition(layerID, cdcHit->getIWire());

  drawCircle(drawString, position, driftTime, hitColor);
}

void NiceDrawing::drawTrackCand(std::stringstream& drawString, genfit::TrackCand* TrackCand, std::string trackColor)
{
  TVector2 momentum(TrackCand->getMomSeed().X(), TrackCand->getMomSeed().Y());
  TVector2 position(TrackCand->getPosSeed().X(), TrackCand->getPosSeed().Y());

  int charge = TrackCand->getChargeSeed() * 1.1;

  drawAnyTrack(drawString, momentum, charge, trackColor, position);
}

void NiceDrawing::drawAnyTrack(std::stringstream& drawString, TVector2 momentum, int charge, std::string trackColor, TVector2 position, int linewidth)
{
  double radius = sqrt(momentum.X() * momentum.X() + momentum.Y() * momentum.Y()) / (1.5 * 0.00299792458);

  double alpha = atan2(momentum.Y(), momentum.X());

  double yc = position.Y() + radius * cos(alpha) * -1 * charge;
  double xc = position.X() + radius * sin(alpha) * charge;

  if (radius > 56.5)
    drawArc(drawString, position, TVector2(xc, yc), radius, charge, trackColor, linewidth);
//    drawArc(drawString, position, radius, charge, trackColor, linewidth);
  else
    drawCircle(drawString, TVector2(xc, yc), radius, trackColor, linewidth);
//    drawCircle(drawString, position, radius, trackColor, linewidth);
}

void NiceDrawing::drawTrackCands()
{
  StoreArray<genfit::TrackCand> CandArray(m_TrackCandColName);
  StoreArray<CDCHit> HitArray(m_HitColName);

  if (CandArray.getEntries() == 0)
    return;

  std::stringstream ss;

  for (int iTrack = 0; iTrack < CandArray.getEntries(); ++iTrack) {
    genfit::TrackCand* TrackCandidate = CandArray[iTrack];

    std::string trackColor = getColor(iTrack);

    drawTrackCand(ss, TrackCandidate, trackColor);

    BOOST_FOREACH(int hitID, TrackCandidate->getHitIDs(Const::CDC)) {
      CDCHit* TrackHit = HitArray[hitID];

//      B2INFO("HitID: " << hitID);

      drawCDCHit(ss, TrackHit, trackColor);
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawMCTracks()
{
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);

  std::stringstream ss;

  for (int iPart = 0; iPart < mcParticles.getEntries(); ++iPart) {
    MCParticle* part = mcParticles[iPart];

    if (part->hasStatus(MCParticle::c_StableInGenerator) && fabs(part->getCharge()) > 0.001)
      drawMCTrack(ss, part, "lightblue");
  }

  m_fig << ss.str();
}

void NiceDrawing::drawMCTrack(std::stringstream& drawString, MCParticle* mcPart, std::string trackColor)
{
  TVector2 momentum(mcPart->getMomentum().X(), mcPart->getMomentum().Y());

  int charge = int(mcPart->getCharge());

//  double radius = sqrt(momentum.X() * momentum.X() + momentum.Y() * momentum.Y()) / (1.5 * 0.00299792458);

//  double alpha = atan2(momentum.Y(), momentum.X());

  double yc = mcPart->getVertex().Y();
  double xc = mcPart->getVertex().X();


  TVector2 position(xc, yc);

  drawAnyTrack(drawString, momentum, charge, trackColor, position, 4);
}

/*
void CDCNiceDrawingModule::drawTracks()
{
  StoreArray< genfit::Track > tracks(m_trackColName);
  StoreArray< CDCHit > cdcHits(m_HitColName);

  if (tracks.getEntries() == 0) // tracks
    return;

  std::stringstream ss;

  for (int iTrack = 0; iTrack < tracks.getEntries(); iTrack++) {
    genfit::Track* track = tracks[iTrack];

    std::string color = getColor(iTrack);

    drawTrack(ss, track, color);

    BOOST_FOREACH(int iHit, track->getCand().getHitIDs(Const::CDC)) {
      CDCHit* trackHit = cdcHits[iHit];

      drawCDCHit(ss, trackHit, color);
    }
  }

  m_fig << ss.str();
}


void CDCNiceDrawingModule::drawTrack(std::stringstream& drawString, genfit::Track* track, std::string color)
{
  TVector2 momentum(track->getMom().X(), track->getMom().Y());
  TVector2 position(track->getPos().X(), track->getPos().Y());

  int charge = track->getCharge() * 1.1;

  drawAnyTrack(drawString, momentum, charge, color, position, 2);
}
*/

std::pair<TVector2, TVector2> NiceDrawing::getIntersect(TVector2 center, TVector2 position)
{
  TVector2 A(position.X(), position.Y());
  TVector2 B(center.X(), center.Y());
  double a = sqrt(B.X() * B.X() + B.Y() * B.Y());
  double b = m_rCDC * 1.05;
  if (a < (b / 2))
    b = 2 * a * 0.98;
  double c = a;
  double cosAlpha = (b * b + c * c - a * a) / (2 * b * c);
  TVector2 u_AB = (B - A) * 1 / c;
  TVector2 pu_AB(u_AB.Y(), -1 * u_AB.X());

  TVector2 intersect_1 = A + u_AB * (b * cosAlpha) + pu_AB * (b * sqrt(1 - cosAlpha * cosAlpha));
  TVector2 intersect_2 = A + u_AB * (b * cosAlpha) - pu_AB * (b * sqrt(1 - cosAlpha * cosAlpha));

  return std::make_pair(intersect_1, intersect_2);
}
