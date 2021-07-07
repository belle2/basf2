/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

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
          << "averageRate/F:"
          << "superLayerHitRate[" << f_nSuperLayer << "]/F:"
          << "layerHitRate[" << f_nLayer << "]/F:"
          << "layerPhiHitRate[" << f_nLayer << "][" << f_nPhiDivision << "]/F:"
          << "timeWindowForSmallCell/I:"
          << "timeWindowForNormalCell/I:"
          << "nActiveWireInTotal/I:"
          << "nActiveWireInSuperLayer[" << f_nSuperLayer << "]/I:"
          << "nActiveWireInLayer[" << f_nLayer << "]/I:"
          << "nActiveWireInLayerPhi[" << f_nLayer << "][" << f_nPhiDivision << "]/I:"
          << "numEvents/I:"
          << "valid/O";
      tree->Branch("cdc", &m_rates, leafList.str().c_str());

      if (m_enableBadWireTreatment) {
        countActiveWires();
      } else {
        countActiveWires_countAll();
      }
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
      if (m_enableBackgroundHitFilter) {
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

      CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      ///// getter functions of CDCHit:
      /////   unsigned short getID() /* encoded wire ID*/
      /////   unsigned short CDCHit::getICLayer() /* 0-55 */
      /////   unsigned short CDCHit::getISuperLayer() /* 0-8 */
      /////   short          CDCHit::getTDCCount()
      /////   unsigned short CDCHit::getADCCount() /* integrated charge over the cell */
      /////   unsigned short CDCHit::getTOT() /* time over threshold */
      for (CDCHit& hit : m_digits) {
        const WireID wireID(hit.getID());
        if (m_enableBadWireTreatment && geometryPar.isBadWire(wireID))
          continue;

        const unsigned short iSuperLayer  = hit.getISuperLayer();
        const unsigned short iLayer       = hit.getICLayer();
        const unsigned short iWireInLayer = hit.getIWire();
        const unsigned short iPhiBin      = getIPhiBin(iSuperLayer, iWireInLayer);
        const unsigned short adc = hit.getADCCount();
        const short tdc          = hit.getTDCCount();

        if (m_enableBackgroundHitFilter) {
          if (CDCHitToBackgroundFlag[&hit]) {
            if (m_enableMarkBackgroundHit) {
              unsigned short newStatus = (hit.getStatus() | 0x100);
              hit.setStatus(newStatus);
            }
            continue;
          }
        } else {
          if (iSuperLayer == 0 && adc < 15)
            continue;
          if (iSuperLayer != 0 && adc < 18)
            continue;
        }

        if (not isInTimeWindow(iSuperLayer, tdc))
          continue;


        rates.averageRate += 1;
        rates.superLayerHitRate[iSuperLayer] += 1;
        rates.layerHitRate[iLayer] += 1;
        rates.layerPhiHitRate[iLayer][iPhiBin] += 1;
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;
    }

    void CDCHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      m_rates.timeWindowForSmallCell  = m_timeWindowUpperEdge_smallCell  - m_timeWindowLowerEdge_smallCell;
      m_rates.timeWindowForNormalCell = m_timeWindowUpperEdge_normalCell - m_timeWindowLowerEdge_normalCell;

      // normalize : hit rate in the time stamp in kHz
      m_rates.normalize();

      /////  / nActiveWire => hit rate [kHz] per wire
      ///// average hit rate per wire [kHz] in all wires
      if (m_nActiveWireInTotal == 0) {
        m_rates.averageRate = 0;
      } else {
        m_rates.averageRate /= m_nActiveWireInTotal;
      }
      ///// average hit rate per wire [kHz] in SulerLayer
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL)
        if (m_nActiveWireInSuperLayer[iSL] == 0) {
          m_rates.superLayerHitRate[iSL] = 0;
        } else {
          m_rates.superLayerHitRate[iSL] /= m_nActiveWireInSuperLayer[iSL];
        }
      ///// average hit rate per wire [kHz] in Layer
      for (int iLayer = 0 ; iLayer < f_nLayer ; ++iLayer)
        if (m_nActiveWireInLayer[iLayer] == 0) {
          m_rates.layerHitRate[iLayer] = 0;
        } else {
          m_rates.layerHitRate[iLayer] /= m_nActiveWireInLayer[iLayer];
        }
      ///// average hit rate per wire [kHz] in phi in Layer
      for (int iLayer = 0 ; iLayer < f_nLayer ; ++iLayer)
        for (int iPhi = 0 ; iPhi < f_nPhiDivision ; ++iPhi)
          if (m_nActiveWireInLayerPhi[iLayer][iPhi] == 0) {
            m_rates.layerPhiHitRate[iLayer][iPhi] = 0;
          } else {
            m_rates.layerPhiHitRate[iLayer][iPhi] /= m_nActiveWireInLayerPhi[iLayer][iPhi];
          }

      /// copy nActiveWire
      m_rates.nActiveWireInTotal = m_nActiveWireInTotal;
      for (int i = 0 ; i < f_nSuperLayer ; ++i)
        m_rates.nActiveWireInSuperLayer[i] = m_nActiveWireInSuperLayer[i];
      for (int i = 0 ; i < f_nLayer ; ++i)
        m_rates.nActiveWireInLayer[i] = m_nActiveWireInLayer[i];
      for (int i = 0 ; i < f_nLayer ; ++i)
        for (int j = 0 ; j < f_nPhiDivision ; ++j)
          m_rates.nActiveWireInLayerPhi[i][j] = m_nActiveWireInLayerPhi[i][j];
    }


    void CDCHitRateCounter::countActiveWires_countAll()
    {
      static const unsigned short nlayer_in_SL[f_nSuperLayer] = { 8, 6, 6,
                                                                  6, 6, 6,
                                                                  6, 6, 6
                                                                };
      static const unsigned short nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                                    224, 256, 288,
                                                                    320, 352, 384
                                                                  };

      m_nActiveWireInTotal = 0; ///// initialize
      int contLayerID = 0;//// continuous layer numbering
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        m_nActiveWireInSuperLayer[iSL] = 0; //// initialize
        for (int i = 0 ; i < nlayer_in_SL[iSL] ; ++i) {
          m_nActiveWireInTotal             += nwire_in_layer[iSL];
          m_nActiveWireInSuperLayer[iSL]   += nwire_in_layer[iSL];
          m_nActiveWireInLayer[contLayerID] = nwire_in_layer[iSL];
          for (int j = 0 ; j < f_nPhiDivision ; ++j)
            m_nActiveWireInLayerPhi[contLayerID][j] = nwire_in_layer[iSL] / f_nPhiDivision;
          ++contLayerID;
        }
      }
    }



    void CDCHitRateCounter::countActiveWires()
    {
      static const unsigned short nlayer_in_SL[f_nSuperLayer] = { 8, 6, 6,
                                                                  6, 6, 6,
                                                                  6, 6, 6
                                                                };
      static const unsigned short nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                                    224, 256, 288,
                                                                    320, 352, 384
                                                                  };

      CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      m_nActiveWireInTotal = 0; ///// initialize
      int contLayerID = 0;//// continuous layer numbering

      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        m_nActiveWireInSuperLayer[iSL] = 0; //// initialize
        for (int iL = 0 ; iL < nlayer_in_SL[iSL] ; ++iL) { //// iL: layerID in SL
          m_nActiveWireInLayer[contLayerID] = 0;//// initialize
          for (int iPhi = 0 ; iPhi < f_nPhiDivision ; ++iPhi)
            m_nActiveWireInLayerPhi[contLayerID][iPhi] = 0;//// initialize

          for (int i = 0 ; i < nwire_in_layer[iSL] ; ++i) {
            WireID wireID(iSL, iL, i);
            if (not geometryPar.isBadWire(wireID)) {
              ++m_nActiveWireInLayerPhi[contLayerID][getIPhiBin(iSL, i)];
              ++m_nActiveWireInLayer[contLayerID];
            }
          }/// end i loop
          m_nActiveWireInSuperLayer[iSL] += m_nActiveWireInLayer[contLayerID];
          ++contLayerID;
        }/// end iL loop
        m_nActiveWireInTotal += m_nActiveWireInSuperLayer[iSL];
      }/// end iSL loop


      //B2INFO("CDC, # of Active wires / # of total wires");
      std::cout << "CDC, # of Active wires / # of total wires" << std::endl;
      int contLayerID_2 = 0;
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        for (int iL = 0 ; iL < nlayer_in_SL[iSL] ; ++iL) {
          //B2INFO("Layer "<< contLayerID_2 << ": "
          std::cout << "Layer " << contLayerID_2 << ": "
                    << m_nActiveWireInLayer[contLayerID_2] << " / "
                    //<< nwire_in_layer[iSL] );
                    << nwire_in_layer[iSL] << std::endl;
          ++contLayerID_2;
        }
      }

    }


    unsigned short CDCHitRateCounter::getIPhiBin(unsigned short iSL, unsigned short iWireInLayer)
    {
      static const unsigned short nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                                    224, 256, 288,
                                                                    320, 352, 384
                                                                  };
      return iWireInLayer / (nwire_in_layer[iSL] / f_nPhiDivision);
    }


  } // Background namespace
} // Belle2 namespace

