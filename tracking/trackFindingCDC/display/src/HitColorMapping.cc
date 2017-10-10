/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/HitColorMapping.h>
#include <tracking/trackFindingCDC/display/Colors.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <set>
#include <sstream>

using namespace Belle2;
using namespace TrackFindingCDC;

/// The default color to be used.
const std::string c_bkgHitColor = "orange";

/// Color for the case a particle a pdg code not mentioned in the colorByPDGCode map.
const std::string c_missingPDGColor = "lime";

/// Map to define the color for the most relevant.
const std::map<int, std::string> c_colorByPDGCode = {
  { -999, "orange"},
  {11, "blue"},
  { -11, "blue"},
  {13, "turquoise"},
  { -13, "turquoise"},
  {15, "cyan"},
  { -15, "cyan"},
  {22, "violet"},
  {211, "green"},
  { -211, "green"},
  {321, "olive"},
  { -321, "olive"},
  {2212, "red"},
  { -2212, "red"},
};

std::string ZeroDriftLengthColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCWireHit wirehit(&hit);
  if (wirehit.getRefDriftLength() == 0) {
    return "red";
  } else {
    return c_bkgHitColor;
  }
}

std::string ZeroDriftLengthStrokeWidthMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCWireHit wirehit(&hit, nullptr);
  if (wirehit.getRefDriftLength() == 0) {
    return "1";
  } else {
    return "0.2";
  }
}

TakenFlagColorMap::TakenFlagColorMap()
{
  if (not m_storedWireHits) {
    B2WARNING("CDCWireHitVector could not be found on the DataStore. Cannot plot the taken flags.");
  }
}

std::string TakenFlagColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  if (m_storedWireHits) {
    const std::vector<CDCWireHit>& wireHits = *m_storedWireHits;
    ConstVectorRange<CDCWireHit> wireHitRange{std::equal_range(wireHits.begin(), wireHits.end(), hit)};

    if (not wireHitRange.empty()) {
      const CDCWireHit& wireHit =  wireHitRange.front();
      if (wireHit.getAutomatonCell().hasTakenFlag()) {
        return "red";
      }
    }
  }
  return c_bkgHitColor;
}

std::string RLColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  const CDCMCHitLookUp& mcHitLookUp = mcHitLookUp.getInstance();
  short int rlInfo = mcHitLookUp.getRLInfo(&hit);
  if (rlInfo == 1) {
    return ("green");
  } else if (rlInfo == -1) {
    return ("red");
  } else {
    return (c_bkgHitColor);
  }
}

std::string RLColorMap::info()
{
  return "Local right left passage variable: green <-> right, red <-> left, orange <-> not determinable.\n";
}

std::string PosFlagColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCSimHit* simHit = hit.getRelated<CDCSimHit>("CDCSimHits");
  int posFlag = simHit->getPosFlag();
  if (posFlag == 0) {
    return ("green"); // right
  } else if (posFlag == 1) {
    return ("red"); // left
  } else {
    return (c_bkgHitColor);
  }
}

std::string PosFlagColorMap::info()
{
  return "PosFlag variable of the related CDCSimHit: green <-> 0 (Right), red <-> 1 (Left), orange <-> determinable.\n";
}

std::string BackgroundTagColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCSimHit* cdcSimHit = hit.getRelated<CDCSimHit>("CDCSimHits");
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
    case SimHitBase::BG_TAG::bg_BHWide_LER: return "cyan";
    case SimHitBase::BG_TAG::bg_BHWide_HER: return "darkcyan";
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

std::string MCSegmentIdColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  TrackFindingCDC::CDCMCHitLookUp mcHitLookUp;
  mcHitLookUp.getInstance();
  TrackFindingCDC::Index inTrackiSegment = mcHitLookUp.getInTrackSegmentId(&hit);

  if (inTrackiSegment < 0) {
    return (c_bkgHitColor);
  } else {
    // values are all fractions of their respective scale
    double hue = 50 * inTrackiSegment % 360 / 360.0;
    double saturation = 0.75, lightness = 0.5;
    std::array<double, 3> rgb = Colors::hlsToRgb(hue, lightness, saturation);
    std::ostringstream oss;
    std::string color;
    oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
    color = oss.str();
    return color;
  }
}

std::string TOFColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCSimHit* simHit = hit.getRelated<CDCSimHit>("CDCSimHits");
  double timeOfFlight = simHit->getFlightTime();
  return timeOfFlightToColor(timeOfFlight);
}

std::string TOFColorMap::timeOfFlightToColor(const double timeOfFlight)
{
  //values are all fractions of their respective scale

  //Full color circle in 3 nanoseconds
  double hue = fmod(360 / 3.0 * timeOfFlight, 360) / 360;
  double saturation = 0.75, lightness = 0.5;
  std::array<double, 3> rgb = Colors::hlsToRgb(hue, lightness, saturation);
  std::ostringstream oss;
  std::string color;
  oss << "rgb(" << rgb[0] * 100 << "%, " << rgb[1] * 100 << "%, " << rgb[2] * 100 << "%)";
  color = oss.str();
  return color;
}

std::string ReassignedSecondaryMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  RelationVector<MCParticle> relatedMCParticles =
    hit.getRelationsWith<MCParticle>("MCParticles");
  if (relatedMCParticles.size() == 0) {
    return c_bkgHitColor;
  } else {
    double mcRelationWeight = relatedMCParticles.weight(0);
    if (mcRelationWeight > 0) {
      return "green";
    } else {
      return "red";
    }
  }
}

MCParticleColorMap::MCParticleColorMap()
  : m_colors(Colors::getList())
  , m_iColor(0)
  , m_usedColors()
{
}

std::string MCParticleColorMap::info()
{
  std::ostringstream oss;
  for (const std::pair<int, std::string>& colorForMCParticleID : m_usedColors) {
    oss << "MCParticle " << colorForMCParticleID.first << " -> " << colorForMCParticleID.second << "\n";
  }
  return oss.str();
}

std::string MCParticleColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  MCParticle* mcParticle = hit.getRelated<MCParticle>("MCParticles");
  if (mcParticle != nullptr) {
    int mcParticleId = mcParticle->getArrayIndex();
    if (m_usedColors.count(mcParticleId) == 1) {
      return m_usedColors[mcParticleId];
    } else {
      ++m_iColor;
      std::string color = m_colors[m_iColor % m_colors.size()];
      m_usedColors.emplace(mcParticleId, color);
      return color;
    }
  } else {
    return c_bkgHitColor;
  }
}

std::string MCPDGCodeColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  MCParticle* mcParticle = hit.getRelated<MCParticle>("MCParticles");
  int pdgCode = mcParticle != nullptr ? mcParticle->getPDG() : -999;

  auto itFound = c_colorByPDGCode.find(pdgCode);
  if (itFound != c_colorByPDGCode.end()) {
    return itFound->second;
  } else {
    B2WARNING("Unknown PDG code " << pdgCode);
    return c_missingPDGColor;
  }
}

std::string MCPDGCodeColorMap::info()
{
  std::ostringstream oss;
  oss << "\nLegend:";

  std::map<std::string, std::set<int>> pdgCodeByColor;

  for (auto item : c_colorByPDGCode) {
    pdgCodeByColor[item.second].insert(item.first);
  }

  for (auto item : pdgCodeByColor) {
    oss << '\n' << item.first << "->[";
    oss << join(", ", item.second);
    oss << ']';
  }
  oss << '\n';
  return oss.str();
}

std::string MCPrimaryColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  MCParticle* mcParticle = hit.getRelated<MCParticle>("MCParticles");
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
    return c_bkgHitColor;
  }
}

std::string MCPrimaryColorMap::info()
{
  return "Legend:\n"
         "blue->primary\n"
         "green->secondary decay in flight\n"
         "orange->beam background\n";
}

std::string SimHitPDGCodeColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCSimHit* simHit = hit.getRelated<CDCSimHit>("CDCSimHits");
  int pdgCode = simHit != nullptr ? simHit->getPDGCode() : -999;
  auto itFound = c_colorByPDGCode.find(pdgCode);
  if (itFound != c_colorByPDGCode.end()) {
    return itFound->second;
  } else {
    B2WARNING("Unknown PDG code " << pdgCode);
    return c_missingPDGColor;
  }
}

std::string SimHitPDGCodeColorMap::info()
{
  std::ostringstream oss;
  oss << "\nLegend:";

  std::map<std::string, std::set<int>> pdgCodeByColor;

  for (auto item : c_colorByPDGCode) {
    pdgCodeByColor[item.second].insert(item.first);
  }

  for (auto item : pdgCodeByColor) {
    oss << '\n' << item.first << "->[";
    oss << join(", ", item.second);
    oss << ']';
  }
  oss << '\n';
  return oss.str();
}

std::string SimHitIsBkgColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  CDCSimHit* simHit = hit.getRelated<CDCSimHit>("CDCSimHits");
  bool bkgTag = simHit->getBackgroundTag();
  if (bkgTag) {
    return "gray";
  } else {
    return "red";
  }
}

std::string NLoopsColorMap::map(int index __attribute__((unused)), const CDCHit& hit)
{
  const CDCMCHitLookUp& mcHitLookUp = mcHitLookUp.getInstance();

  int nLoops = mcHitLookUp.getNLoops(&hit);

  if (nLoops < 0) {
    return c_bkgHitColor;
  }
  return Colors::getWheelColor(70 * nLoops);
}
