/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett, Christian Pulvermacher
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDDEDXTRACK_H
#define VXDDEDXTRACK_H

#include <reconstruction/dataobjects/DedxConstants.h>

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <TVector3.h>

#include <vector>

namespace Belle2 {
  /** Debug output for VXDDedxPID module.
   *
   * Contains information of individual hits belonging to a track
   */
  class VXDDedxTrack : public RelationsObject {
    friend class VXDDedxPIDModule;

  public:

    /** Default constructor */
    VXDDedxTrack() :
      RelationsObject(),
      m_eventID(0), m_trackID(0),
      m_p(0), m_cosTheta(0), m_charge(0),
      m_length(0.0), m_nHits(0), m_nHitsUsed(0),
      m_pdg(0), m_mother_pdg(0), m_p_true(0)
    {
      //for all detectors
      for (int i = 0; i <= Dedx::c_SVD; i++)
        m_dedx_avg[i] = m_dedx_avg_truncated[i] = m_dedx_avg_truncated_err[i] = 0.0;

      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++)
        m_vxdLogl[i] = 0.0;
    }

    /** Add a single hit to the object */
    void addHit(int sid, int layer, int adcCount, double dx, double dEdx)
    {
      m_sensorID.push_back(sid);
      m_layer.push_back(layer);
      m_adcCount.push_back(adcCount);
      m_dx.push_back(dx);
      m_dEdx.push_back(dEdx);
    }

    /** add a dE/dx value */
    void addDedx(int layer, double distance, double dedxValue)
    {
      dedxLayer.push_back(layer);
      dist.push_back(distance);
      dedx.push_back(dedxValue);
      m_length += distance;
    }


    /** Get average dE/dx for given detector. */
    double getDedx(Const::EDetector detector) const
    {
      int iDet = (int)(detector - Const::PXD);
      if (iDet < 0 or iDet > Dedx::c_SVD)
        return 0.0;
      return m_dedx_avg[iDet];
    }

    /** Get truncated average dE/dx for given detector. */
    double getDedxTruncated(Const::EDetector detector) const
    {
      int iDet = (int)(detector - Const::PXD);
      if (iDet < 0 or iDet > Dedx::c_SVD)
        return 0.0;
      return m_dedx_avg_truncated[iDet];
    }

    /** Get truncated average dE/dx for given detector. */
    double getDedxTruncatedErr(Const::EDetector detector) const
    {
      int iDet = (int)(detector - Const::PXD);
      if (iDet < 0 or iDet > Dedx::c_SVD)
        return 0.0;
      return m_dedx_avg_truncated_err[iDet];
    }

    /** Return the event ID */
    double eventID() const { return m_eventID; }
    /** Return the track ID */
    double trackID() const { return m_trackID; }
    /** Return cos(theta) for this Track */
    double getCosTheta() const { return m_cosTheta; }

    /** Return the momentum */
    double getMomentum() const { return m_p; }
    /** Return the number of hits on the track */
    double size() const { return m_nHits; }
    /** Return the number of hits used in truncated mean */
    double getNHitsUsed() const { return m_nHitsUsed; }

    /** Return the (global) layer number */
    int getLayer(int i) const { return m_layer[i]; }
    /** Return the sensor ID for this hit */
    int getSensorID(int i) const { return m_sensorID[i]; }
    /** Return the adcCount for this hit */
    int getADCCount(int i) const { return m_adcCount[i]; }
    /** Return the path length through the cell */
    double getDx(int i) const { return m_dx[i]; }
    /** Return the dE/dx value for this hit */
    double getDedx(int i) const { return m_dEdx[i]; }

    /** Return the vector of dE/dx values for this object */
    std::vector< double > getDedxList() const { return m_dEdx; }

  private:

    int m_eventID; /**< event in which this Track was found */
    int m_trackID; /**< ID number of the Track */

    // hit level information
    std::vector<int> m_layer;   /**< layer number */
    std::vector<int> m_sensorID; /**< unique sensor ID */
    std::vector<int> m_adcCount;   /**< adcCount per hit */
    std::vector<double> m_dx;   /**< path length in cell */
    std::vector<double> m_dEdx; /**< charge per path length */

    // one entry per layer (just don't mix with the hit arrays)
    std::vector<double> dedx; /**< extracted dE/dx (arb. units, detector dependent) */
    std::vector<double> dist; /**< distance flown through active medium in current segment */
    std::vector<double> dedxLayer; /**< layer id corresponding to dedx */

    // track level information
    double m_p; /**< momentum at the IP */
    double m_cosTheta; /**< cos(theta) for the track */
    short m_charge; /**< particle charge from tracking (+1 or -1) */

    double m_length; /**< total distance travelled by the track */
    short m_nHits; /**< number of hits on this track */
    short m_nHitsUsed; /**< number of hits on this track used in truncated mean */

    double m_pdg; /**< MC PID */
    double m_mother_pdg; /**< MC PID of mother particle */
    double m_p_true; /**< MC true momentum */

    double m_dedx_avg[2];               /**< dE/dx averaged */
    double m_dedx_avg_truncated[2];     /**< dE/dx averaged, truncated mean */
    double m_dedx_avg_truncated_err[2]; /**< standard deviation of m_dedx_avg_truncated */

    double m_vxdLogl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    ClassDef(VXDDedxTrack, 1); /**< Debug output for VXDDedxPID module. */
  };
}
#endif
