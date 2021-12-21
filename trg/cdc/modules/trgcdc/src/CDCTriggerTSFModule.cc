/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "trg/cdc/modules/trgcdc/CDCTriggerTSFModule.h"

#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <trg/cdc/Layer.h>
#include <trg/cdc/Wire.h>
#include <trg/cdc/WireHit.h>
#include <trg/cdc/Segment.h>

#include <fstream>

#define P3D HepGeom::Point3D<double>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(CDCTriggerTSF);

CDCTriggerTSFModule::CDCTriggerTSFModule() : Module::Module()
{
  setDescription(
    "The Track Segment Finder module of the CDC trigger.\n"
    "Combines CDCHits from the same super layer to CDCTriggerSegmentHits.\n"
  );
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("CDCHitCollectionName",
           m_CDCHitCollectionName,
           "Name of the input StoreArray of CDCHits.",
           string("CDCHits4Trg"));
  addParam("TSHitCollectionName",
           m_TSHitCollectionName,
           "Name of the output StoreArray of CDCTriggerSegmentHits.",
           string(""));
  addParam("InnerTSLUTFile",
           m_innerTSLUTFilename,
           "The filename of LUT for track segments from the inner-most super layer",
           string(""));
  addParam("OuterTSLUTFile",
           m_outerTSLUTFilename,
           "The filename of LUT for track segments from the outer super layers",
           string(""));
  addParam("ClockSimulation",
           m_clockSimulation,
           "Switch to simulate each data clock cycle separately.",
           false);
  addParam("makeTrueLRTable",
           m_makeTrueLRTable,
           "Switch to create a table of hit pattern <-> "
           "number of true left / true right, which is needed to create the LUT",
           false);
  addParam("innerTrueLRTableFilename",
           m_innerTrueLRTableFilename,
           "Filename for the true left/right table for the innermost super layer.",
           string("innerTrueLRTable.dat"));
  addParam("outerTrueLRTableFilename",
           m_outerTrueLRTableFilename,
           "Filename for the true left/right table for the outer super layers.",
           string("outerTrueLRTable.dat"));
  addParam("Deadchannel",
           m_deadchflag,
           "Mask dead channels based on database. True:mask False:unmask",
           true);
  addParam("Crosstalk_tdcfilter",
           m_crosstalk_tdcfilter,
           "TDC based crosstalk filtering logic on CDCFE. True:enable False:disable",
           true);
}

void
CDCTriggerTSFModule::initialize()
{
  // register DataStore elements
  m_segmentHits.registerInDataStore(m_TSHitCollectionName);
  m_cdcHits.isRequired(m_CDCHitCollectionName);
  if (m_makeTrueLRTable) {
    StoreArray<CDCSimHit> simhits;
    simhits.isRequired();
    innerTrueLRTable.assign(pow(2, 16), vector<unsigned>(3, 0));
    outerTrueLRTable.assign(pow(2, 12), vector<unsigned>(3, 0));
  }
  // register relations
  StoreArray<MCParticle> mcparticles;
  m_segmentHits.registerRelationTo(m_cdcHits);
  mcparticles.registerRelationTo(m_segmentHits);

  // Prepare track segment shapes.
  // First a structure of wires is created for all layers and super layers.
  // Each track segment consists of pointers to wires in this structure.
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  const unsigned nLayers = cdc.nWireLayers();
  TRGClock* clockTDC = new TRGClock("CDCTrigger TDC clock", 0, 500. / cdc.getTdcBinWidth());
  TRGClock* clockData = new TRGClock("CDCTrigger data clock", *clockTDC, 1, 16);
  clocks.push_back(clockTDC);
  clocks.push_back(clockData);

  //...Loop over layers...
  int superLayerId = -1;
  unsigned lastNWires = 0;
  int lastShifts = -1000;
  // separate counters for axial and stereo layers and super layers
  int ia = -1;
  int is = -1;
  int ias = -1;
  int iss = -1;
  /* cppcheck-suppress variableScope */
  unsigned axialStereoLayerId;
  unsigned axialStereoSuperLayerId = 0;
  unsigned nWires = 0;
  for (unsigned i = 0; i < nLayers; i++) {
    if (i < cdc.getOffsetOfFirstLayer()) {
      continue;
    }
    const unsigned nWiresInLayer = cdc.nWiresInLayer(i);

    //...Axial or stereo?...
    int nShifts = cdc.nShifts(i);
    bool axial = (nShifts == 0);
    if (axial) ++ia;
    else ++is;
    axialStereoLayerId = (axial) ? ia : is;

    // Add empty TRGCDCLayer in case a superlayer is not present
    if (superLayers.size() == 0 and cdc.getOffsetOfFirstSuperLayer() != 0) {
      for (uint superLayerOffset = 0; superLayerOffset < cdc.getOffsetOfFirstSuperLayer(); superLayerOffset++) {
        superLayers.push_back(vector<TRGCDCLayer*>());
        superLayerId++;
      }
    }

    //...Is this in a new super layer?...
    if ((lastNWires != nWiresInLayer) || (lastShifts != nShifts)) {
      ++superLayerId;
      superLayers.push_back(vector<TRGCDCLayer*>());
      if (axial) ++ias;
      else ++iss;
      axialStereoSuperLayerId = (axial) ? ias : iss;
      lastNWires = nWiresInLayer;
      lastShifts = nShifts;
    }

    //...Calculate radius of adjacent field wires...
    const float swr = cdc.senseWireR(i);
    const float innerRadius = cdc.fieldWireR(i - 1);
    const float outerRadius = (i < nLayers - 1) ?
                              cdc.fieldWireR(i) :
                              swr + (swr - innerRadius);

    //...New layer...
    TRGCDCLayer* layer = new TRGCDCLayer(i,
                                         superLayerId + cdc.getOffsetOfFirstSuperLayer(),
                                         superLayers[superLayerId].size(),
                                         axialStereoLayerId,
                                         axialStereoSuperLayerId,
                                         cdc.zOffsetWireLayer(i),
                                         nShifts,
                                         M_PI * swr * swr / nWiresInLayer,
                                         nWiresInLayer,
                                         innerRadius,
                                         outerRadius);
    superLayers.back().push_back(layer);

    //...Loop over all wires in a layer...
    for (unsigned j = 0; j < nWiresInLayer; j++) {
      const P3D fp = P3D(cdc.wireForwardPosition(i, j).x(),
                         cdc.wireForwardPosition(i, j).y(),
                         cdc.wireForwardPosition(i, j).z());
      const P3D bp = P3D(cdc.wireBackwardPosition(i, j).x(),
                         cdc.wireBackwardPosition(i, j).y(),
                         cdc.wireBackwardPosition(i, j).z());
      TRGCDCWire* tw = new TRGCDCWire(nWires++, j, *layer, fp, bp, *clockTDC);
      layer->push_back(tw);
    }
  }

  //...Make TSF's...
  const unsigned nWiresInTS[2] = {15, 11};
  const int shape[2][30] = {
    {
      -2, 0, // relative layer id, relative wire id
      -1, -1,
      -1, 0,
      0, -1,
      0, 0,
      0, 1,
      1, -2,
      1, -1,
      1, 0,
      1, 1,
      2, -2,
      2, -1,
      2, 0,
      2, 1,
      2, 2
    },
    {
      -2, -1,
      -2, 0,
      -2, 1,
      -1, -1,
      -1, 0,
      0, 0,
      1, -1,
      1, 0,
      2, -1,
      2, 0,
      2, 1,
      0, 0,
      0, 0,
      0, 0,
      0, 0
    }
  };
  const int layerOffset[2] = {5, 2};
  unsigned id = 0;
  unsigned idTS = 0;
  for (unsigned i = 0; i < superLayers.size(); i++) {
    if (i < cdc.getOffsetOfFirstSuperLayer()) {
      TRGCDCLayer* emptylayer = new TRGCDCLayer();
      tsLayers.push_back(emptylayer);
      continue;
    }
    unsigned tsType = (i) ? 1 : 0;

    //...TS layer... w is a central wire
    const TRGCDCCell* ww = superLayers[i][layerOffset[tsType]]->front();
    TRGCDCLayer* layer = new TRGCDCLayer(id++, *ww);
    tsLayers.push_back(layer);

    //...Loop over all wires in a central wire layer...
    const unsigned nWiresInLayer = ww->layer().nCells();
    B2DEBUG(100, "SL " << i << " layerOffset " << layerOffset[tsType] << ", "
            << superLayers[i].size() << " layers, " << nWiresInLayer << " wires");
    for (unsigned j = 0; j < nWiresInLayer; j++) {
      const TRGCDCWire& w =
        (TRGCDCWire&) superLayers[i][layerOffset[tsType]]->cell(j);

      const unsigned localId = w.localId();
      const unsigned layerId = w.localLayerId();
      vector<const TRGCDCWire*> cells;

      B2DEBUG(110, "TS localId " << localId << " layerId " << layerId);

      for (unsigned k = 0; k < nWiresInTS[tsType]; k++) {
        const int laid = layerId + shape[tsType][k * 2];
        const int loid = localId + shape[tsType][k * 2 + 1];

        B2DEBUG(120, "cell localId " << loid << " layerId " << laid);

        const TRGCDCWire& c =
          (TRGCDCWire&) superLayers[i][laid]->cell(loid);

        cells.push_back(&c);
      }

      TRGCDCSegment* ts;
      if (w.superLayerId()) {
        ts = new TRGCDCSegment(idTS++,
                               *layer,
                               w,
                               *clockData,
                               m_outerTSLUTFilename,
                               cells);
      } else {
        ts = new TRGCDCSegment(idTS++,
                               *layer,
                               w,
                               *clockData,
                               m_innerTSLUTFilename,
                               cells);
      }
      ts->initialize();
      layer->push_back(ts);
    }
  }

}

void
CDCTriggerTSFModule::beginRun()
{
  if (m_deadchflag) {
    if (not m_db_deadchannel.isValid()) {
      StoreObjPtr<EventMetaData> evtMetaData;
      B2ERROR("No database for CDCTRG dead channel mapping. Channel masking is skipped. exp " << evtMetaData->getExperiment() << " run "
              << evtMetaData->getRun());
      for (unsigned int i = 0; i < c_nSuperLayers; i++) { //SL
        for (unsigned int j = 0; j < MAX_N_LAYERS; j++) { //Layer
          for (unsigned int k = 0; k < c_maxNDriftCells; k++) { //
            deadch_map[i][j][k] = true;
          }
        }
      }
    } else {
      for (unsigned int i = 0; i < c_nSuperLayers; i++) { //SL
        for (unsigned int j = 0; j < MAX_N_LAYERS; j++) { //Layer
          for (unsigned int k = 0; k < c_maxNDriftCells; k++) { //
            deadch_map[i][j][k] = m_db_deadchannel->getdeadch(i, j, k);
          }
        }
      }
    }
  }
}

void
CDCTriggerTSFModule::event()
{
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();

  // fill CDCHits into track segment shapes

  //crosstalk filter
  vector<int> filtered_hit;
  for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
    filtered_hit.push_back(0);
  }

  if (m_crosstalk_tdcfilter) {
    //check number of hits in each asic
    int ncdchit_asic[500][6] = {0};
    vector<int> id_ncdchit_asic[500][6];
    for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
      RelationVector<CDCRawHit> cdcrawHits = m_cdcHits[i]->getRelationsTo<CDCRawHit>();
      if (cdcrawHits.size() > 0) {
        CDCRawHit* cdcrawhit = cdcrawHits[0];
        int boardid = cdcrawhit->getBoardId();
        int fechid = cdcrawhit->getFEChannel();
        int asicid = fechid / 8;
        if (boardid >= 0 && boardid < 500 && asicid >= 0 && asicid < 6) {
          ncdchit_asic[boardid][asicid]++;
          id_ncdchit_asic[boardid][asicid].push_back(i);
        }
      }
    }
    //check 16ns time coinsidence if >=4 hits are found in the same asic
    for (int i = 0; i < 500; i++) {
      for (int j = 0; j < 6; j++) {
        if (ncdchit_asic[i][j] >= 4) {
          std::vector<short> tdc_asic;
          for (int k = 0; k < ncdchit_asic[i][j]; k++) {
            short tdc = m_cdcHits[id_ncdchit_asic[i][j][k]]->getTDCCount();
            tdc_asic.push_back(tdc);
          }
          std::sort(tdc_asic.begin(), tdc_asic.end());
          for (int ncoin = ncdchit_asic[i][j]; ncoin >= 4; ncoin--) {
            for (int k = 0; k < ncdchit_asic[i][j] - ncoin; k++) {
              if (tdc_asic[k + ncoin - 1] - tdc_asic[k] <= 16) {
                for (int l = k; l < k + ncoin - 1; l++) {
                  filtered_hit[id_ncdchit_asic[i][j][l]] = 1;
                }
                //break loop
                ncoin = 0;
                k = 8;
              }
            }
          }
          tdc_asic.clear();
        }
      }
    }
  }

  //...Loop over CDCHits...
  for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
    // get the wire
    const CDCHit& h = *m_cdcHits[i];
    // mask dead channel
    if (m_deadchflag) {
      if (!deadch_map[h.getISuperLayer()][h.getILayer()][h.getIWire()]) {
        continue;
      }
    }
    // skim crosstalk hit
    if (filtered_hit[i] == 1)continue;

    TRGCDCWire& w =
      (TRGCDCWire&) superLayers[h.getISuperLayer()][h.getILayer()]->cell(h.getIWire());

    // trigger timing signal
    const int tdcCount = floor((cdc.getT0(WireID(h.getID())) / cdc.getTdcBinWidth()
                                - h.getTDCCount() + 0.5) / 2);
    TRGTime rise = TRGTime(tdcCount, true, w.signal().clock(), w.name());
    TRGTime fall = rise;
    fall.shift(1).reverse();
    TRGSignal signal = rise & fall;
    w.addSignal(signal);

    if (w.hit()) continue;
    // make a trigger wire hit (needed for relations)
    // all unneeded variables are set to 0 (TODO: remove them completely?)
    TRGCDCWireHit* hit = new TRGCDCWireHit(w, i,
                                           0, 0, 0, 0, 0, 0, 0, 0);
    w.hit(hit);
  }



  // neibor supression
  unsigned neibor_hit[10][1000] = {};
  for (unsigned isl = 0; isl < tsLayers.size(); ++isl) {
    for (unsigned its = 0; its < tsLayers[isl]->nCells(); ++its) {
      TRGCDCSegment& s = (TRGCDCSegment&) tsLayers[isl]->cell(its);
      // simulate with logicLUTFlag = true
      // TODO: either add parameter or remove the option in Segment::simulate()
      s.simulate(m_clockSimulation, true,
                 m_CDCHitCollectionName, m_TSHitCollectionName);
      if (!m_clockSimulation && s.signal().active() && s.priorityPosition() == 3) {
        neibor_hit[isl][its] = 1;
      }
    }
  }

  // simulate track segments and create track segment hits
  for (unsigned isl = 0; isl < tsLayers.size(); ++isl) {
    for (unsigned its = 0; its < tsLayers[isl]->nCells(); ++its) {
      TRGCDCSegment& s = (TRGCDCSegment&) tsLayers[isl]->cell(its);
      // simulate with logicLUTFlag = true
      // TODO: either add parameter or remove the option in Segment::simulate()
      s.simulate(m_clockSimulation, true,
                 m_CDCHitCollectionName, m_TSHitCollectionName);
      // store hits and create relations
      // for clock simulation already done in simulate
      // TODO: move it to simulate also for simulateWithoutClock?
      if (!m_clockSimulation && s.signal().active()) {

        //neibor supression
        if (s.priorityPosition() != 3 && (neibor_hit[isl][(its - 1) % tsLayers[isl]->nCells()] == 1
                                          || neibor_hit[isl][(its + 1) % tsLayers[isl]->nCells()] == 1))continue;

        const CDCHit* priorityHit = m_cdcHits[s.priority().hit()->iCDCHit()];
        const CDCTriggerSegmentHit* tsHit =
          m_segmentHits.appendNew(*priorityHit,
                                  s.id(),
                                  s.priorityPosition(),
                                  s.LUT()->getValue(s.lutPattern()),
                                  s.priorityTime(),
                                  s.fastestTime(),
                                  s.foundTime());
        // relation to all CDCHits in segment
        for (unsigned iw = 0; iw < s.wires().size(); ++iw) {
          const TRGCDCWire* wire = (TRGCDCWire*)s[iw];
          if (wire->signal().active()) {
            // priority wire has relation weight 2
            double weight = (wire == &(s.priority())) ? 2. : 1.;
            tsHit->addRelationTo(m_cdcHits[wire->hit()->iCDCHit()], weight);
          }
        }
        // relation to MCParticles (same as priority hit)
        RelationVector<MCParticle> mcrel = priorityHit->getRelationsFrom<MCParticle>();
        for (unsigned imc = 0; imc < mcrel.size(); ++imc) {
          mcrel[imc]->addRelationTo(tsHit, mcrel.weight(imc));
        }
        // get true left/right
        if (m_makeTrueLRTable) {
          const CDCSimHit* simHit = priorityHit->getRelatedFrom<CDCSimHit>();
          if (simHit && !simHit->getBackgroundTag()) {
            if (isl == 0)
              innerTrueLRTable[s.lutPattern()][simHit->getLeftRightPassage()] += 1;
            else
              outerTrueLRTable[s.lutPattern()][simHit->getLeftRightPassage()] += 1;
          } else {
            if (isl == 0)
              innerTrueLRTable[s.lutPattern()][2] += 1;
            else
              outerTrueLRTable[s.lutPattern()][2] += 1;
          }
        }
      }
    }
  }

  //...Clear hit information after we're finished...
  clear();
}

void
CDCTriggerTSFModule::terminate()
{
  // delete clocks
  for (unsigned ic = 0; ic < clocks.size(); ++ic) {
    delete clocks[ic];
  }
  clocks.clear();

  // delete wire layers
  for (unsigned isl = 0; isl < superLayers.size(); ++isl) {
    for (unsigned il = 0; il < superLayers[isl].size(); ++il) {
      for (unsigned iw = 0; iw < superLayers[isl][il]->nCells(); ++iw) {
        delete &(superLayers[isl][il]->cell(iw));
      }
      delete superLayers[isl][il];
    }
    superLayers[isl].clear();
  }
  superLayers.clear();

  // delete TS layers
  for (unsigned isl = 0; isl < tsLayers.size(); ++isl) {
    for (unsigned its = 0; its < tsLayers[isl]->nCells(); ++its) {
      delete &(tsLayers[isl]->cell(its));
    }
    delete tsLayers[isl];
  }
  tsLayers.clear();

  // save true left/right table
  if (m_makeTrueLRTable) {
    ofstream innerFile(m_innerTrueLRTableFilename);
    ostream_iterator<unsigned> innerIterator(innerFile, " ");
    for (unsigned pattern = 0; pattern < innerTrueLRTable.size(); ++pattern) {
      copy(innerTrueLRTable[pattern].begin(), innerTrueLRTable[pattern].end(),
           innerIterator);
      innerFile << "\n";
    }
    ofstream outerFile(m_outerTrueLRTableFilename);
    ostream_iterator<unsigned> outerIterator(outerFile, " ");
    for (unsigned pattern = 0; pattern < outerTrueLRTable.size(); ++pattern) {
      copy(outerTrueLRTable[pattern].begin(), outerTrueLRTable[pattern].end(),
           outerIterator);
      outerFile << "\n";
    }
  }

}

void
CDCTriggerTSFModule::clear()
{
  const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
  for (unsigned isl = 0; isl < superLayers.size(); ++isl) {
    if (isl < cdc.getOffsetOfFirstSuperLayer()) {
      continue;
    }
    for (unsigned il = 0; il < superLayers[isl].size(); ++il) {
      for (unsigned iw = 0; iw < superLayers[isl][il]->nCells(); ++iw) {
        TRGCDCWire& w = (TRGCDCWire&) superLayers[isl][il]->cell(iw);
        delete w.hit();
        w.clear();
      }
    }
    for (unsigned its = 0; its < tsLayers[isl]->nCells(); ++its) {
      TRGCDCSegment& s = (TRGCDCSegment&) tsLayers[isl]->cell(its);
      s.clear();
    }
  }

}
