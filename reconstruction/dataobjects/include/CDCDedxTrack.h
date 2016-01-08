/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCDEDXTRACK_H
#define CDCDEDXTRACK_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <TVector3.h>

#include <vector>

namespace Belle2 {
  /** Debug output for CDCDedxPID module.
   *
   * Contains information of individual hits belonging to a track
   */
  class CDCDedxTrack : public RelationsObject {
    friend class CDCDedxPIDModule;
    friend class CDCDedxScanModule;
    friend class DedxCorrectionModule;

  public:

    /** Default constructor */
    CDCDedxTrack() :
      RelationsObject(),
      m_eventID(0), m_trackID(0),
      m_p(0), m_p_cdc(0), m_cosTheta(0), m_charge(0),
      m_length(0.0), m_nHits(0), m_nHitsUsed(0),
      m_pdg(0), m_mother_pdg(0), m_p_true(0)
    {
      m_dedx_avg = m_dedx_avg_truncated = m_dedx_avg_truncated_err = 0.0;

      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcChi[i] = -999.;
        m_cdcLogl[i] = 0.0;
      }
    }

    /** Add a single hit to the object */
    void addHit(int wire, int layer, double doca, double enta, int adcCount, double dE, double dx, double dEdx, double cellHeight,
                double cellHalfWidth, int driftT, double driftD, double driftDRes)
    {
      m_wire.push_back(wire);
      m_layer.push_back(layer);
      m_doca.push_back(doca);
      m_enta.push_back(enta);
      m_adcCount.push_back(adcCount);
      m_dE.push_back(dE);
      m_dx.push_back(dx);
      m_dEdx.push_back(dEdx);
      m_cellHeight.push_back(cellHeight);
      m_cellHalfWidth.push_back(cellHalfWidth);
      m_driftT.push_back(driftT);
      m_driftD.push_back(driftD);
      m_driftDRes.push_back(driftDRes);
    }

    /** add a dE/dx value */
    void addDedx(int layer, double distance, double dedxValue)
    {
      dedxLayer.push_back(layer);
      dist.push_back(distance);
      dedx.push_back(dedxValue);
      m_length += distance;
    }

    /** Return the event ID */
    double eventID() const { return m_eventID; }
    /** Return the track ID */
    double trackID() const { return m_trackID; }

    /** Get truth information. */
    double getPDG() const { return m_pdg; }

    /** Get average dE/dx. */
    double getDedx() const { return m_dedx_avg; }
    /** Get truncated average dE/dx. */
    double getTruncatedMean() const { return m_dedx_avg_truncated; }
    /** Get truncated average dE/dx. */
    double getError() const { return m_dedx_avg_truncated_err; }

    /** Return the vector of dE/dx values for this object */
    std::vector< double > getDedxList() const { return m_dEdx; }

    /** Return the momentum in the CDC */
    double getMomentum() const { return m_p_cdc; }
    /** Return the number of hits on this Track */
    int size() const { return m_nHits; }
    /** Return the number of hits used in truncated mean */
    double getNHitsUsed() const { return m_nHitsUsed; }
    /** Return cos(theta) for this Track */
    double getCosTheta() const { return m_cosTheta; }

    /** Return the (global) layer number */
    int getLayer(int i) const { return m_layer[i]; }
    /** Return the sensor ID for this hit: wire number for CDC (0-14336) */
    int getWire(int i) const { return m_wire[i]; }

    /** Return the adcCount for this hit */
    int getADCCount(int i) const { return m_adcCount[i]; }
    /** Return the charge from this hit */
    double getDE(int i) const { return m_dE[i]; }
    /** Return the path length through the cell */
    double getDx(int i) const { return m_dx[i]; }
    /** Return the dE/dx value for this hit */
    double getDedx(int i) const { return m_dEdx[i]; }
    /** Return the distance of closest approach for this hit */
    double getDoca(int i) const { return m_doca[i]; }
    /** Return the entrance angle for this hit */
    double getEnta(int i) const { return m_enta[i]; }

    /** Return the height of the Cell */
    double getCellHeight(int i) const { return m_cellHeight[i]; }
    /** Return the half-width of the Cell */
    double getCellHalfWidth(int i) const { return m_cellHalfWidth[i]; }

    /** Return the drift time for a hit */
    int getDriftT(int i) const { return m_driftT[i]; }
    /** Return the drift distance for a hit */
    double getDriftD(int i) const { return m_driftD[i]; }
    /** Return the drift distance resolution for a hit */
    double getDriftDRes(int i) const { return m_driftDRes[i]; }

    /** Get average dE/dx. */
    void setMean(double mean) { m_dedx_avg = mean; }
    /** Get truncated average dE/dx. */
    void setTruncatedMean(double mean) { m_dedx_avg_truncated = mean; }
    /** Get truncated average dE/dx. */
    void setError(double error) { m_dedx_avg_truncated_err = error; }

    /** Set the dE/dx value for this hit */
    void setDedx(int i, double dedx) { m_dEdx[i] = dedx; }

  private:

    int m_eventID; /**< event in which this Track was found */
    int m_trackID; /**< ID number of the Track */

    // hit level information
    std::vector<int> m_layer;   /**< layer number */
    std::vector<int> m_wire; /**< wire ID in the CDC */
    std::vector<int> m_adcCount;   /**< adcCount per hit */
    std::vector<double> m_dE;   /**< charge per hit */
    std::vector<double> m_dx;   /**< path length in cell */
    std::vector<double> m_dEdx; /**< charge per path length */
    std::vector<double> m_doca; /**< distance of closest approach */
    std::vector<double> m_enta; /**< entrance angle */

    std::vector<double> m_cellHeight;    /**< height of the cdc cell */
    std::vector<double> m_cellHalfWidth; /**< half-width of the cdc cell */

    std::vector<int> m_driftT; /**< drift time for each hit */
    std::vector<double> m_driftD; /**< drift distance for each hit */
    std::vector<double> m_driftDRes; /**< drift distance resolution for each hit */

    // one entry per layer (just don't mix with the hit arrays)
    std::vector<double> dedx; /**< extracted dE/dx (arb. units, detector dependent) */
    std::vector<double> dist; /**< distance flown through active medium in current segment */
    std::vector<double> dedxLayer; /**< layer id corresponding to dedx */

    // track level information
    double m_p; /**< momentum at the IP */
    double m_p_cdc; /**< momentum at the inner layer of the CDC */
    double m_cosTheta; /**< cos(theta) for the track */
    short m_charge; /**< particle charge from tracking (+1 or -1) */

    double m_length; /**< total distance travelled by the track */
    short m_nHits; /**< number of hits on this track */
    short m_nHitsUsed; /**< number of hits on this track used in truncated mean */

    double m_pdg; /**< MC PID */
    double m_mother_pdg; /**< MC PID of mother particle */
    double m_p_true; /**< MC true momentum */

    double m_dedx_avg;               /**< dE/dx averaged */
    double m_dedx_avg_truncated;     /**< dE/dx averaged, truncated mean */
    double m_dedx_avg_truncated_err; /**< standard deviation of m_dedx_avg_truncated */

    double m_predmean[Const::ChargedStable::c_SetSize];
    double m_predres[Const::ChargedStable::c_SetSize];

    double m_cdcChi[Const::ChargedStable::c_SetSize]; /**< chi values for each particle type */
    double m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    ClassDef(CDCDedxTrack, 2); /**< Debug output for CDCDedxPID module. */
  };
}
#endif
