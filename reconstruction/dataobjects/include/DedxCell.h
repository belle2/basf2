/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef DEDXCELL_H
#define DEDXCELL_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <TVector3.h>

#include <vector>

namespace Belle2 {
  /** Debug output for DedxCellPID module.
   *
   * Contains information of individual hits belonging to a track
   */
  class DedxCell : public RelationsObject {
    friend class DedxCellPIDModule;
    friend class DedxScanModule;
    friend class DedxCorrectionModule;

  public:

    /** Default constructor */
    DedxCell() :
      RelationsObject(),
      m_size(0), m_vxd(0), m_eventID(0), m_trackID(0),
      m_p(0), m_p_cdc(0), m_cosTheta(0), m_charge(0),
      m_nHits(0), m_nHitsUsed(0),
      m_pdg(0), m_mother_pdg(0), m_p_true(0)
    {
      //for all particles
      //for all detectors
      for (int i = 0; i < Dedx::c_num_detectors; i++)
        m_dedx_avg[i] = m_dedx_avg_truncated[i] = m_dedx_avg_truncated_err[i] = 0.0;

      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogl[i] = 0.0;
        m_svdLogl[i] = 0.0;
      }
    }

    /** Add a single hit to the object */
    void addHit(int sid, int layer, int wireID, double doca, double enta, int adcCount, double dE, double dx, double dEdx,
                double cellHeight, double cellHalfWidth, int driftT, double driftD, double driftDRes)
    {
      if (sid == Dedx::c_CDC) m_size++;
      else m_vxd++;
      m_sensorID.push_back(sid);
      m_layer.push_back(layer);
      m_wireID.push_back(wireID);
      m_adcCount.push_back(adcCount);
      m_dE.push_back(dE);
      m_dx.push_back(dx);
      m_dEdx.push_back(dEdx);
      m_doca.push_back(doca);
      m_enta.push_back(enta);
      m_cellHeight.push_back(cellHeight);
      m_cellHalfWidth.push_back(cellHalfWidth);
      m_driftT.push_back(driftT);
      m_driftD.push_back(driftD);
      m_driftDRes.push_back(driftDRes);
    }

    /** add a dE/dx value */
    void addDedx(int layer, float dedxValue)
    {
      dedxLayer.push_back(layer);
      dedx.push_back(dedxValue);
    }

    /** Return the event ID */
    double eventID() const { return m_eventID; }
    /** Return the track ID */
    double trackID() const { return m_trackID; }

    /** Return the momentum in the CDC */
    double getMomentum() const { return m_p_cdc; }
    /** Return the number of hits on the track */
    double getNHits() const { return m_nHits; }
    /** Return the number of hits used in truncated mean */
    double getNHitsUsed() const { return m_nHitsUsed; }


    /** Return the (global) layer number */
    int getLayer(int i) const { return m_layer[i]; }
    /** Return the (global) wire ID (0-14336) */
    int getWireID(int i) const { return m_wireID[i]; }

    /** Return the sensor ID for this hit */
    int getSensorID(int i) const { return m_sensorID[i]; }
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

    /** Return the mean value of dE/dx */
    double getMean() const { return m_dedx_avg[2]; }
    /** Return the truncated mean value of dE/dx */
    double getTruncatedMean() const { return m_dedx_avg_truncated[2]; }
    /** Return the error on the truncated mean value of dE/dx */
    double getError() const { return m_dedx_avg_truncated_err[2]; }

    /** Return the number of good hits on this Track */
    int size() const { return m_size; }
    /** Return cos(theta) for this Track */
    double getCosTheta() const { return m_cosTheta; }

    /** Return the vector of dE/dx values for this object */
    std::vector< double > getDedxList() const { return m_dEdx; }

    /** Return the mean value of dE/dx */
    void setMean(int sid, double mean) { m_dedx_avg[sid] = mean; }
    /** Return the truncated mean value of dE/dx */
    void setTruncatedMean(int sid, double mean) { m_dedx_avg_truncated[sid] = mean; }
    /** Return the error on the truncated mean value of dE/dx */
    void setError(int sid, double err) { m_dedx_avg_truncated_err[sid] = err; }

    /** Set the dE/dx value for this hit */
    void setDedx(int i, double dedx) { m_dEdx[i] = dedx; }

  private:

    int m_size;    /**< the number of hits in the CDC */
    int m_vxd;    /**< the number of hits in the VXD */

    int m_eventID; /**< event in which this Track was found */
    int m_trackID; /**< ID number of the Track */

    // hit level information
    std::vector<int> m_layer;   /**< layer number */
    std::vector<int> m_wireID;   /**< sense wire ID */

    std::vector<int> m_sensorID; /**< unique sensor ID (wire ID in CDC) */
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
    std::vector<double> dedxLayer; /**< layer id corresponding to dedx */

    // track level information
    double m_p; /**< momentum at the IP */
    double m_p_cdc; /**< momentum at the inner layer of the CDC */
    double m_cosTheta; /**< cos(theta) for the track */
    short m_charge; /**< particle charge from tracking (+1 or -1) */
    short m_nHits; /**< number of hits on this track */
    short m_nHitsUsed; /**< number of hits on this track used in truncated mean */

    double m_pdg; /**< MC PID */
    double m_mother_pdg; /**< MC PID of mother particle */
    double m_p_true; /**< MC true momentum */

    double m_dedx_avg[Dedx::c_num_detectors];               /**< dE/dx averaged */
    double m_dedx_avg_truncated[Dedx::c_num_detectors];     /**< dE/dx averaged, truncated mean */
    double m_dedx_avg_truncated_err[Dedx::c_num_detectors]; /**< standard deviation of m_dedx_avg_truncated */

    float m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */
    float m_svdLogl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    ClassDef(DedxCell, 1); /**< Debug output for DedxCellPID module. */
  };
}
#endif
