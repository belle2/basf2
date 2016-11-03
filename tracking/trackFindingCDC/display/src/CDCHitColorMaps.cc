/*
 * CDCHitColorMaps.cc
 *
 *  Created on: Jun 8, 2015
 *      Author: dschneider
 */
#include <tracking/trackFindingCDC/display/CDCHitColorMaps.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <set>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <sstream>

using namespace Belle2;
using namespace TrackFindingCDC;



std::string ZeroDriftLengthColorMap::map(const int, const CDCHit& cdcHit)
{
  TrackFindingCDC::CDCWireHit wirehit(&cdcHit);
  if (wirehit.getRefDriftLength() == 0) {
    return ("red");
  } else {
    return (m_bkgHitColor);
  }
}

std::string ZeroDriftLengthStrokeWidthMap::map(const int, const CDCHit& cdcHit)
{
  CDCWireHit wirehit(&cdcHit, nullptr);
  if (wirehit.getRefDriftLength() == 0) {
    return ("1");
  } else {
    return ("0.2");
  }
}



TakenFlagColorMap::TakenFlagColorMap()
{
  if (not m_storedWireHits) {
    B2WARNING("CDCWireHitVector could not be found on the DataStore. Cannot plot the taken flags.");
  }
}

std::string TakenFlagColorMap::map(const int, const CDCHit& cdcHit)
{
  if (m_storedWireHits) {
    const std::vector<CDCWireHit>& wireHits = *m_storedWireHits;
    ConstVectorRange<CDCWireHit> wireHitRange =
      std::equal_range(wireHits.begin(), wireHits.end(), cdcHit);

    if (not wireHitRange.empty()) {
      const CDCWireHit& wireHit =  wireHitRange.front();
      if (wireHit.getAutomatonCell().hasTakenFlag()) {
        return "red";
      }
    }
  }
  return m_bkgHitColor;
}

std::string RLColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCMCHitLookUp mcHitLookUp;
  mcHitLookUp.getInstance();
  short int rlInfo = mcHitLookUp.getRLInfo(&cdcHit);
  if (rlInfo == 1) {
    return ("green");
  } else if (rlInfo == -1) {
    return ("red");
  } else {
    return (m_bkgHitColor);
  }
}

std::string RLColorMap::info()
{
  return "Local right left passage variable: green <-> right, red <-> left, orange <-> not determinable.";
}


std::string PosFlagColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCSimHit* simHit = cdcHit.getRelated<CDCSimHit>("CDCSimHits");
  int posFlag = simHit->getPosFlag();
  if (posFlag == 0) {
    return ("green"); // right
  } else if (posFlag == 1) {
    return ("red"); // left
  } else {
    return (m_bkgHitColor);
  }
}

std::string PosFlagColorMap::info()
{
  return "PosFlag variable of the related CDCSimHit: green <-> 0 (Right), red <-> 1 (Left), orange <-> determinable.";
}

std::string BackgroundTagColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCSimHit* cdcSimHit = cdcHit.getRelated<CDCSimHit>("CDCSimHits");
  short backgroundtag = cdcSimHit->getBackgroundTag();
  switch (backgroundtag) {
    case SimHitBase::BG_TAG::bg_none: return "orange";
    case SimHitBase::BG_TAG::bg_Coulomb_LER: return "red";
    case SimHitBase::BG_TAG::bg_Coulomb_HER: return "darkred";
    case SimHitBase::BG_TAG::bg_RBB_LER: return "blue";
    case SimHitBase::BG_TAG::bg_RBB_HER: return "darkblue";
    case SimHitBase::BG_TAG::bg_Touschek_LER: return "green";
    case SimHitBase::BG_TAG::bg_Touschek_HER: return "darkgreen";
    case SimHitBase::BG_TAG::bg_twoPhoton: return "violet";
    case SimHitBase::BG_TAG::bg_RBB_gamma: return "skyblue";
    case SimHitBase::BG_TAG::bg_RBB_LER_far: return "turquoise";
    case SimHitBase::BG_TAG::bg_RBB_HER_far: return "darkturquoise";
    case SimHitBase::BG_TAG::bg_Touschek_LER_far: return "olivergreen";
    case SimHitBase::BG_TAG::bg_Touschek_HER_far: return "darkolivegreen";
    case SimHitBase::BG_TAG::bg_SynchRad_LER: return "goldenrod";
    case SimHitBase::BG_TAG::bg_SynchRad_HER: return "darkgoldenrod";
    case SimHitBase::BG_TAG::bg_other: return "orange";
    default:
      B2INFO("Background tag " << backgroundtag << " not associated with a color.\n");
      return ("orange");
  }
}

std::string BackgroundTagColorMap::info()
{
  return ("Background tag color coding is:\n"
          "bg_Coulomb_HER: darkred\n"
          "bg_Coulomb_LER: red\n"
          "bg_RBB_HER: darkblue\n"
          "bg_RBB_HER_far: darkturquoise\n"
          "bg_RBB_LER: blue\n"
          "bg_RBB_LER_far: turquoise\n"
          "bg_SynchRad_HER: darkgoldenrod\n"
          "bg_SynchRad_LER: goldenrod\n"
          "bg_Touschek_HER: darkgreen\n"
          "bg_Touschek_HER_far: darkolivegreen\n"
          "bg_Touschek_LER: green\n"
          "bg_Touschek_LER_far: olivergreen\n"
          "bg_RBB_gamma: skyblue\n"
          "bg_none: orange\n"
          "bg_other: orange\n"
          "bg_twoPhoton: violet\n"
         );
}

std::string MCSegmentIdColorMap::map(const int, const CDCHit& cdcHit)
{
  TrackFindingCDC::CDCMCHitLookUp mcHitLookUp;
  mcHitLookUp.getInstance();
  TrackFindingCDC::Index inTrackiSegment = mcHitLookUp.getInTrackSegmentId(&cdcHit);

  if (inTrackiSegment < 0) {
    return (m_bkgHitColor);
  } else {
    //values are all fractions of their respective scale
    double hue = 50 * inTrackiSegment % 360 / 360.0;
    double saturation = 0.75, lightness = 0.5;
    std::vector<double> rgb = HLSToRGB::hlsToRgb(hue, lightness, saturation);
    std::ostringstream oss;
    std::string color;
    oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
    color = oss.str();
    return (color);
  }
}

std::string TOFColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCSimHit* simHit = cdcHit.getRelated<CDCSimHit>("CDCSimHits");
  double timeOfFlight = simHit->getFlightTime();
  return (timeOfFlightToColor(timeOfFlight));
}

std::string TOFColorMap::timeOfFlightToColor(const double timeOfFlight)
{
  //values are all fractions of their respective scale

  //Full color circle in 3 nanoseconds
  double hue = fmod(360 / 3.0 * timeOfFlight, 360) / 360;
  double saturation = 0.75, lightness = 0.5;
  std::vector<double> rgb = HLSToRGB::hlsToRgb(hue, lightness, saturation);
  std::ostringstream oss;
  std::string color;
  oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
  color = oss.str();
  B2INFO(color);
  return (color);
}

std::string ReassignedSecondaryMap::map(const int, const CDCHit& cdcHit)
{
  RelationVector<MCParticle> relatedMCParticles = cdcHit.getRelationsWith<MCParticle>("MCParticles");
  if (relatedMCParticles.size() == 0) {
    return m_bkgHitColor;
  } else {
    double mcRelationWeight = relatedMCParticles.weight(0);
    if (mcRelationWeight > 0) {
      return "green";
    } else {
      return "red";
    }
  }
}

std::string MCParticleColorMap::map(const int, const CDCHit& cdcHit)
{
  MCParticle* mcParticle = cdcHit.getRelated<MCParticle>("MCParticles");

  if (mcParticle != nullptr) {
    int mcParticleId = mcParticle->getArrayIndex();
    if (m_newColors.count(mcParticleId) == 1) {
      return m_newColors[mcParticleId];
    } else {
      ++m_iColor;
      std::string color = m_listColors[m_iColor];
      m_newColors.insert(std::pair<int, std::string>(mcParticleId, color));
      return color;
    }
  } else {
    return m_bkgHitColor;
  }
}

std::string MCPDGCodeColorMap::map(const int, const CDCHit& cdcHit)
{
  MCParticle* mcParticle = cdcHit.getRelated<MCParticle>("MCParticles");
  int pdgcode;

  if (mcParticle != nullptr) {
    pdgcode = mcParticle->getPDG();
  } else {
    pdgcode = -999;
  }

  for (std::map<int, std::string>::iterator it = m_colorByPDGCode.begin(); it != m_colorByPDGCode.end(); ++it) {
    if (pdgcode == it->first) return m_colorByPDGCode[pdgcode];
  }

  B2INFO("Unknown PDG code " << pdgcode);
  return m_missingPDGColor;
}

std::string MCPDGCodeColorMap::info()
{
  std::ostringstream oss;
  oss << "\nLegend:";

  std::map<std::string, std::set<int>> pdgCodeByColor;

  for (auto item : m_colorByPDGCode) {
    pdgCodeByColor[item.second].insert(item.first);
  }

  for (auto item : pdgCodeByColor) {
    oss << '\n' << item.first << "->[";

    for (std::set<int>::iterator pdgCode = item.second.begin(); pdgCode != --item.second.end();
         ++pdgCode) {
      oss << *pdgCode << ',';
    }
    oss << *(--item.second.end()) << ']';
  }

  return oss.str();
}

std::string MCPrimaryColorMap::map(const int, const CDCHit& cdcHit)
{
  MCParticle* mcParticle = cdcHit.getRelated<MCParticle>("MCParticles");
  if (mcParticle != nullptr) {
    unsigned short int primaryFlag = 1;
    bool isPrimary = mcParticle->hasStatus(primaryFlag);
    int secondaryProcess = mcParticle->getSecondaryPhysicsProcess();

    if (isPrimary) {
      return "blue";
    } else if (secondaryProcess > 200) {
      return "green"; // decay in flight
    } else {
      return "red";
    }
  } else {
    return m_bkgHitColor;
  }
}

std::string MCPrimaryColorMap::info()
{
  return ("Legend:\n"
          "blue->primary\n"
          "green->secondary decay in flight\n"
          "orange->beam background");
}

std::string SimHitPDGCodeColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCSimHit* simHit = cdcHit.getRelated<CDCSimHit>("CDCSimHits");
  int pdgCode;

  if (simHit != nullptr) {
    pdgCode = simHit->getPDGCode();
  } else {
    pdgCode = -999;
  }

  for (std::map<int, std::string>::iterator it = m_colorByPDGCode.begin(); it != m_colorByPDGCode.end(); ++it) {
    if (pdgCode == it->first) return m_colorByPDGCode[pdgCode];
  }

  B2INFO("Unknown PDG code " << pdgCode);
  return m_missingPDGColor;
}

std::string SimHitIsBkgColorMap::map(const int, const CDCHit& cdcHit)
{
  CDCSimHit* simHit = cdcHit.getRelated<CDCSimHit>("CDCSimHits");
  bool bkgTag = simHit->getBackgroundTag();
  if (bkgTag) {
    return "gray";
  } else {
    return "red";
  }
}
