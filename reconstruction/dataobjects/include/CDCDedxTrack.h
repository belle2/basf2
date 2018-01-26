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
      m_scale(0), m_coscor(0), m_rungain(0),
      l_nHits(0), l_nHitsUsed(0), h_nHits(0)
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
      l_nhitscombined.push_back(nhitscombined);
      l_wirelongesthit.push_back(wirelongesthit);
      l_layer.push_back(layer);
      l_path.push_back(distance);
      l_dedx.push_back(dedxValue);

      m_length += distance;
    }

    /** Add a single hit to the object */
    void addHit(int lwire, int wire, int layer, double doca, double enta, int adcCount, double dE, double path, double dedx,
                double cellHeight, double cellHalfWidth, int driftT, double driftD, double driftDRes, double wiregain, double twodcor,
                double onedcor)
    {
      h_nHits++;
      h_lwire.push_back(lwire);
      h_wire.push_back(wire);
      h_layer.push_back(layer);
      h_path.push_back(path);
      h_dedx.push_back(dedx);
      h_adcCount.push_back(adcCount);
      h_doca.push_back(doca);
      h_enta.push_back(enta);
      h_driftT.push_back(driftT);

      h_dE.push_back(dE);
      h_cellHeight.push_back(cellHeight);
      h_cellHalfWidth.push_back(cellHalfWidth);
      h_driftD.push_back(driftD);
      h_driftDRes.push_back(driftDRes);

      h_wiregain.push_back(wiregain);
      h_twodcor.push_back(twodcor);
      h_onedcor.push_back(onedcor);
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
    std::vector< double > getDedxList() const { return h_dedx; }

    /** Return the track momentum valid in the CDC */
    double getMomentum() const { return m_p_cdc; }
    /** Return cos(theta) for this track */
    double getCosTheta() const { return m_cosTheta; }
    /** Return the charge for this track */
    int getCharge() const { return m_charge; }
    /** Return the total path length for this track */
    double getLength() const { return m_length; }

    /** Return the cosine correction for this track */
    double getCosineCorrection() const { return m_coscor; }
    /** Return the run gain for this track */
    double getRunGain() const { return m_rungain; }

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
    int getNLayerHits() const { return l_dedx.size(); }
    /** Return the number of hits used to determine the truncated mean */
    double getNLayerHitsUsed() const { return l_nHitsUsed; }

    /** Return the number of hits combined per layer */
    int getNHitsCombined(int i) const { return l_nhitscombined[i]; }
    /** Return the wire number of the longest hit per layer */
    int getWireLongestHit(int i) const { return l_wirelongesthit[i]; }
    /** Return the (global) layer number for a layer hit */
    int getLayer(int i) const { return l_layer[i]; }
    /** Return the distance travelled in this layer */
    double getLayerPath(int i) const { return l_path[i]; }
    /** Return the total dE/dx for this layer */
    double getLayerDedx(int i) const { return l_dedx[i]; }

    /** Set the total dE/dx for this layer */
    void setLayerDedx(int i, double dedx) { l_dedx[i] = dedx; }


    // Hit level

    /** Return the number of hits for this track */
    int size() const { return h_dedx.size(); }

    /** Return the sensor ID for this hit: wire number in the layer */
    int getWireInLayer(int i) const { return h_lwire[i]; }
    /** Return the sensor ID for this hit: wire number for CDC (0-14336) */
    int getWire(int i) const { return h_wire[i]; }
    /** Return the (global) layer number for a hit */
    int getHitLayer(int i) const { return h_layer[i]; }

    /** Return the path length through the cell for this hit */
    double getPath(int i) const { return h_path[i]; }
    /** Return the dE/dx value for this hit */
    double getDedx(int i) const { return h_dedx[i]; }
    /** Return the adcCount for this hit */
    int getADCCount(int i) const { return h_adcCount[i]; }
    /** Return the distance of closest approach to the sense wire for this hit */
    double getDoca(int i) const { return h_doca[i]; }
    /** Return the entrance angle in the CDC cell for this hit */
    double getEnta(int i) const { return h_enta[i]; }
    /** Return the drift time for this hit */
    int getDriftT(int i) const { return h_driftT[i]; }

    /** Return the ionization charge collected for this hit */
    double getDE(int i) const { return h_dE[i]; }
    /** Return the height of the CDC cell */
    double getCellHeight(int i) const { return h_cellHeight[i]; }
    /** Return the half-width of the CDC cell */
    double getCellHalfWidth(int i) const { return h_cellHalfWidth[i]; }
    /** Return the drift distance for this hit */
    double getDriftD(int i) const { return h_driftD[i]; }
    /** Return the drift distance resolution for this hit */
    double getDriftDRes(int i) const { return h_driftDRes[i]; }

    /** Return the wire gain for this hit */
    double getWireGain(int i) const { return h_wiregain[i]; }
    /** Return the 2D correction for this hit */
    double getTwoDCorrection(int i) const { return h_twodcor[i]; }
    /** Return the 1D correction for this hit */
    double getOneDCorrection(int i) const { return h_onedcor[i]; }

    /** Set the dE/dx value for this hit */
    void setDedx(int i, double dedx) { h_dedx[i] = dedx; }

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
    double m_coscor;  /**< calibration cosine correction */
    double m_rungain; /**< calibration run gain */
    std::vector<double> h_wiregain; /**< calibration hit gain (indexed on number of hits) */
    std::vector<double> h_twodcor;  /**< calibration 2-D correction (indexed on number of hits) */
    std::vector<double> h_onedcor;  /**< calibration 1-D correction (indexed on number of hits) */
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
    int l_nHits;     /**< number of layerhits on this track */
    int l_nHitsUsed; /**< number of hits on this track used for truncated mean */
    std::vector<int> l_nhitscombined; /**< number of hits combined in the layer */
    std::vector<int> l_wirelongesthit; /**< wire id for the longest hit in the layer */
    std::vector<int> l_layer; /**< layer id corresponding to dedx */
    std::vector<double> l_path;   /**< distance flown through active medium in current segment */
    std::vector<double> l_dedx;   /**< extracted dE/dx (arb. units, detector dependent) */

    // hit level information
    int h_nHits;     /**< number of hits on this track */
    std::vector<int> h_lwire;     /**< wire ID within the layer */
    std::vector<int> h_wire;     /**< continuous wire ID in the CDC */
    std::vector<int> h_layer;    /**< layer number */
    std::vector<double> h_path;    /**< path length in the CDC cell */
    std::vector<double> h_dedx;  /**< charge per path length (dE/dx) */
    std::vector<int> h_adcCount; /**< adcCount per hit */
    std::vector<double> h_doca;  /**< distance of closest approach to sense wire */
    std::vector<double> h_enta;  /**< entrance angle in CDC cell */
    std::vector<double> h_dE;    /**< charge per hit */
    std::vector<int> h_driftT;       /**< drift time for each hit */
    std::vector<double> h_driftD;    /**< drift distance for each hit */
    std::vector<double> h_driftDRes; /**< drift distance resolution for each hit */

    std::vector<double> h_cellHeight;    /**< height of the CDC cell */
    std::vector<double> h_cellHalfWidth; /**< half-width of the CDC cell */

    ClassDef(CDCDedxTrack, 7); /**< Debug output for CDCDedxPID module. */
  };
}
#endif
