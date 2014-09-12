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
      m_size(0), m_eventID(0), m_trackID(0),
      m_p(0), m_cosTheta(0),
      m_dedx_avg(0), m_dedx_avg_truncated(0), m_dedx_avg_truncated_err(0) {
      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogL[i] = 0.0;
        m_svdLogL[i] = 0.0;
      }
    }

    /** Add a single hit to the object */
    void addHit(int layer, int wireID, double doca, double enta, double dE, double dx, double dEdx, double cellHeight, double cellHalfWidth) {
      m_size++;
      m_layer.push_back(layer);
      m_wireID.push_back(wireID);
      m_dE.push_back(dE);
      m_dx.push_back(dx);
      m_dEdx.push_back(dEdx);
      m_doca.push_back(doca);
      m_enta.push_back(enta);
      m_cellHeight.push_back(cellHeight);
      m_cellHalfWidth.push_back(cellHalfWidth);
      m_dedx_avg = m_dedx_avg_truncated = m_dedx_avg_truncated_err = 0.0;

      //for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_cdcLogL[i] = 0.0;
        m_svdLogL[i] = 0.0;
      }
    }

    /** Return the event ID */
    double eventID() { return m_eventID; }
    /** Return the track ID */
    double trackID() { return m_trackID; }

    /** Return the (global) layer number */
    int getLayer(int i) { return m_layer[i]; }
    /** Return the (global) wire ID (0-14336) */
    int getWireID(int i) { return m_wireID[i]; }

    /** Return the charge from this hit */
    double getDE(int i) { return m_dE[i]; }
    /** Return the path length through the cell */
    double getDx(int i) { return m_dx[i]; }
    /** Return the dE/dx value for this hit */
    double getDedx(int i) { return m_dEdx[i]; }
    /** Return the distance of closest approach for this hit */
    double getDoca(int i) { return m_doca[i]; }
    /** Return the entrance angle for this hit */
    double getEnta(int i) { return m_enta[i]; }

    /** Return the height of the Cell */
    double getCellHeight(int i) { return m_cellHeight[i]; }
    /** Return the half-width of the Cell */
    double getCellHalfWidth(int i) { return m_cellHalfWidth[i]; }

    /** Return the mean value of dE/dx */
    double getMean() { return m_dedx_avg; }
    /** Return the truncated mean value of dE/dx */
    double getTruncatedMean() { return m_dedx_avg_truncated; }
    /** Return the error on the truncated mean value of dE/dx */
    double getError() { return m_dedx_avg_truncated_err; }

    /** Return the vector of dE/dx values for this object */
    std::vector< double > getDedxList() { return m_dEdx; }

    /** Return the mean value of dE/dx */
    void setMean(double mean) { m_dedx_avg = mean; }
    /** Return the truncated mean value of dE/dx */
    void setTruncatedMean(double mean) { m_dedx_avg_truncated = mean; }
    /** Return the error on the truncated mean value of dE/dx */
    void setError(double err) { m_dedx_avg_truncated_err = err; }

    /** Set the dE/dx value for this hit */
    void setDedx(int i, double dedx) { m_dEdx[i] = dedx; }

    /** Return the number of good hits on this Track */
    int size() { return m_size; }
    /** Return cos(theta) for this Track */
    double getCosTheta() { return m_cosTheta; }

  private:

    int m_size;    /**< the number of good hits on this Track */

    int m_eventID; /**< event in which this Track was found */
    int m_trackID; /**< ID number of the Track */

    // hit level information
    std::vector<int> m_layer;   /**< layer number */
    std::vector<int> m_wireID;   /**< sense wire ID */

    std::vector<double> m_dE;   /**< charge per hit */
    std::vector<double> m_dx;   /**< path length in cell */
    std::vector<double> m_dEdx; /**< charge per path length */
    std::vector<double> m_doca; /**< distance of closest approach */
    std::vector<double> m_enta; /**< entrance angle */

    std::vector<double> m_cellHeight;    /**< height of the cdc cell */
    std::vector<double> m_cellHalfWidth; /**< half-width of the cdc cell */

    // track level information
    double m_p; /**< momentum at point of closest approach to origin */
    double m_cosTheta; /**< cos(theta) for the track */

    double m_dedx_avg;               /**< dE/dx averaged */
    double m_dedx_avg_truncated;     /**< dE/dx averaged, truncated mean */
    double m_dedx_avg_truncated_err; /**< standard deviation of m_dedx_avg_truncated */

    float m_cdcLogL[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */
    float m_svdLogL[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    ClassDef(DedxCell, 9); /**< Debug output for DedxCellPID module. */
  };
}
#endif
