/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <cdc/modules/cdcCrossTalkAdder/CDCCrossTalkAdderModule.h>

#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <map>

using namespace std;
using namespace Belle2;
using namespace CDC;

// register module
REG_MODULE(CDCCrossTalkAdder)
CDCCrossTalkAdderModule::CDCCrossTalkAdderModule() : Module()
{
  // Set description
  setDescription("Overlays signal-induced asic cross-talk to CDCHits.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("InputCDCHitsName", m_inputCDCHitsName, "Name of input array. Should consist of CDCHits.", string(""));
  addParam("Issue2ndHitWarning", m_issue2ndHitWarning, "=true: issue a warning when a 2nd TDC hit is found.", true);
  addParam("IncludeEarlyXTalks", m_includeEarlyXTalks, "=true: include earlier x-talks as well than the signal hit in question.",
           true);
  addParam("DebugLevel", m_debugLevel, "Debug level; 20-29 are usable.", 20);
}

void CDCCrossTalkAdderModule::initialize()
{
  m_hits.isRequired(m_inputCDCHitsName);

  m_cdcgp = &(CDCGeometryPar::Instance());

  m_invOfTDCBinWidth = 1. / m_cdcgp->getTdcBinWidth();

  m_xTalkFromDB = new DBObjPtr<CDCCrossTalkLibrary>;
  if ((*m_xTalkFromDB).isValid()) {
  } else {
    B2FATAL("CDCCrossTalkLibrary invalid!");
  }

  m_fEElectronicsFromDB = new DBArray<CDCFEElectronics>;
  if ((*m_fEElectronicsFromDB).isValid()) {
    (*m_fEElectronicsFromDB).addCallback(this, &CDCCrossTalkAdderModule::setFEElectronics);
    setFEElectronics();
  } else {
    B2FATAL("CDCCrossTalkAdder:: CDCFEElectronics not valid!");
  }
}

void CDCCrossTalkAdderModule::event()
{
  map<WireID, XTalkInfo> xTalkMap;
  map<WireID, XTalkInfo> xTalkMap1;
  map<WireID, XTalkInfo>::iterator iterXTalkMap1;

  // Loop over all cdc hits to create a xtalk map
  int OriginalNoOfHits = m_hits.getEntries();
  B2DEBUG(m_debugLevel, "\n \n" << "#CDCHits " << OriginalNoOfHits);
  for (const auto& aHit : m_hits) {
    if (m_issue2ndHitWarning && aHit.is2ndHit()) {
      B2WARNING("2nd TDC hit found, but not ready for it!");
    }
    WireID wid(aHit.getID());
    //    B2DEBUG(m_debugLevel, "Encoded wireid of current CDCHit: " << wid);
    short tdcCount = aHit.getTDCCount();
    short adcCount = aHit.getADCCount();
    short tot      = aHit.getTOT();
    short board    = m_cdcgp->getBoardID(wid);
    short channel  = m_cdcgp->getChannelID(wid);
    const vector<pair<short, asicChannel>> xTalks = (*m_xTalkFromDB)->getLibraryCrossTalk(channel, tdcCount, adcCount, tot);

    int nXTalks = xTalks.size();
    for (int i = 0; i < nXTalks; ++i) {
      const unsigned short tdcCount4XTalk = xTalks[i].second.TDC;
      if (i == 0) {
        B2DEBUG(m_debugLevel, "\n" << "          signal: " << channel << " " << tdcCount << " " << adcCount << " " << tot);
      }
      B2DEBUG(m_debugLevel, "xtalk: " << xTalks[i].first << " " << tdcCount4XTalk << " " << xTalks[i].second.ADC << " " <<
              xTalks[i].second.TOT);
      WireID widx = m_cdcgp->getWireID(board, xTalks[i].first);
      if (!m_cdcgp->isBadWire(widx)) { // for non-bad wire
        if (m_includeEarlyXTalks || (xTalks[i].second.TDC <= tdcCount)) {
          const double t0 = m_cdcgp->getT0(widx);
          const double ULOfTDC = (t0 - m_lowEdgeOfTimeWindow[board]) * m_invOfTDCBinWidth;
          const double LLOfTDC = (t0 - m_uprEdgeOfTimeWindow[board]) * m_invOfTDCBinWidth;
          if (LLOfTDC <= tdcCount4XTalk && tdcCount4XTalk <= ULOfTDC) {
            const unsigned short status = 0;
            xTalkMap.insert(make_pair(widx, XTalkInfo(tdcCount4XTalk, xTalks[i].second.ADC, xTalks[i].second.TOT, status)));
          }
        }
        //  } else {
        //    cout<<"badwire= " << widx.getICLayer() <<" "<< widx.getIWire() << endl;
      }
    } //end of xtalk loop
  } //end of cdc hit loop

  //Loop over all xtalk hits to creat a new xtalk map with only the fastest hits kept (approx.)
  B2DEBUG(m_debugLevel, "#xtalk  hits: " << xTalkMap.size());
  for (const auto& aHit : xTalkMap) {
    WireID wid = aHit.first;

    iterXTalkMap1 = xTalkMap1.find(wid);
    unsigned short tdcCount = aHit.second.m_tdc;
    unsigned short adcCount = aHit.second.m_adc;
    unsigned short tot      = aHit.second.m_tot;
    unsigned short status   = aHit.second.m_status;

    if (iterXTalkMap1 == xTalkMap1.end()) { // new entry
      xTalkMap1.insert(make_pair(wid, XTalkInfo(tdcCount, adcCount, tot, status)));
      //      B2DEBUG(m_debugLevel, "Creating a new xtalk hit with encoded wire no.: " << wid);
    } else { // not new; check if fastest
      if (tdcCount < iterXTalkMap1->second.m_tdc) {
        iterXTalkMap1->second.m_tdc = tdcCount;
        B2DEBUG(m_debugLevel, "TDC-count of current xtalk: " << tdcCount);
      }
      iterXTalkMap1->second.m_adc += adcCount;
      iterXTalkMap1->second.m_tot += tot; // approx.
    }
  } // end of xtalk loop

  //add xtalk in the same way as the beam bg. overlay
  B2DEBUG(m_debugLevel, "#xtalk1 hits: " << xTalkMap1.size());
  for (const auto& aX : xTalkMap1) {
    bool append = true;
    const unsigned short tdc4Bg = aX.second.m_tdc;
    const unsigned short adc4Bg = aX.second.m_adc;
    const unsigned short tot4Bg = aX.second.m_tot;
    const unsigned short status4Bg = aX.second.m_status;

    for (int iHit = 0; iHit < OriginalNoOfHits; ++iHit) {
      CDCHit& aH = *(m_hits[iHit]);
      if (aH.getID() != aX.first.getEWire()) { //wire id unmatched
        continue;
      } else { //wire id matched
        append = false;
        const unsigned short tdc4Sg = aH.getTDCCount();
        const unsigned short adc4Sg = aH.getADCCount();
        const unsigned short tot4Sg = aH.getTOT();
        //  B2DEBUG(m_debuglevel, "Sg tdc,adc,tot= " << tdc4Sg << " " << adc4Sg << " " << tot4Sg);
        //  B2DEBUG(m_debugLevel, "Bg tdc,adc,tot= " << tdc4Bg << " " << adc4Bg << " " << tot4Bg);

        // If the BG hit is faster than the true hit, the TDC count is replaced, and
        // the relations are removed. ADC counts are summed up.
        if (tdc4Sg < tdc4Bg) {
          aH.setTDCCount(tdc4Bg);
          aH.setStatus(status4Bg);
          auto relSimHits = aH.getRelationsFrom<CDCSimHit>();
          for (int i = relSimHits.size() - 1; i >= 0; --i) {
            relSimHits.remove(i);
          }
          auto relMCParticles = aH.getRelationsFrom<MCParticle>();
          for (int i = relMCParticles.size() - 1; i >= 0; --i) {
            relMCParticles.remove(i);
          }
        }

        aH.setADCCount(adc4Sg + adc4Bg);

        //Set TOT for signal+background case. It is assumed that the start timing
        //of a pulse (input to ADC) is given by the TDC-count. This is an
        //approximation becasue analog (for ADC) and digital (for TDC) parts are
        //different in the front-end electronics.
        unsigned short s1 = tdc4Sg; //start time of 1st pulse
        unsigned short s2 = tdc4Bg; //start time of 2nd pulse
        unsigned short w1 = tot4Sg; //its width
        unsigned short w2 = tot4Bg; //its width
        if (tdc4Sg < tdc4Bg) {
          s1 = tdc4Bg;
          w1 = tot4Bg;
          s2 = tdc4Sg;
          w2 = tot4Sg;
        }
        w1 *= 32;
        w2 *= 32;
        const unsigned short e1 = s1 - w1; //end time of 1st pulse
        const unsigned short e2 = s2 - w2; //end time of 2nd pulse
        //  B2DEBUG(m_debuglevel, "s1,e1,w1,s2,e2,w2= " << s1 << " " << e1 << " " << w1 << " " << s2 << " " << e2 << " " << w2);

        double pulseW = w1 + w2;
        if (e1 <= e2) {
          pulseW = w1;
        } else if (e1 <= s2) {
          pulseW = s1 - e2;
        }

        unsigned short board = m_cdcgp->getBoardID(aX.first);
        aH.setTOT(std::min(std::round(pulseW / 32.), static_cast<double>(m_widthOfTimeWindow[board])));
        B2DEBUG(m_debugLevel, "replaced tdc,adc,tot,wid,status= " << aH.getTDCCount() << " " << aH.getADCCount() << " " << aH.getTOT() <<
                " " << aH.getID() << " " << aH.getStatus());
        break;
      }
    } //end of cdc hit loop

    if (append) {
      m_hits.appendNew(tdc4Bg, adc4Bg, aX.first, status4Bg, tot4Bg);
      B2DEBUG(m_debugLevel, "appended tdc,adc,tot,wid,status= " << tdc4Bg << " " << adc4Bg << " " << tot4Bg << " " << aX.first << " " <<
              status4Bg);
    }
  } //end of x-talk loop
  B2DEBUG(m_debugLevel, "original #hits, #hits= " << OriginalNoOfHits << " " << m_hits.getEntries());
}

// Set FEE parameters (from DB)
void CDCCrossTalkAdderModule::setFEElectronics()
{
  const double el1TrgLatency = m_cdcgp->getMeanT0(); // ns
  B2DEBUG(m_debugLevel, "L1TRGLatency= " << el1TrgLatency);
  const double c = 32. * m_cdcgp->getTdcBinWidth();

  if (!m_fEElectronicsFromDB) B2FATAL("No FEEElectronics dbobject!");
  const CDCFEElectronics& fp = *((*m_fEElectronicsFromDB)[0]);
  int mode = (fp.getBoardID() == -1) ? 1 : 0;
  int iNBoards = static_cast<int>(nBoards);

  //set typical values for all channels first if mode=1
  if (mode == 1) {
    for (int bdi = 1; bdi < iNBoards; ++bdi) {
      m_uprEdgeOfTimeWindow[bdi] = el1TrgLatency - c * (fp.getTrgDelay() + 1);
      if (m_uprEdgeOfTimeWindow[bdi] < 0.) B2FATAL("Upper edge of time window < 0!");
      m_lowEdgeOfTimeWindow[bdi] = m_uprEdgeOfTimeWindow[bdi] - c * (fp.getWidthOfTimeWindow() + 1);
      if (m_lowEdgeOfTimeWindow[bdi] > 0.) B2FATAL("Lower edge of time window > 0!");
      m_widthOfTimeWindow[bdi] = fp.getWidthOfTimeWindow() + 1;
    }
  }

  //ovewrite    values for specific channels if mode=1
  //set typical values for all channels if mode=0
  for (const auto& fpp : (*m_fEElectronicsFromDB)) {
    int bdi = fpp.getBoardID();
    if (mode == 0 && bdi ==  0) continue; //bdi=0 is dummy (not used)
    if (mode == 1 && bdi == -1) continue; //skip typical case
    if (bdi < 0 || bdi >= iNBoards) B2FATAL("Invalid no. of FEE board!");
    m_uprEdgeOfTimeWindow[bdi] = el1TrgLatency - c * (fpp.getTrgDelay() + 1);
    if (m_uprEdgeOfTimeWindow[bdi] < 0.) B2FATAL("Upper edge of time window < 0!");
    m_lowEdgeOfTimeWindow[bdi] = m_uprEdgeOfTimeWindow[bdi] - c * (fpp.getWidthOfTimeWindow() + 1);
    if (m_lowEdgeOfTimeWindow[bdi] > 0.) B2FATAL("Lower edge of time window > 0!");
    m_widthOfTimeWindow[bdi] = fpp.getWidthOfTimeWindow() + 1;
  }

  //debug
  B2DEBUG(m_debugLevel, "mode= " << mode);
  for (int bdi = 1; bdi < iNBoards; ++bdi) {
    B2DEBUG(m_debugLevel, bdi << " " << m_lowEdgeOfTimeWindow[bdi] << " " << m_uprEdgeOfTimeWindow[bdi]);
  }
}
