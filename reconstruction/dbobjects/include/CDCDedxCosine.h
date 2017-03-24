/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TObject.h>

namespace Belle2 {

  /**
   *   dE/dx wire gain calibration constants
   */

  class CDCDedxCosine: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedxCosine(): m_nbins(30), m_costh(m_nbins, 0), m_means(m_nbins, 0) {};

    /**
     * Constructor
     */
    CDCDedxCosine(int nbins, std::vector<float>& costh, std::vector<float>& means): m_nbins(nbins), m_costh(costh), m_means(means) {};

    /**
     * Destructor
     */
    ~CDCDedxCosine() {};

    /** Return cos(theta)
     * @return cos(theta)
     */
    float getCosTheta(int bin) const {return m_costh[bin]; };

    /** Return dE/dx mean value
     * @return dE/dx mean value
     */
    float getCosine(int bin) const {return m_means[bin]; };

  private:
    int m_nbins; /**< number of cos(theta) bins for electron saturation correction */
    std::vector<float> m_costh; /**< central bin values for cos(theta) bins */
    std::vector<float> m_means; /**< dE/dx means in bins of cos(theta) */

    ClassDef(CDCDedxCosine, 1); /**< ClassDef */
  };
} // end namespace Belle2
