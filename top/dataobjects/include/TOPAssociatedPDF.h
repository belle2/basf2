/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/datastore/RelationsObject.h>
#include <vector>

namespace Belle2 {

  /**
   * Class to store analytic PDF associated with a photon
   */
  class TOPAssociatedPDF : public RelationsObject {

  public:

    /**
     * parameters of a PDF peak
     */
    struct PDFPeak {
      float position = 0;  /**< position in time */
      float width = 0; /**< width (sigma) */
      float numPhotons = 0;  /**< number of photons */
      float fic = 0; /**< Cerenkov azimuthal angle phi */
      float e = 0; /**< mean photon energy [eV] */
      float sige = 0; /**< photon energy sigma squared [eV^2] */
      int nx = 0; /**< total number of reflections in x */
      int ny = 0; /**< total number of reflections in y */
      int nxm = 0; /**< number of reflections in x before mirror */
      int nym = 0; /**< number of reflections in y before mirror */
      int nxe = 0; /**< number of reflections in x in prism */
      int nye = 0; /**< number of reflections in y in prism */
      float xd = 0; /**< unfolded x coordinate of a pixel */
      float yd = 0; /**< unfolded y coordinate of a pixel */
      int type = 0; /**< 0 unknown, 1 direct photon, 2 reflected photon */
      float kxe = 0; /**< reconstructed photon direction in x at emission */
      float kye = 0; /**< reconstructed photon direction in y at emission */
      float kze = 0; /**< reconstructed photon direction in z at emission */
      float kxd = 0; /**< reconstructed photon direction in x at detection */
      float kyd = 0; /**< reconstructed photon direction in y at detection */
      float kzd = 0; /**< reconstructed photon direction in z at detection */
    };

    /**
     * Default constructor
     */
    TOPAssociatedPDF() {}

    /**
     * Useful constructor
     */
    explicit TOPAssociatedPDF(int PDG): m_PDG(PDG)
    {}

    /**
     * Append PDF peak
     */
    void appendPeak(const PDFPeak& peak, float weight)
    {
      m_peaks.push_back(peak);
      m_weights.push_back(weight);
    }

    /**
     * Set background weight
     */
    void setBackgroundWeight(float weight)
    {
      m_BGWeight = weight;
    }

    /**
     * Set delta-ray weight
     */
    void setDeltaRayWeight(float weight)
    {
      m_deltaRayWeight = weight;
    }

    /**
     * Returns PDG code of a particle hypothesis used to construct PDF
     */
    int getPDGHypothesis() const {return m_PDG;}

    /**
     * Returns a collection of associated PDF peaks
     */
    const std::vector<PDFPeak>& getPeaks() const {return m_peaks;}

    /**
     * Returns weights of associated PDF peaks
     */
    const std::vector<float>& getWeights() const {return m_weights;}

    /**
     * Returns background weight
     */
    float getBGWeight() const {return m_BGWeight;}

    /**
     * Returns delta-ray weight
     */
    float getDeltaRayWeight() const {return m_deltaRayWeight;}

    /**
     * Returns a PDF peak selected randomly according to weights.
     * Null ptr is returned if background or delta-ray contribution is selected instead.
     */
    const PDFPeak* getSinglePeak() const;

  private:

    int m_PDG = 0; /**< PDG code of a particle hypothesis used to construct PDF */
    std::vector<PDFPeak> m_peaks; /**< collection of associated PDF peaks */
    std::vector<float> m_weights; /**< corresponding weights */
    float m_BGWeight = 0; /**< background weight */
    float m_deltaRayWeight = 0; /**< delta-ray weight */

    ClassDefOverride(TOPAssociatedPDF, 2); /**< ClassDef */

  };

}
