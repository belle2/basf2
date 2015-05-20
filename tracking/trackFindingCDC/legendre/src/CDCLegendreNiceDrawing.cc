/*
 * CDCNiceDrawingModule.cc
 *
 *  Created on: May 31, 2013
 *      Author: kronenbitter
 */

#include <tracking/trackFindingCDC/legendre/CDCLegendreNiceDrawing.h>


#include <framework/gearbox/Const.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <cmath>

#include <boost/foreach.hpp>

#include <cdc/geometry/CDCGeometryPar.h>
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>

#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

NiceDrawing::NiceDrawing(std::string& TrackCandColName, std::string& trackColName,
                         std::string& HitColName, std::string& StoreDirectory, bool drawMCSignal,
                         bool drawCands, std::string& mcParticlesColName):
  m_TrackCandColName(TrackCandColName), m_mcTrackCandColName(std::string("")), m_trackColName(trackColName), m_HitColName(HitColName),
  m_StoreDirectory(StoreDirectory),
  m_drawMCSignal(drawMCSignal), m_drawCands(drawCands), m_mcParticlesColName(mcParticlesColName)
{

}

NiceDrawing::NiceDrawing(std::string& TrackCandColName, std::string& mcTrackCandColName, std::string& trackColName,
                         std::string& HitColName, std::string& StoreDirectory, bool drawMCSignal,
                         bool drawCands, std::string& mcParticlesColName):
  m_TrackCandColName(TrackCandColName), m_mcTrackCandColName(mcTrackCandColName), m_trackColName(trackColName),
  m_HitColName(HitColName), m_StoreDirectory(StoreDirectory),
  m_drawMCSignal(drawMCSignal), m_drawCands(drawCands), m_mcParticlesColName(mcParticlesColName)
{

}

void NiceDrawing::initialize()
{
  m_eventCounter = 1;

  m_max = 800;

  m_realMax = 250;

  m_rCDC = 113;

  m_scale = m_max / float(m_realMax);

  initColorVec();

  initWireString();

  StoreArray<genfit::TrackCand>::required(m_TrackCandColName);
  StoreArray<CDCHit>::required(m_HitColName);

  if (m_drawMCSignal)
    StoreArray<MCParticle>::required(m_mcParticlesColName);

  if (m_mcTrackCandColName != std::string("")) {
    StoreArray<genfit::TrackCand>::required(m_mcTrackCandColName);
    StoreArray<MCParticle>::required(m_mcParticlesColName);
  }

}

void NiceDrawing::event(bool drawAlsoDifference)
{
  if (drawAlsoDifference) {
    drawOneFileForEvent(DrawStatus::draw_tracks);
    drawOneFileForEvent(DrawStatus::draw_mc_tracks);
    drawOneFileForEvent(DrawStatus::draw_track_candidates);
    drawOneFileForEvent(DrawStatus::draw_not_reconstructed);
    drawOneFileForEvent(DrawStatus::draw_not_reconstructed_hits);
  } else {
    drawOneFileForEvent(DrawStatus::draw_tracks);
  }
  m_eventCounter += 1;
}

void NiceDrawing::drawOneFileForEvent(DrawStatus drawStatus)
{
  initFig(drawStatus);
  drawWires();
  if (drawStatus != DrawStatus::draw_not_reconstructed_hits) {
    drawCDCHits();
  }
  if (m_drawMCSignal)
    drawMCTracks();

  if (drawStatus == DrawStatus::draw_tracks) {
    drawTrackCands(m_TrackCandColName);
  } else if (drawStatus == DrawStatus::draw_not_reconstructed) {
    drawMCTracksMinusTrackCandidates();
  } else if (drawStatus == DrawStatus::draw_track_candidates) {
    drawTrackCandidatesMinusMCTracks();
  } else if (drawStatus == DrawStatus::draw_mc_tracks) {
    drawTrackCands(m_mcTrackCandColName);
  } else if (drawStatus == DrawStatus::draw_not_reconstructed_hits) {
    drawNotReconstructedCDCHits();
  }

  finalizeFile();
}

void NiceDrawing::initFig(DrawStatus drawStatus)
{

  std::string suffix("_");

  if (drawStatus == DrawStatus::draw_not_reconstructed) {
    suffix = std::string("_NotReconstructed_");
  } else if (drawStatus == DrawStatus::draw_track_candidates) {
    suffix = std::string("_FakeTracks_");
  } else if (drawStatus == DrawStatus::draw_mc_tracks) {
    suffix = std::string("_MC_");
  } else if (drawStatus == DrawStatus::draw_not_reconstructed_hits) {
    suffix = std::string("_NotReconstructedHits_");
  }

  std::stringstream ss;

  if (drawStatus == DrawStatus::draw_mc_tracks) {
    StoreArray<genfit::TrackCand> mcTrackCandidates(m_mcTrackCandColName);

    ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << suffix << "cdc.svg";
  } else {
    StoreArray<genfit::TrackCand> trackCandidates(m_TrackCandColName);

    ss << m_StoreDirectory << std::setfill('0') << std::setw(4) << m_eventCounter << suffix << "cdc.svg";
  }

  m_fig.open(ss.str().c_str());

  m_fig << "<?xml version=\"1.0\" ?> \n<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\""
        << m_max << "pt\" height=\"" << m_max << "pt\" viewBox=\"0 0 " << m_max << " " << m_max << "\" version=\"1.1\">\n";
  m_fig << "<text x=\"0\" y=\"10\" fill=\"black\">" << suffix << "</text>\n";

  if (drawStatus != DrawStatus::draw_tracks) {
    m_fig << "<text x=\"0\" y=\"20\" fill=\"blue\">The purity is too low</text>\n";
    m_fig << "<text x=\"0\" y=\"30\" fill=\"green\">No other track has relates hits</text>\n";
    m_fig << "<text x=\"0\" y=\"40\" fill=\"red\">The track related to this has in turn another 'better' relation</text>\n";
    m_fig << "<text x=\"0\" y=\"50\" fill=\"orange\">Another reason</text>\n";
  }
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
      if (WireID(iLayer, iWire).getISuperLayer() % 2 == 0)
        drawCircle(m_wireString, wirePos, rWire, "gray", 1., 1.);
      else
        drawCircle(m_wireString, wirePos, rWire, "black", 1., 1.);
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

  m_zReference = 0.;
  double fraction;

  fraction = (m_zReference - wireBegin.z()) / (wireBegin.z() - wireEnd.z());
  double WireX = (wireBegin.x() + fraction * (wireBegin.x() - wireEnd.x()));
  double WireY = (wireBegin.y() + fraction * (wireBegin.y() - wireEnd.y()));

  return TVector2(WireX, WireY);
}

void NiceDrawing::drawArc(std::stringstream& drawString, TVector2 position, TVector2 center, double radius, int charge,
                          std::string color, double linewidth, double opacity)
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

  drawString << "fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << linewidth << "\" stroke-opacity=\"" << opacity <<
             "\" />\n";
}

void NiceDrawing::drawCircle(std::stringstream& drawString, TVector2 position, double radius, std::string color, double linewidth,
                             double opacity)
{
  TVector3 circle_new = translateCircle(position, radius);

  drawString << "<circle cx=\"" << circle_new[0] << "\" cy=\"" << circle_new[1] << "\" r=\"" << circle_new[2] <<
             "\" fill=\"none\" stroke=\"" << color << "\" stroke-width=\"" << linewidth << "\" stroke-opacity=\"" << opacity << "\" />\n";
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


void NiceDrawing::drawCDCHit(std::stringstream& drawString, const CDCHit* cdcHit, std::string hitColor)
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

void NiceDrawing::drawNotReconstructedCDCHits()
{
  StoreArray<CDCHit> HitArray(m_HitColName);
  StoreArray<genfit::TrackCand> mcTrackCands(m_mcTrackCandColName);
  StoreArray<genfit::TrackCand> trackCands(m_TrackCandColName);

  if (HitArray.getEntries() == 0)
    return;


  std::stringstream ss;


  for (int iMCTrack = 0; iMCTrack < mcTrackCands.getEntries(); ++iMCTrack) {
    genfit::TrackCand* mcTrackCandidate = mcTrackCands[iMCTrack];

    std::string trackColor = getColor(iMCTrack);

    for (int mcHitID : mcTrackCandidate->getHitIDs(Const::CDC)) {
      CDCHit* mcTrackHit = HitArray[mcHitID];

      bool foundInAPTTrackCandidate = false;

      for (int iTrack = 0; iTrack < trackCands.getEntries(); ++iTrack) {
        genfit::TrackCand* trackCandidate = trackCands[iTrack];
        std::vector<int> hitIDs = trackCandidate->getHitIDs(Const::CDC);
        if (std::find(hitIDs.begin(), hitIDs.end(), mcHitID) != hitIDs.end()) {
          foundInAPTTrackCandidate = true;
          break;
        }
      }

      if (!foundInAPTTrackCandidate)
        drawCDCHit(ss, mcTrackHit, trackColor);
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawTrackCand(std::stringstream& drawString, const genfit::TrackCand* TrackCand, std::string trackColor)
{
  TVector2 momentum(TrackCand->getMomSeed().X(), TrackCand->getMomSeed().Y());
  TVector2 position(TrackCand->getPosSeed().X(), TrackCand->getPosSeed().Y());

  int charge = TrackCand->getChargeSeed() * 1.1;

  drawAnyTrack(drawString, momentum, charge, trackColor, position);
}

void NiceDrawing::drawAnyTrack(std::stringstream& drawString, TVector2 momentum, int charge, std::string trackColor,
                               TVector2 position, int linewidth)
{
  double radius = sqrt(momentum.X() * momentum.X() + momentum.Y() * momentum.Y()) / (1.5 * 0.00299792458);

  double alpha = atan2(momentum.Y(), momentum.X());

  double yc = position.Y() + radius * cos(alpha) * -1 * charge;
  double xc = position.X() + radius * sin(alpha) * charge;

  if (radius > 56.5)
    drawArc(drawString, position, TVector2(xc, yc), radius, charge, trackColor, linewidth);
  else
    drawCircle(drawString, TVector2(xc, yc), radius, trackColor, linewidth);
}

void NiceDrawing::drawTrackCands(std::string& trackCandColName)
{
  StoreArray<genfit::TrackCand> CandArray(trackCandColName);
  StoreArray<CDCHit> HitArray(m_HitColName);

  if (CandArray.getEntries() == 0)
    return;

  std::stringstream ss;

  for (int iTrack = 0; iTrack < CandArray.getEntries(); ++iTrack) {
    genfit::TrackCand* trackCandidate = CandArray[iTrack];

    std::string trackColor = getColor(iTrack);

    drawTrackCand(ss, trackCandidate, trackColor);

    BOOST_FOREACH(int hitID, trackCandidate->getHitIDs(Const::CDC)) {
      CDCHit* TrackHit = HitArray[hitID];

      drawCDCHit(ss, TrackHit, trackColor);
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawMCTracksMinusTrackCandidates()
{
  StoreArray<genfit::TrackCand> mcParticleTrackCandidates(m_mcTrackCandColName);
  StoreArray<CDCHit> HitArray(m_HitColName);

  std::stringstream ss;

  for (int iTrack = 0; iTrack < mcParticleTrackCandidates.getEntries(); ++iTrack) {
    genfit::TrackCand* mcTrackCandidate = mcParticleTrackCandidates[iTrack];

    std::string trackColor = getColor(iTrack);

    int ptMatchingStatus = mcTrackCandidate->getMcTrackId();

    if (ptMatchingStatus >= 0) {
      // A good pt track candidate is related to this pattern track candidate
      continue;
    } else if (ptMatchingStatus == -99) {
      // BACKGROUND = no (or no good) pt track related to the hits, green
      trackColor = getColor(1);
    } else if (ptMatchingStatus == -9) {
      // CLONE = the pt track related to this is in turn better relates to another mc track candidate, red
      trackColor = getColor(2);
    } else {
      B2INFO("Pt Matching Status " << ptMatchingStatus);
      trackColor = getColor(3);
    }

    drawTrackCand(ss, mcTrackCandidate, trackColor);
    BOOST_FOREACH(int hitID, mcTrackCandidate->getHitIDs(Const::CDC)) {
      CDCHit* TrackHit = HitArray[hitID];

      drawCDCHit(ss, TrackHit, trackColor);
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawTrackCandidatesMinusMCTracks()
{
  // FakeTracks
  StoreArray<genfit::TrackCand> particleTrackCandidates(m_TrackCandColName);
  StoreArray<CDCHit> HitArray(m_HitColName);

  std::stringstream ss;

  for (int iTrack = 0; iTrack < particleTrackCandidates.getEntries(); ++iTrack) {
    genfit::TrackCand* trackCandidate = particleTrackCandidates[iTrack];

    int mcMatchingStatus = trackCandidate->getMcTrackId();

    std::string trackColor = getColor(iTrack);
    //std::string qualityEstimation = getQualityEstimationOfPTTrackCandidate(trackCandidate);

    //drawDescribingQualityText(ss, qualityEstimation, trackColor, yPosition);

    if (mcMatchingStatus >= 0) {
      // A good mc track candidate is related to this pattern track candidate
      continue;
    } else if (mcMatchingStatus == -999) {
      // GHOST = purity too low, blue
      trackColor = getColor(0);
    } else if (mcMatchingStatus == -99) {
      // BACKGROUND = no mc track related to the hits, green
      trackColor = getColor(1);
    } else if (mcMatchingStatus == -9) {
      // CLONE = the mc track related to this is in turn better related to another pattern track candidate, red
      trackColor = getColor(2);
    } else {
      B2INFO("Mc Matching Status " << mcMatchingStatus);
      trackColor = getColor(3);
    }

    drawTrackCand(ss, trackCandidate, trackColor);

    for (int hitID : trackCandidate->getHitIDs(Const::CDC)) {
      CDCHit* trackHit = HitArray[hitID];

      RelationVector<MCParticle> relationsToMCParticles = trackHit->getRelationsWith<MCParticle>(m_mcParticlesColName);

      // This CDC Hits is not related to a MCParticle.
      if (relationsToMCParticles.size() == 0)
        drawCDCHit(ss, trackHit, "lightblue");
      else
        drawCDCHit(ss, trackHit, trackColor);
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawMCTracks()
{
  StoreArray<MCParticle> mcParticles(m_mcParticlesColName);
  StoreArray<CDCHit> cdcHits(m_HitColName);


  std::stringstream ss;

  for (int iPart = 0; iPart < mcParticles.getEntries(); ++iPart) {
    MCParticle* part = mcParticles[iPart];

    if (part->hasStatus(MCParticle::c_StableInGenerator) && fabs(part->getCharge()) > 0.001) {
      drawMCTrack(ss, part, "lightblue");

      for (const CDCHit& hit :  part->getRelationsTo<CDCHit>()) {
        drawCDCHit(ss, &hit, "lightsteelblue");
      }
    }
  }

  m_fig << ss.str();
}

void NiceDrawing::drawMCTrack(std::stringstream& drawString, MCParticle* mcPart, std::string trackColor)
{
  TVector2 momentum(mcPart->getMomentum().X(), mcPart->getMomentum().Y());

  int charge = int(mcPart->getCharge());

  double vertex_position_y = mcPart->getVertex().Y();
  double vertex_position_x = mcPart->getVertex().X();

  TVector2 position(vertex_position_x, vertex_position_y);

  drawAnyTrack(drawString, momentum, charge, trackColor, position, 4);
}

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

std::string NiceDrawing::getQualityEstimationOfPTTrackCandidate(const genfit::TrackCand* trackCand)
{
  std::stringstream ss;
  TVector3 positionVector = trackCand->getPosSeed();
  TVector3 momentumVector = trackCand->getMomSeed();

  double radiusOfMostInnerCDCWire = getWirePosition(0, 0).Mod();

  if (positionVector.Pt() > radiusOfMostInnerCDCWire / 2.0) {
    ss << " away_from_center ";
  }

  ss << momentumVector.Pt() ;

  return ss.str();
}

void NiceDrawing::drawDescribingQualityText(std::stringstream& drawString, std::string& qualityEstimation, std::string& trackColor,
                                            double yPosition)
{
  drawString << "<text x=\"0\" y=\"" << yPosition << "\" fill=\"" << trackColor << "\"> Track Quality " << qualityEstimation <<
             "</text>\n";
}
