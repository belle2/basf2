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
#include <cdc/dataobjects/WireID.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <trg/cdc/Layer.h>
#include <trg/cdc/Wire.h>
#include <trg/cdc/WireHit.h>
#include <trg/cdc/Segment.h>
#include <tracking/dataobjects/RecoTrack.h>

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
  addParam("makeRecoLRTable",
           m_makeRecoLRTable,
           "Switch to create a table of hit pattern <-> "
           "number of reconstructed left / true right, which is needed to create the LUT",
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
           false);
  addParam("innerRecoLRTableFilename",
           m_innerRecoLRTableFilename,
           "Filename for the reconnstructed left/right table for the innermost super layer.",
           string("innerRecoLRTable.dat"));
  addParam("outerRecoLRTableFilename",
           m_outerRecoLRTableFilename,
           "Filename for the reconstructed left/right table for the outer super layers.",
           string("outerRecoLRTable.dat"));
  addParam("relateAllHits",
           m_relateAllHits,
           "Flag to relate all cdchits to the TrackSegment, not just the priority hits.",
           true);
  addParam("ADC_cut_enable",
           m_adcflag,
           "remove hits with lower ADC than cut threshold. True:enable False:disable",
           false);
  addParam("ADC_cut_threshold",
           m_adccut,
           "Threshold for the adc cut.  Default: -1",
           -1);
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
    innerTrueLRTable.assign(pow(2, 16), vector<unsigned>(5, 0));
    outerTrueLRTable.assign(pow(2, 12), vector<unsigned>(5, 0));
  }
  if (m_makeRecoLRTable) {
    m_recoTracks.isRequired("RecoTracks");
    innerRecoLRTable.assign(pow(2, 16), vector<unsigned>(5, 0));
    outerRecoLRTable.assign(pow(2, 12), vector<unsigned>(5, 0));
  }
  // register relations
  StoreArray<MCParticle> mcparticles;
  m_segmentHits.registerRelationTo(m_cdcHits);
  mcparticles.registerRelationTo(m_segmentHits);
  m_recoTracks.registerRelationTo(m_segmentHits);
  m_recoTracks.registerRelationTo(m_cdcHits);
  m_segmentHits.registerRelationTo(m_recoTracks);
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
                                         superLayerId,
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
    int ncdchit_asic[500][6] = {{0}};
    vector<int> id_ncdchit_asic[500][6];
    for (int i = 0; i < m_cdcHits.getEntries(); ++i) {
      UChar_t lay = m_cdcHits[i]->getICLayer();
      UShort_t IWire = m_cdcHits[i]->getIWire();
      WireID wireid(lay, IWire);
      int boardid = cdc.getBoardID(wireid);
      int fechid = cdc.getChannelID(wireid);
      int asicid = fechid / 8;
      if (boardid >= 0 && boardid < 500 && asicid >= 0 && asicid < 6) {
        ncdchit_asic[boardid][asicid]++;
        id_ncdchit_asic[boardid][asicid].push_back(i);
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
            bool breakOuterLoop = false;
            for (int k = 0; k < ncdchit_asic[i][j] - ncoin; k++) {
              if (tdc_asic[k + ncoin - 1] - tdc_asic[k] <= 16) {
                for (int l = k; l < k + ncoin - 1; l++) {
                  filtered_hit[id_ncdchit_asic[i][j][l]] = 1;
                }
                breakOuterLoop = true;
                break;
              }
            }
            if (breakOuterLoop)
              break;
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

    // remove hits with low ADC
    if (m_adcflag) {
      if (h.getADCCount() < m_adccut)continue;
    }

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
        unsigned short adcSum = 0;
        // relation to all CDCHits in segment
        for (unsigned iw = 0; iw < s.wires().size(); ++iw) {
          const TRGCDCWire* wire = (TRGCDCWire*)s[iw];
          if (wire->signal().active()) {
            // priority wire has relation weight 2
            double weight = (wire == &(s.priority())) ? 2. : 1.; // not sure if this is needed..
            if (weight == 2. || m_relateAllHits) {
              tsHit->addRelationTo(m_cdcHits[wire->hit()->iCDCHit()], weight);
              adcSum += m_cdcHits[wire->hit()->iCDCHit()]->getADCCount();
            }
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
            if (isl == 0) {
              B2DEBUG(100, its << " creating entry in TrueLUT for pattern: " << s.lutPattern() << " :  " << simHit->getLeftRightPassage());
              innerTrueLRTable[s.lutPattern()][simHit->getLeftRightPassage()] += 1;
              innerTrueLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              innerTrueLRTable[s.lutPattern()][4] += adcSum;
            } else {
              outerTrueLRTable[s.lutPattern()][simHit->getLeftRightPassage()] += 1;
              outerTrueLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              outerTrueLRTable[s.lutPattern()][4] += adcSum;
              B2DEBUG(100, its << " creating entry in TrueLUT for pattern: " << s.lutPattern() << " :  " << simHit->getLeftRightPassage());
            }
          } else {
            if (isl == 0) {
              B2DEBUG(100, its  << " creating bghit in TrueLUT for pattern: " << s.lutPattern());
              innerTrueLRTable[s.lutPattern()][2] += 1;
              innerTrueLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              innerTrueLRTable[s.lutPattern()][4] += adcSum;
            }  else {
              B2DEBUG(100, its  << " creating bghit in TrueLUT for pattern: " << s.lutPattern());
              outerTrueLRTable[s.lutPattern()][2] += 1;
              outerTrueLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              outerTrueLRTable[s.lutPattern()][4] += adcSum;
            }
          }
        }

        if (m_makeRecoLRTable) {
          // for the recotable, we have no simhits and w can have more than one recotrack per event
          // so wee need to loop over them:
          unsigned lrflag = 2; // see explanation below
          for (int ireco = 0; ireco < m_recoTracks.getEntries(); ++ireco) {
            //        std::cout << "recotrack " << ireco << " of " << m_recoTracks.getEntries());
            RecoTrack* recoTrack = m_recoTracks[ireco];
            // since there is no relation between recotracks and the tshits yet, we need to create them first.
            // within the recotrack class, there is a function which returns the hitids of the cdchits used
            // in the recotrack. now we can loop over them and compare them with the id from the priorityhit:
            // /
            // Before looping over the recotracks, we set the rl information to 'bkg hit'. Then, we loop over all
            // recotracks and determine if there is a relation and wether it passed left or right. If this is set for
            // one recotrack, we set the rl information to the corresponding value. if it is set for another recotrack,
            // we will also use this information for the recolrtable and set the corresponding value again.
            // Just in the case, where after the loop over all recotracks it wasn't related to any of them, we will set
            // the background flag in the recolrtable.
            vector<CDCHit*> cdcHits = recoTrack->getCDCHitList();
            bool related = false;
            for (unsigned iHit = 0; iHit < cdcHits.size(); ++iHit) {
//std::cout << "now looping over cdchits... " << iHit << "/" << cdcHits.size() << std::endl;
              if (tsHit->getID() == cdcHits[iHit]->getID()) {
                // check, wether recotrack is already related to ts, skip in this case.
                // this is necessary because sometimes two wires are related to the same ts // dont get it, should be uneccessary
                if (related == false) related = true;
                else continue;
//              std::cout << "ts " << tsHit->getID() << " :  creating relation to recotrack " << ireco;
                //              std::cout << tsHit->getID() << " " << cdcHits[iHit]->getID() << " " << iHit << " matching id of priohit and current cdchit, creating relation... " << std::endl;
                recoTrack->addRelationTo(tsHit);
                tsHit->addRelationTo(recoTrack);
                if (isl == 0) {
                  // this getrightleftinformation function returns 2 for a right pass, and 3 for a left pass
                  if (recoTrack->getRightLeftInformation(cdcHits[iHit]) ==  3) lrflag = 0;
                  if (recoTrack->getRightLeftInformation(cdcHits[iHit]) ==  2) lrflag = 1;
                  innerRecoLRTable[s.lutPattern()][lrflag] += 1;
                  innerRecoLRTable[s.lutPattern()][3] += tsHit->priorityTime();
                  innerRecoLRTable[s.lutPattern()][4] += adcSum;
                  B2DEBUG(100, its << " creating entry in recoLUT for pattern: " << s.lutPattern() << " :  " << lrflag << " (recotrack " << ireco <<
                          "), hit: " << iHit);
                } else {
                  if (recoTrack->getRightLeftInformation(cdcHits[iHit]) ==  3) lrflag = 0;
                  if (recoTrack->getRightLeftInformation(cdcHits[iHit]) ==  2) lrflag = 1;
                  outerRecoLRTable[s.lutPattern()][lrflag] += 1;
                  outerRecoLRTable[s.lutPattern()][3] += tsHit->priorityTime();
                  outerRecoLRTable[s.lutPattern()][4] += adcSum;
                  B2DEBUG(100, its << " creating entry in recoLUT for pattern: " << s.lutPattern() << " :  " << lrflag << " (recotrack " << ireco <<
                          "), hit: " << iHit);
                }
//std::cout << " , lrflag: " << lrflag << ", 0=left, 1=right";
                //break;
              }
            }
          }
          if (lrflag == 2) {
            if (isl == 0) {
              B2DEBUG(100, its << " creating bghit in recoLUT for pattern: " << s.lutPattern());
              innerRecoLRTable[s.lutPattern()][2] += 1;
              innerRecoLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              innerRecoLRTable[s.lutPattern()][4] += adcSum;
            } else {
              B2DEBUG(100, its << " creating bghit in recoLUT for pattern: " << s.lutPattern());
              outerRecoLRTable[s.lutPattern()][2] += 1;
              outerRecoLRTable[s.lutPattern()][3] += tsHit->priorityTime();
              outerRecoLRTable[s.lutPattern()][4] += adcSum;

            }
            //std::cout << " , lrflag: " << lrflag << ", 0=left, 1=right, 2=bg";
          }
          //std::cout << std::endl;
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

  // save reco left/right table
  if (m_makeRecoLRTable) {
    ofstream innerFile(m_innerRecoLRTableFilename);
    ostream_iterator<unsigned> innerIterator(innerFile, " ");
    for (unsigned pattern = 0; pattern < innerRecoLRTable.size(); ++pattern) {
      copy(innerRecoLRTable[pattern].begin(), innerRecoLRTable[pattern].end(),
           innerIterator);
      innerFile << "\n";
    }
    ofstream outerFile(m_outerRecoLRTableFilename);
    ostream_iterator<unsigned> outerIterator(outerFile, " ");
    for (unsigned pattern = 0; pattern < outerRecoLRTable.size(); ++pattern) {
      copy(outerRecoLRTable[pattern].begin(), outerRecoLRTable[pattern].end(),
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
