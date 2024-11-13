/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/EventLevelTriggerTimeInfo.h>
#include <reconstruction/dataobjects/CDCDedxHit.h>
#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <reconstruction/dataobjects/CDCDedxLikelihood.h>

#include <framework/database/DBObjPtr.h>
#include <reconstruction/dbobjects/CDCDedxScaleFactor.h>
#include <reconstruction/dbobjects/CDCDedxWireGain.h>
#include <reconstruction/dbobjects/CDCDedxRunGain.h>
#include <reconstruction/dbobjects/CDCDedxCosineCor.h>
#include <reconstruction/dbobjects/CDCDedx2DCell.h>
#include <reconstruction/dbobjects/CDCDedx1DCell.h>
#include <reconstruction/dbobjects/CDCDedxADCNonLinearity.h>
#include <reconstruction/dbobjects/CDCDedxCosineEdge.h>
#include <reconstruction/dbobjects/CDCDedxMeanPars.h>
#include <reconstruction/dbobjects/CDCDedxSigmaPars.h>
#include <reconstruction/dbobjects/CDCDedxHadronCor.h>
#include <reconstruction/dbobjects/CDCDedxInjectionTime.h>

#include <string.h>

namespace Belle2 {

  /**
   * Module that creates PID likelihoods from CDC hit information stored in CDCDedxHits using parameterized means and resolutions.
   */
  class CDCDedxPIDCreatorModule : public Module {

  public:

    /** Default constructor */
    CDCDedxPIDCreatorModule();

    /** Destructor */
    virtual ~CDCDedxPIDCreatorModule();

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each event. All processing of the event takes place in this method. */
    virtual void event() override;

  private:

    /**
     * Helper structure for merging dEdx measurements
     */
    struct DEDX {
      const CDCDedxHit* hit = nullptr; /**< hit */
      int cWire = -1; /**< continuous wire number */
      int cLayer = -1; /**< continuous layer number */
      double dx = 0; /**< track length */
      double dE = 0; /**< energy deposit */
      int nhits = 0; /**< number of merged hits */
      double dxMax = 0; /**< longest track length */

      /**
       * Add measurement on the same wire
       * @param hit_in wire hit
       * @param cWire_in continuous wire number
       * @param cLayer_in continuous layer number
       * @param dx_in track length
       * @param dE_in energy deposit
       */
      void add(const CDCDedxHit& hit_in, int cWire_in, int cLayer_in, double dx_in, double dE_in)
      {
        if (not hit) {
          hit = &hit_in;
          cWire = cWire_in;
          cLayer = cLayer_in;
          dx = dx_in;
          dE = dE_in;
          nhits = 1;
          dxMax = dx;
        } else if (hit_in.getWireID() == hit->getWireID()) {
          dx += dx_in;
          if (hit->getADCCount() != hit_in.getADCCount()) dE += dE_in; // these hits are not piled-up
          nhits++;
          if (dx_in > dxMax) dxMax = dx_in;
        } else {
          B2ERROR("DEDX helper: measurements on different wires cannot be merged");
        }
      }

      /**
       * Add measurement in the same layer
       * @param dedx measurement
       */
      void add(const DEDX& dedx)
      {
        if (cLayer < 0) {
          cWire = dedx.cWire;
          cLayer = dedx.cLayer;
          dx = dedx.dx;
          dE = dedx.dE;
          nhits = dedx.nhits;
          dxMax = dedx.dxMax;
        } else if (dedx.cLayer == cLayer) {
          dx += dedx.dx;
          dE += dedx.dE;
          nhits += dedx.nhits;
          if (dedx.dxMax > dxMax) {
            dxMax = dedx.dxMax;
            cWire = dedx.cWire; // wire ID with longest hit
          }
        } else {
          B2ERROR("DEDX helper: measurements in different layers cannot be merged");
        }
      }
    };

    /** hadron saturation parameterization part 1 */
    double I2D(double cosTheta, double I) const;

    /** hadron saturation parameterization part 2 */
    double D2I(double cosTheta, double D) const;

    /** parameterized beta-gamma curve for predicted means */
    double meanCurve(double x, const double* par, int version) const;

    /** calculate the predicted mean using the parameterized resolution */
    double getMean(double bg) const;

    /** parameterized resolution for predictions */
    double sigmaCurve(double x, const double* par, int version) const;

    /** calculate the predicted resolution using the parameterized resolution */
    double getSigma(double dedx, double nhit, double cos, double timereso) const;

    // module steering parameters
    double m_removeLowest; /**< portion of events with low dE/dx to discard */
    double m_removeHighest; /**< portion of events with high dE/dx to discard */
    bool m_useBackHalfCurlers; /**< whether to use the back half of curlers */
    bool m_enableDebugOutput; /**< option to write out debugging information to CDCDedxTracks */
    std::string m_likelihoodsName; /**< name of collection of PID likelihoods */
    std::string m_dedxTracksName; /**< name of collection of debug output */

    // collections
    StoreArray<Track> m_tracks; /**< collection of tracks */
    StoreArray<CDCDedxHit> m_hits; /**< collection of hits */
    StoreArray<MCParticle> m_mcParticles; /**< collection of MC particles */
    StoreObjPtr<EventLevelTriggerTimeInfo> m_TTDInfo; /**< injection time info */
    StoreArray<CDCDedxLikelihood> m_likelihoods; /**< collection of PID likelihoods */
    StoreArray<CDCDedxTrack> m_dedxTracks; /**< collection of debug output */

    // parameters: calibration constants
    DBObjPtr<CDCDedxScaleFactor> m_DBScaleFactor; /**< Scale factor to make electrons ~1 */
    DBObjPtr<CDCDedxRunGain> m_DBRunGain; /**< Run gain DB object */
    DBObjPtr<CDCDedxInjectionTime> m_DBInjectTime; /**< time gain/reso DB object */
    DBObjPtr<CDCDedxCosineCor> m_DBCosineCor; /**< Electron saturation correction DB object */
    DBObjPtr<CDCDedxCosineEdge> m_DBCosEdgeCor; /**< non-linearly ACD correction DB object */
    DBObjPtr<CDCDedxWireGain> m_DBWireGains; /**< Wire gain DB object */
    DBObjPtr<CDCDedx2DCell> m_DB2DCell; /**< 2D correction DB object */
    DBObjPtr<CDCDedx1DCell> m_DB1DCell; /**< 1D correction DB object */
    DBObjPtr<CDCDedxADCNonLinearity> m_DBNonlADC; /**< non-linearly ACD correction DB object */

    // parameters to determine the predicted means and resolutions, and hadron correction
    DBObjPtr<CDCDedxMeanPars> m_DBMeanPars; /**< dE/dx mean parameters */
    DBObjPtr<CDCDedxSigmaPars> m_DBSigmaPars; /**< dE/dx resolution parameters */
    DBObjPtr<CDCDedxHadronCor> m_DBHadronCor; /**< hadron saturation parameters */

    // other
    int m_nLayerWires[9] = {0}; /**< lookup table for number of wires per superlayer (indexed by superlayer) */

  };

} // Belle2 namespace

