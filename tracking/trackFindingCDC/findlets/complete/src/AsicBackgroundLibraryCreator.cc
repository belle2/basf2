/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/complete/AsicBackgroundLibraryCreator.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <cdc/dataobjects/CDCHit.h>
#include <framework/logging/Logger.h>
#include <iostream>
#include <TTree.h>
#include <TFile.h>
#include <TBranch.h>
using namespace Belle2;
using namespace TrackFindingCDC;
using std::vector;
using std::map;
using std::pair;
using std::sort;
using std::min;
using std::max;

/// Helper function to reconstruct 2D distance from a hit to a 3D track

float getDist2D(const TrackFindingCDC::CDCTrajectory3D& trajectory, const TrackFindingCDC::CDCWireHit* wireHit)
{
  const TrackFindingCDC::CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
  const TrackFindingCDC::CDCTrajectorySZ& trajectorySZ = trajectory.getTrajectorySZ();
  TrackFindingCDC::Vector2D recoPos2D;
  if (wireHit->isAxial()) {
    recoPos2D = wireHit->reconstruct2D(trajectory2D);
  } else {
    const TrackFindingCDC::CDCWire& wire = wireHit->getWire();
    const TrackFindingCDC::Vector2D& posOnXYPlane = wireHit->reconstruct2D(trajectory2D);
    const double arcLength = trajectory2D.calcArcLength2D(posOnXYPlane);
    const double z = trajectorySZ.mapSToZ(arcLength);
    const TrackFindingCDC::Vector2D& wirePos2DAtZ = wire.getWirePos2DAtZ(z);
    const TrackFindingCDC::Vector2D& recoPosOnTrajectory = trajectory2D.getClosest(wirePos2DAtZ);
    const double driftLength = wireHit->getRefDriftLength();
    TrackFindingCDC::Vector2D disp2D = recoPosOnTrajectory - wirePos2DAtZ;
    disp2D.normalizeTo(driftLength);
    recoPos2D = wirePos2DAtZ + disp2D;
  }
  const float distanceToHit = trajectory2D.getDist2D(recoPos2D);
  return abs(distanceToHit);
}

void AsicBackgroundLibraryCreator::initialize()
{
  Super::initialize();
  // database:
  m_channelMapFromDB = std::make_unique<DBArray<CDCChannelMap>> ();

  if ((*m_channelMapFromDB).isValid()) {
    B2DEBUG(29, "CDC Channel map is  valid");
  } else {
    B2FATAL("CDC Channel map is not valid");
  }

  // Library for writing

  auto leavesCreator = [this](TTree & tree) {
    if (m_write_extra_vars) {
      tree.Branch("Dist", &m_dist_signal, "DistSig/f:DistBg/f");
      tree.Branch("ADC", &m_adc_sig, "ADC_Sig/S:ADC_bg/S");
      tree.Branch("Track", &m_n_hit_track, "tr_nhit/S");
    }
    tree.Branch("Board", &m_board, "board/b");
    tree.Branch("Channel", &m_channel, "channel/b");
    tree.Branch("Nhit", &m_n_hit, "nhit/b");
    tree.Branch("Asic", &m_asic_info[0],
                "TDC0/S:ADC0/S:TOT0/S:TDC1/S:ADC1/S:TOT1/S:TDC2/S:ADC2/S:TOT2/S:TDC3/S:ADC3/S:TOT3/S:TDC4/S:ADC4/S:TOT4/S:TDC5/S:ADC5/S:TOT5/S:TDC6/S:ADC6/S:TOT6/S:TDC7/S:ADC7/S:TOT7/S");
    return;
  };

  m_recorder.reset(new Recorder(
                     leavesCreator
                     , m_library_name
                     , "ASIC"));
}

void AsicBackgroundLibraryCreator::beginRun()
{
  Super::beginRun();
  // Load map from DB:
  for (const auto& cm : (*m_channelMapFromDB)) {
    const int isl = cm.getISuperLayer();
    const int il = cm.getILayer();
    const int iw = cm.getIWire();
    const int iBoard = cm.getBoardID();
    const int iCh = cm.getBoardChannel();
    const WireID wireId(isl, il, iw);
    m_map[wireId.getEWire()] = std::pair<int, int>(iBoard, iCh);
  }
}

std::string AsicBackgroundLibraryCreator::getDescription()
{
  return "Finds suitable ASICs with a single hit attached to a track and uses them to create the library";
}

void AsicBackgroundLibraryCreator::apply(const std::vector<CDCWireHit>& wireHits, const std::vector<CDCTrack>& tracks)
{

  map< pair<int, int>, vector<const CDCWireHit*>> groupedByAsic;
  for (const CDCWireHit& wireHit : wireHits) {
    auto eWire = wireHit.getWireID().getEWire();
    B2ASSERT("Channel map NOT found for the channel", m_map.count(eWire) > 0);
    auto board = m_map[eWire].first;
    auto channel = m_map[eWire].second;
    auto asicID = pair<int, int>(board, channel / 8);  // ASIC are groups of 8 channels
    groupedByAsic[asicID].push_back(&wireHit);
  };
  for (auto& [asicID, asicList] :  groupedByAsic) {
    selectAsic(asicList, tracks);
  };

  return;
}

void AsicBackgroundLibraryCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalHitNumberASIC"),
                                m_minimal_hit_number,
                                "Required number of hits per ASIC for library creation",
                                m_minimal_hit_number);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "AsicLibraryFileName"),
                                m_library_name,
                                "ASIC library file name",
                                m_library_name);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "maximalDistanceSignal"),
                                m_distance_signal_max,
                                "maximal distance in cm from track to signal hit",
                                m_distance_signal_max);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalDistanceBackground"),
                                m_distance_background_min,
                                "minimal distance in cm from track to background hit",
                                m_distance_background_min);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "useAxialHitsOnly"),
                                m_use_axial_hits_only,
                                "use axial layers only",
                                m_use_axial_hits_only);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "writeExtraVars"),
                                m_write_extra_vars,
                                "Write extra variables to the library",
                                m_write_extra_vars);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "minimalHitsOnTrack"),
                                m_minimal_hits_on_track,
                                "Required number of hits on track for library creation",
                                m_minimal_hits_on_track);

  // set some defaults:
  moduleParamList->getParameter<std::string>("inputTracks").setDefaultValue("CDCTrackVector");
  moduleParamList->getParameter<std::string>("inputWireHits").setDefaultValue("CDCWireHitVector");
}

void AsicBackgroundLibraryCreator::terminate()
{
  m_recorder->write();
  Super::terminate();
}

void AsicBackgroundLibraryCreator::selectAsic(const std::vector<const CDCWireHit*>& wireHits, const std::vector<CDCTrack>& tracks)
{

  if (wireHits.size() < m_minimal_hit_number) {
    return;
  };

  if (wireHits.size() > 8) {
    B2ERROR("Number of hits per asic should not exceed 8, observe too many hits." << LogVar("nHits", wireHits.size()));
    /// This is abnormal situation, detected for few runs, related to CDC unpacker. Hits are to be marked as background.
    return;
  }


  // count taken non-background hits:
  int count = 0;
  const CDCWireHit* signal = nullptr;
  for (auto& hit : wireHits) {
    if (!(*hit)->hasBackgroundFlag() && (*hit)->hasTakenFlag()) {
      count += 1;
      signal = hit;
    }
  }

  // require one and only one taken hit
  if (count != 1) {
    return;
  }


  // check if only axial hits are used:

  if ((!signal->isAxial()) && m_use_axial_hits_only) {
    return;
  }

  // find the track to which this hit belongs
  const CDCTrack* signalTrack = nullptr;
  for (auto& track : tracks) {
    for (auto& hit : track) {
      if (&hit.getWireHit() == signal) {
        signalTrack = &track;
        break;
      }
    }
    if (signalTrack != nullptr) {
      break;
    }
  }

  if (signalTrack == nullptr) {
    B2DEBUG(29, "AsicBackgroundLibraryCreator::No track found for the signal hit");
    return;
  }

  m_n_hit_track = signalTrack->size();

  if (m_n_hit_track < m_minimal_hits_on_track) return;

  // check the distance from the track to each signal hit
  const auto& trajectory = signalTrack->getStartTrajectory3D();

  m_dist_bg = 1000.;

  for (auto& hit : wireHits) {

    const float dist = getDist2D(trajectory, hit);


    if (hit == signal) {
      m_dist_signal = dist;
      if (dist > m_distance_signal_max) return;
    } else {
      m_dist_bg = min(m_dist_bg, dist);
      if (dist < m_distance_background_min) return;
    }
  }


  // Ok, passes all cuts !

  // reset the library entries
  for (auto& channel : m_asic_info) {
    channel.TDC = -1;
    channel.ADC = -1;
    channel.TOT = -1;
  }

  // add to the library

  m_n_hit = wireHits.size();

  m_adc_max_bg = 0;
  for (auto& hit : wireHits) {
    auto eWire = hit->getWireID().getEWire();
    auto channel = m_map[eWire].second;
    auto asicCH = channel % 8;
    m_asic_info[asicCH].ADC = hit->getHit()->getADCCount();
    m_asic_info[asicCH].TDC = hit->getHit()->getTDCCount();
    m_asic_info[asicCH].TOT = hit->getHit()->getTOT();

    if (hit != signal) {
      m_adc_max_bg = max(m_adc_max_bg, m_asic_info[asicCH].ADC);
    }
  }
  // also signal hit info
  auto eWire = signal->getWireID().getEWire();
  m_channel = m_map[eWire].second;
  m_board   = m_map[eWire].first;
  m_adc_sig = signal->getHit()->getADCCount();

  // make sure that ADC of the signal is >= than ADC of the background:
  if (m_adc_sig < m_adc_max_bg) return;

  // write out
  m_recorder->capture();
}
