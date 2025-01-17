/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/DedxConstants.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

#include <vector>
#include <TH2F.h>

namespace Belle2 {

  /**
   * Debug output for VXDDedxPID module.
   *
   * Contains information of individual hits belonging to a track.
   */
  class VXDDedxTrack : public RelationsObject {
    friend class VXDDedxPIDModule;

  public:

    /** Default constructor */
    VXDDedxTrack() = default;

    /** Add a single hit to the object */
    void addHit(int sid, int layer, int adcCount, double dx, double dEdx);

    /** add dE/dx information for a VXD layer */
    void addDedx(int layer, double distance, double dedxValue);

    /** Get dE/dx truncated mean for given detector */
    double getDedx(Const::EDetector detector) const;

    /** Get the error on the dE/dx truncated mean for given detector */
    double getDedxError(Const::EDetector detector) const;

    /** Get the dE/dx mean for given detector */
    double getDedxMean(Const::EDetector detector) const;

    /** Return the event ID */
    int eventID() const { return m_eventID; }

    /** Return the track ID */
    int trackID() const { return m_trackID; }

    /** Return cos(theta) for this yrack */
    double getCosTheta() const { return m_cosTheta; }

    /** Return the momentum valid at the IP */
    double getMomentum() const { return m_p; }

    /** Return the MC truemomentum valid at the IP */
    double getTrueMomentum() const { return m_pTrue; }

    /** Return the number of hits for this track */
    int size() const { return m_nHits; }

    /** Return the number of hits used to determine the truncated mean */
    int getNHitsUsed() const { return m_nHitsUsed; }

    /** Return the (global) layer number */
    int getLayer(int i) const { return m_layer[i]; }

    /** Return the sensor ID for this hit */
    int getSensorID(int i) const { return m_sensorID[i]; }

    /** Return the adcCount for this hit */
    int getADCCount(int i) const { return m_adcCount[i]; }

    /** Return the path length through the layer */
    double getDx(int i) const { return m_dx[i]; }

    /** Return the dE/dx value for this hit */
    double getDedxOfHit(int i) const { return m_dEdx[i]; }

    /** Return the vector of dE/dx values for this track */
    const std::vector<double>& getDedxList() const { return m_dEdx; }

    /**
     * Clear log likelihoods (set to zero) and reset the counter of added log likelihood values
     */
    void clearLogLikelihoods();

    /**
     * Calculate and add log likelihoods to array m_vxdLogl.
     * @param PDFs PDF's
     * @param detector detector (PXD or SVD)
     * @param truncated if true, use dE/dx truncated mean, otherwise use dE/dx measured in layers
     */
    void addLogLikelihoods(const std::vector<const TH2F*>& PDFs, Dedx::Detector detector, bool truncated);

    /**
     * Are log likelihoods available?
     * @return true if available
     */
    bool areLogLikelihoodsAvailable() const {return m_numAdded > 0;}

    /**
     * Get the array of log likelihoods.
     * @return array of log likelihoods of the size of ChargedStable particle set
     */
    const double* getLogLikelihoods() const {return m_vxdLogl;}

  private:

    /**
     * Calculate and add log likelihoods to array m_vxdLogl.
     * @param PDFs PDF's
     * @param dedxValue value of dE/dx
     * @param minPDFValue minimal PDF value (used when bins are found empty)
     */
    void addLogLikelihoods(const std::vector<const TH2F*>& PDFs, double dedxValue, double minPDFValue);

    int m_eventID = 0; /**< event in which this Track was found */
    int m_trackID = 0; /**< ID number of the Track */

    // hit level information
    std::vector<int> m_layer;    /**< VXD layer number */
    std::vector<int> m_sensorID; /**< unique sensor ID */
    std::vector<int> m_adcCount; /**< adcCount per hit */
    std::vector<double> m_dx;    /**< path length in layer */
    std::vector<double> m_dEdx;  /**< charge per path length */

    // one entry per layer (just don't mix with the hit arrays)
    std::vector<double> dedx;      /**< extracted dE/dx (arb. units, detector dependent) */
    std::vector<double> dist;      /**< distance flown through active medium in current segment */
    std::vector<double> dedxLayer; /**< layer id corresponding to dE/dx measurement */

    // track level information
    double m_p = 0;        /**< momentum at the IP */
    double m_cosTheta = 0; /**< cos(theta) for the track */
    short m_charge = 0;    /**< particle charge from tracking (+1 or -1) */

    double m_length = 0;   /**< total distance travelled by the track */
    short m_nHits = 0;     /**< number of hits on this track */
    short m_nHitsUsed = 0; /**< number of hits on this track used in the truncated mean */

    double m_pdg = 0;       /**< MC PID */
    double m_motherPDG = 0; /**< MC PID of mother particle */
    double m_pTrue = 0;     /**< MC true momentum */

    double m_dedxAvg[2] = {0};            /**< dE/dx mean value per track */
    double m_dedxAvgTruncated[2] = {0};   /**< dE/dx truncated mean per track */
    double m_dedxAvgTruncatedErr[2] = {0}; /**< standard deviation of m_dedxAvgTruncated */

    double m_vxdLogl[Const::ChargedStable::c_SetSize] = {0}; /**< log likelihood for each particle */
    int m_numAdded = 0; /**< counter of added log likelihood values */

    ClassDef(VXDDedxTrack, 3); /**< class version for ROOT streamer */
  };
}
