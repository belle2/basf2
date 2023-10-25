/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/dataobjects/CDCTriggerMLPInput.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <framework/dataobjects/BinnedEventT0.h>

#include "TH1F.h"

using namespace Belle2;
/** CDC Trigger DQM Module */
class CDCTriggerNeuroDQMOnlineModule : public HistoModule {  // <- derived from HistoModule class

public:
  /** Constructor */
  CDCTriggerNeuroDQMOnlineModule();
  /** Destructor */
  virtual ~CDCTriggerNeuroDQMOnlineModule();


private:

  struct TSLine {
    const CDCTriggerSegmentHit* hit;
    std::string strline;
    //vector<const CDCTriggerTrack*> relin2dtracks;
    //vector<const CDCTriggerTrack*> relin2dtracks;
    explicit TSLine(const CDCTriggerSegmentHit& h)
    {
      hit = &h;
    }
  };
  typedef std::vector<TSLine> TSLines;

  /** Module functions */
  void initialize() override;
  void beginRun() override;
  void event() override;
  void endRun() override;
  void terminate() override;
  void fillHWPlots();
  void fillSimPlots();
  void makeDebugOutput();
  void fillRecoPlots();

  /**
   * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
   * to be placed in this function.
  */
  void defineHisto() override;
  /** Fill a histogram only with non-zero values */
  void condFill(TH1F* histo, float value)
  {
    if (value != 0) {
      histo->Fill(value);
    }
  }
  /** get pattern of the track */
  unsigned getPattern(CDCTriggerTrack* track, std::string hitCollectionName)
  {
    unsigned pattern = 0;
    for (const CDCTriggerSegmentHit& hit : track->getRelationsTo<CDCTriggerSegmentHit>(hitCollectionName)) {
      unsigned sl = hit.getISuperLayer();
      if (sl % 2 == 1) pattern |= (1 << ((sl - 1) / 2));
    }
    return pattern;
  }
  /** Validity of the pattern*/
  bool isValidPattern(unsigned pattern)
  {
    bool valid = false;
    switch (pattern) {
      case 7:
      case 11:
      case 13:
      case 14:
      case 15:
        valid = true;
    }
    return valid;
  }
  std::string padto(std::string s, unsigned l)
  {
    if (s.size() < l) {
      s.insert(s.begin(), l - s.size(), ' ');
    }
    return s;
  }
  std::string padright(std::string s, unsigned l)
  {
    if (s.size() < l) {
      s.insert(s.end(), l - s.size(), ' ');
    }
    return s;
  }
  bool have_relation(const CDCTriggerTrack& track, const CDCTriggerSegmentHit& hit, std::string& arrayname)
  {
    bool related = false;
    for (const CDCTriggerSegmentHit& ts : track.getRelationsTo<CDCTriggerSegmentHit>(arrayname)) {
      if (&ts == &hit) {related = true;}
    }
    return related;
  }
  void sorted_insert(TSLines& lines, TSLine& line, std::string& arrayname, std::string& firstsortarray, std::string& secondsortarray)
  {
    bool inserted = false;
    bool related = false;
    TSLines::iterator it = lines.begin();
    for (const CDCTriggerTrack& track : line.hit->getRelationsFrom<CDCTriggerTrack>(firstsortarray)) {
      if (!inserted) {
        for (TSLines::iterator i = lines.begin(); i < lines.end(); ++i) {
          if (i->hit->getISuperLayer() % 2 != line.hit->getISuperLayer() % 2) {
            continue;
          }
          if (have_relation(track, *(i->hit), arrayname)) {
            it = i;
            related = true;
            if (i->hit->getSegmentID() > line.hit->getSegmentID()) {
              inserted = true;
              break;
            }
          }
        }
      } else { break; }
    }
    if (related) {
      if (!inserted) {++it; }
      lines.insert(it, line);
    } else {
      for (const CDCTriggerTrack& track : line.hit->getRelationsFrom<CDCTriggerTrack>(secondsortarray)) {
        if (!inserted) {
          for (TSLines::iterator i = it; i < lines.end(); ++i) {
            if (i->hit->getISuperLayer() % 2 != line.hit->getISuperLayer() % 2) {
              continue;
            }
            if (have_relation(track, *(i->hit), arrayname)) {
              it = i;
              related = true;
              if (i->hit->getSegmentID() > line.hit->getSegmentID()) {
                inserted = true;
                break;
              }
            }
          }
        } else { break; }
      }
      if (related) {
        if (!inserted) {++it; }
        lines.insert(it, line);
      } else {
        lines.push_back(line);
      }
    }
  }
  /** Name of the histogram directory in ROOT file */
  std::string m_histogramDirectoryName;

  /** Name for TS hits from unpacker */
  std::string m_unpackedNeuroInputAllSegmentHitsName;
  /** Name for 2D finder tracks from unpacker */
  std::string m_unpacked2DTracksName;
  /** Name for neuro tracks from unpacker */
  std::string m_unpackedNeuroTracksName;
  /** Name for neuro input vector from unpacker */
  std::string m_unpackedNeuroInputVectorName;
  /** Name for neuro input 2d finder tracks */
  std::string m_unpackedNeuroInput2DTracksName;
  /** Name for neuro input Track segments */
  std::string m_unpackedNeuroInputSegmentHitsName;
  /** name of the storearray for hwsim tracks */
  std::string m_simNeuroTracksName;
  /** Name for simulated neuro input vector using HW TS, HW 2D*/
  std::string m_simNeuroInputVectorName;
  /** Name for the RecoTrack array name */
  std::string m_recoTracksName;

  /** Switch to turn on use of recotracks */
  bool m_useRecoTracks;
  /** Switch to turn on use of hw sim tracks */
  bool m_useSimTracks;

  // store arrays for direct access
  /** StoreArray for all TS hits from neuro unpacker */
  StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputAllSegmentHits;
  /** StoreArray for 2D finder tracks from unpacker */
  StoreArray<CDCTriggerTrack> m_unpacked2DTracks;
  /** StoreArray for neuro tracks from unpacker */
  StoreArray<CDCTriggerTrack> m_unpackedNeuroTracks;
  /** StoreArray for neuro input vector from unpacker */
  StoreArray<CDCTriggerMLPInput> m_unpackedNeuroInputVector;
  /** StoreArray for neuro input 2dfinder tracks */
  StoreArray<CDCTriggerTrack> m_unpackedNeuroInput2DTracks;
  /** StoreArray for neuro input Track segments */
  StoreArray<CDCTriggerSegmentHit> m_unpackedNeuroInputSegmentHits;
  /** StoreArray for neuro tracks from TSIM */
  StoreArray<CDCTriggerTrack> m_simNeuroTracks;
  /** StoreArray for neuro input vector from TSIM */
  StoreArray<CDCTriggerMLPInput> m_simNeuroInputVector;
  /** StoreArray for RecoTracks */
  StoreArray<RecoTrack> m_RecoTracks;
  /** storeobjpointer for event time */
  StoreObjPtr<BinnedEventT0> m_eventTime;




  TH1F* m_neuroHWOutdzall = nullptr;
  TH1F* m_neuroHWOutdz0 = nullptr;
  TH1F* m_neuroHWOutdz1 = nullptr;
  TH1F* m_neuroHWOutdz2 = nullptr;
  TH1F* m_neuroHWOutdz3 = nullptr;
  TH1F* m_neuroHWSimRecodZ = nullptr;

  TH1F* m_neuroHWOutZ = nullptr;
  TH1F* m_recoZ = nullptr;
  TH1F* m_recoZ_related = nullptr;
  TH1F* m_neuroHWOutSTTZ = nullptr;
  TH1F* m_neuroHWOutCosTheta = nullptr;
  TH1F* m_neuroHWOutPhi0 = nullptr;
  TH1F* m_neuroHWOutPt = nullptr;
  TH1F* m_neuroHWOutP = nullptr;
  TH1F* m_neuroHWOutm_time = nullptr;
  TH1F* m_neuroHWValTracksNumber = nullptr;
  TH1F* m_neuroHWSector = nullptr;
  TH1F* m_neuroHWInTSID = nullptr;
  TH1F* m_neuroHWInTSIDSel = nullptr;
  TH1F* m_neuroHWInCDCFE = nullptr;
  TH1F* m_neuroHWInm_time = nullptr;
  TH1F* m_neuroHWIn2DTracksNumber = nullptr;

  TH1F* m_neuroHWOutHwSimdZ = nullptr;
  TH1F* m_neuroHWSimZ = nullptr;
  TH1F* m_neuroHWSimCosTheta = nullptr;
  TH1F* m_neuroErrors = nullptr;

  unsigned m_errcount = 9;
  std::vector<std::string> m_errdict{"Not All Axials in HWTrack", "|HW-SW| > 1cm", "Delta Input IDs not all 0", "Delta Input Alphas not all 0", "Drifttime Overflow / Scaling Error", "TS only in HW", "TS only in SW", "Multiple ET per Track", "Eventcounter"};


};



