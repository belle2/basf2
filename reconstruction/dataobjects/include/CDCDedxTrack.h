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
      m_track(0), m_charge(0), m_cosTheta(0), m_p(0), m_p_cdc(0),
      m_pdg(-999), m_mcmass(0), m_mother_pdg(0), m_p_true(0), m_length(0.0),
      m_scale(0), m_cosCor(0), m_runGain(0),
      m_lNHitsUsed(0)
    {
      m_dedx = m_dedx_avg = m_dedx_avg_truncated = m_dedx_avg_truncated_err = 0.0;

      // set default values for all particles
      for (unsigned int i = 0; i < Const::ChargedStable::c_SetSize; i++) {
        m_predmean[i] = 0.0;
        m_predres[i] = 0.0;
        m_cdcChi[i] = -999.;
        m_cdcLogl[i] = 0.0;
      }
    }

    /** add dE/dx information for a CDC layer */
    void addDedx(int nhitscombined, int wirelongesthit, int layer, double distance, double dedxValue)
    {
      m_lNHitsCombined.push_back(nhitscombined);
      m_lWireLongestHit.push_back(wirelongesthit);
      m_lLayer.push_back(layer);
      m_lPath.push_back(distance);
      m_lDedx.push_back(dedxValue);

      m_length += distance;
    }

    /** Add a single hit to the object */
    void addHit(int lwire, int wire, int layer, double doca, double enta, int adcCount, double dE, double path, double dedx,
                double cellHeight, double cellHalfWidth, int driftT, double driftD, double driftDRes, double wiregain, double twodcor,
                double onedcor)
    {
      m_hLWire.push_back(lwire);
      m_hWire.push_back(wire);
      m_hLayer.push_back(layer);
      m_hPath.push_back(path);
      m_hDedx.push_back(dedx);
      m_hADCCount.push_back(adcCount);
      m_hDoca.push_back(doca);
      m_hEnta.push_back(enta);
      m_hDriftT.push_back(driftT);

      m_hdE.push_back(dE);
      m_hCellHeight.push_back(cellHeight);
      m_hCellHalfWidth.push_back(cellHalfWidth);
      m_hDriftD.push_back(driftD);
      m_hDriftDRes.push_back(driftDRes);

      m_hWireGain.push_back(wiregain);
      m_hTwodCor.push_back(twodcor);
      m_hOnedCor.push_back(onedcor);
    }

    /** Return the track ID */
    double trackID() const { return m_track; }

    /** Get the identity of the particle */
    double getPDG() const { return m_pdg; }

    /** Get the track-level MC dE/dx mean for this track */
    double getDedx() const { return m_dedx; }
    /** Get the dE/dx mean for this track */
    double getDedxMean() const { return m_dedx_avg; }
    /** Get dE/dx truncated mean for this track */
    double getTruncatedMean() const { return m_dedx_avg_truncated; }
    /** Get the error on the dE/dx truncated mean for this track */
    double getError() const { return m_dedx_avg_truncated_err; }

    /** Return the vector of dE/dx values for this track */
    std::vector< double > getDedxList() const { return m_hDedx; }

    /** Return the track momentum valid in the CDC */
    double getMomentum() const { return m_p_cdc; }
    /** Return cos(theta) for this track */
    double getCosTheta() const { return m_cosTheta; }
    /** Return the charge for this track */
    int getCharge() const { return m_charge; }
    /** Return the total path length for this track */
    double getLength() const { return m_length; }

    /** Return the cosine correction for this track */
    double getCosineCorrection() const { return m_cosCor; }
    /** Return the run gain for this track */
    double getRunGain() const { return m_runGain; }

    /** Set the track level MC dE/dx mean for this track */
    void setDedx(double dedx) { m_dedx = dedx; }
    /** Set the dE/dx mean for this track */
    void setDedxMean(double mean) { m_dedx_avg = mean; }
    /** Set the dE/dx truncated average for this track */
    void setTruncatedMean(double mean) { m_dedx_avg_truncated = mean; }
    /** Set the error on the dE/dx truncated mean for this track */
    void setError(double error) { m_dedx_avg_truncated_err = error; }


    // Layer level

    /** Return the number of layer hits for this track */
    int getNLayerHits() const { return m_lDedx.size(); }
    /** Return the number of hits used to determine the truncated mean */
    double getNLayerHitsUsed() const { return m_lNHitsUsed; }

    /** Return the number of hits combined per layer */
    int getNHitsCombined(int i) const { return m_lNHitsCombined[i]; }
    /** Return the wire number of the longest hit per layer */
    int getWireLongestHit(int i) const { return m_lWireLongestHit[i]; }
    /** Return the (global) layer number for a layer hit */
    int getLayer(int i) const { return m_lLayer[i]; }
    /** Return the distance travelled in this layer */
    double getLayerPath(int i) const { return m_lPath[i]; }
    /** Return the total dE/dx for this layer */
    double getLayerDedx(int i) const { return m_lDedx[i]; }

    /** Set the total dE/dx for this layer */
    void setLayerDedx(int i, double dedx) { m_lDedx[i] = dedx; }


    // Hit level

    /** Return the number of hits for this track */
    int size() const { return m_hDedx.size(); }

    /** Return the sensor ID for this hit: wire number in the layer */
    int getWireInLayer(int i) const { return m_hLWire[i]; }
    /** Return the sensor ID for this hit: wire number for CDC (0-14336) */
    int getWire(int i) const { return m_hWire[i]; }
    /** Return the (global) layer number for a hit */
    int getHitLayer(int i) const { return m_hLayer[i]; }

    /** Return the path length through the cell for this hit */
    double getPath(int i) const { return m_hPath[i]; }
    /** Return the dE/dx value for this hit */
    double getDedx(int i) const { return m_hDedx[i]; }
    /** Return the adcCount for this hit */
    int getADCCount(int i) const { return m_hADCCount[i]; }
    /** Return the distance of closest approach to the sense wire for this hit */
    double getDoca(int i) const { return m_hDoca[i]; }
    /** Return the entrance angle in the CDC cell for this hit */
    double getEnta(int i) const { return m_hEnta[i]; }
    /** Return the drift time for this hit */
    int getDriftT(int i) const { return m_hDriftT[i]; }

    /** Return the ionization charge collected for this hit */
    double getDE(int i) const { return m_hdE[i]; }
    /** Return the height of the CDC cell */
    double getCellHeight(int i) const { return m_hCellHeight[i]; }
    /** Return the half-width of the CDC cell */
    double getCellHalfWidth(int i) const { return m_hCellHalfWidth[i]; }
    /** Return the drift distance for this hit */
    double getDriftD(int i) const { return m_hDriftD[i]; }
    /** Return the drift distance resolution for this hit */
    double getDriftDRes(int i) const { return m_hDriftDRes[i]; }

    /** Return the wire gain for this hit */
    double getWireGain(int i) const { return m_hWireGain[i]; }
    /** Return the 2D correction for this hit */
    double getTwoDCorrection(int i) const { return m_hTwodCor[i]; }
    /** Return the 1D correction for this hit */
    double getOneDCorrection(int i) const { return m_hOnedCor[i]; }

    /** Set the dE/dx value for this hit */
    void setDedx(int i, double dedx) { m_hDedx[i] = dedx; }

  private:

    // track level information
    int m_track; /**< ID number of the Track */
    int m_charge;    /**< particle charge from tracking (+1 or -1) */
    double m_cosTheta; /**< cos(theta) for the track */
    double m_p;        /**< momentum at the IP */
    double m_p_cdc;    /**< momentum at the inner layer of the CDC */
    double m_pdg;        /**< MC PID */
    double m_mcmass;     /**< MC PID mass */
    double m_mother_pdg; /**< MC PID of mother particle */
    double m_p_true;     /**< MC true momentum */
    double m_length;   /**< total distance travelled by the track */

    // calibration constants
    double m_scale; /**< scale factor to make electrons ~1 */
    double m_cosCor;  /**< calibration cosine correction */
    double m_runGain; /**< calibration run gain */
    std::vector<double> m_hWireGain; /**< calibration hit gain (indexed on number of hits) */
    std::vector<double> m_hTwodCor;  /**< calibration 2-D correction (indexed on number of hits) */
    std::vector<double> m_hOnedCor;  /**< calibration 1-D correction (indexed on number of hits) */
    double m_predmean[Const::ChargedStable::c_SetSize]; /**< predicted dE/dx truncated mean */
    double m_predres[Const::ChargedStable::c_SetSize];  /**< predicted dE/dx resolution */

    // track level dE/dx measurements
    double m_dedx;     /**< track level MC dE/dx truncated mean used to get PID value */
    double m_dedx_avg;               /**< dE/dx mean value per track */
    double m_dedx_avg_truncated;     /**< dE/dx truncated mean per track */
    double m_dedx_avg_truncated_err; /**< standard deviation of m_dedx_avg_truncated */
    double m_cdcChi[Const::ChargedStable::c_SetSize];  /**< chi values for each particle type */
    double m_cdcLogl[Const::ChargedStable::c_SetSize]; /**< log likelihood for each particle, not including momentum prior */

    // layer level information (just don't mix with the hit arrays)
    int m_lNHitsUsed; /**< number of hits on this track used for truncated mean */
    std::vector<int> m_lNHitsCombined; /**< number of hits combined in the layer */
    std::vector<int> m_lWireLongestHit; /**< wire id for the longest hit in the layer */
    std::vector<int> m_lLayer; /**< layer id corresponding to dedx */
    std::vector<double> m_lPath;   /**< distance flown through active medium in current segment */
    std::vector<double> m_lDedx;   /**< extracted dE/dx (arb. units, detector dependent) */

    // hit level information
    std::vector<int> m_hLWire;     /**< wire ID within the layer */
    std::vector<int> m_hWire;     /**< continuous wire ID in the CDC */
    std::vector<int> m_hLayer;    /**< layer number */
    std::vector<double> m_hPath;    /**< path length in the CDC cell */
    std::vector<double> m_hDedx;  /**< charge per path length (dE/dx) */
    std::vector<int> m_hADCCount; /**< adcCount per hit */
    std::vector<double> m_hDoca;  /**< distance of closest approach to sense wire */
    std::vector<double> m_hEnta;  /**< entrance angle in CDC cell */
    std::vector<double> m_hdE;    /**< charge per hit */
    std::vector<int> m_hDriftT;       /**< drift time for each hit */
    std::vector<double> m_hDriftD;    /**< drift distance for each hit */
    std::vector<double> m_hDriftDRes; /**< drift distance resolution for each hit */

    std::vector<double> m_hCellHeight;    /**< height of the CDC cell */
    std::vector<double> m_hCellHalfWidth; /**< half-width of the CDC cell */

    ClassDef(CDCDedxTrack, 7); /**< Debug output for CDCDedxPID module. */
  };
}
#endif
