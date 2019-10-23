/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace std;

namespace Belle2 {

  using TrackFindingCDC::StoreWrappedObjPtr;
  using TrackFindingCDC::CDCWireHit;

  namespace Background {

    void CDCHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      //tree->Branch("cdc", &m_rates, "averageRate/F:numEvents/I:valid/O");
      stringstream leafList;
      leafList
          << "layerHitRate[" << f_nLayer << "]/F:"
          << "superLayerHitRate[" << f_nSuperLayer << "]/F:"
          << "averageRate/F:"
          << "numEvents/I:"
          << "valid/O";
      tree->Branch("cdc", &m_rates, leafList.str().c_str());

      countActiveWires();
    }

    void CDCHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void CDCHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      ///// get background flag as a map ( ASIC crosstalk, etc..? )
      std::map<const CDCHit*, bool> CDCHitToBackgroundFlag;
      {
        StoreWrappedObjPtr<std::vector<CDCWireHit>> storeVector("CDCWireHitVector");
        if (not storeVector) {
          B2FATAL("CDCWireHitVector is unaccessible in DataStore."
                  "Need TFCDC_WireHitParameter module before.");
        }
        const std::vector<CDCWireHit>& cdcWireHitVector = *storeVector;
        for (const auto& cdcWireHit : cdcWireHitVector) {
          const CDCHit* cdcHit = cdcWireHit.getHit();
          CDCHitToBackgroundFlag[cdcHit] = cdcWireHit->hasBackgroundFlag(); ///// cdcWireHit.getAutomatonCell().hasBackgroundFlag()
        }
      }

      ///// getter functions of CDCHit:
      /////   unsigned short CDCHit::getICLayer() /* 0-55 */
      /////   unsigned short CDCHit::getISuperLayer() /* 0-8 */
      /////   short          CDCHit::getTDCCount()
      /////   unsigned short CDCHit::getADCCount() /* integrated charge over the cell */
      /////   unsigned short CDCHit::getTOT() /* time over threshold */
      for (const CDCHit& hit : m_digits) {
        const int iLayer         = hit.getICLayer();
        const int iSuperLayer    = hit.getISuperLayer();
        //const unsigned short adc = hit.getADCCount();

        //if (adc < 20) continue;
        if (CDCHitToBackgroundFlag[&hit]) continue;

        rates.layerHitRate[iLayer] += 1;
        rates.superLayerHitRate[iSuperLayer] += 1;
        rates.averageRate += 1;
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;
    }

    void CDCHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize : nhit / nEvent in a second(time stamp) => # of hit in a event
      m_rates.normalize();

      // optionally: convert to MHz, correct for the masked-out channels etc.
      ///// (# of hit in a event) / nActiveWire => occupancy in an event
      m_rates.averageRate /= m_nActiveWireInTotal;
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL)
        m_rates.superLayerHitRate[iSL] /= m_nActiveWireInSuperLayer[iSL];
      for (int iLayer = 0 ; iLayer < f_nLayer ; ++iLayer)
        m_rates.layerHitRate[iLayer] /= m_nActiveWireInLayer[iLayer];

    }


    void CDCHitRateCounter::countActiveWires()
    {
      const int nlayer_in_SL[f_nSuperLayer] = { 8, 6, 6,
                                                6, 6, 6,
                                                6, 6, 6
                                              };
      const int nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                  224, 256, 288,
                                                  320, 352, 384
                                                };

      m_nActiveWireInTotal = 0; ///// initialize
      int contLayerID = 0;//// continuous layer numbering
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        m_nActiveWireInSuperLayer[iSL] = 0; //// initialize
        for (int i = 0 ; i < nlayer_in_SL[iSL] ; ++i) {
          m_nActiveWireInLayer[contLayerID] = nwire_in_layer[iSL];
          m_nActiveWireInSuperLayer[iSL]   += nwire_in_layer[iSL];
          m_nActiveWireInTotal             += nwire_in_layer[iSL];
          ++contLayerID;
        }
      }
    }


  } // Background namespace
} // Belle2 namespace

