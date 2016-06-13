/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <iostream>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for sigma (intrinsic space resolution).
   */
  class CDCSigmas: public TObject {
  public:

    /**
     * Constants
     */
    enum {c_nSLayers = 56,    /**< total no. of layers */
          c_nSigmaParams = 7  /**< no. of params. for sigma */
         };

    /**
     * Default constructor
     */
    CDCSigmas(): m_sigma() {}

    /**
     * Set sigma parameter
     */
    void setSigmaParam(unsigned short iCLayer, unsigned short i, float param)
    {
      m_sigma[iCLayer][i] = param;
    }

    /**
     * Update sigma parameter
     */
    void addSigmaParam(unsigned short iCLayer, unsigned short i, float delta)
    {
      m_sigma[iCLayer][i] += delta;
    }

    /**
     * Get sigma parameter
     */
    float getSigmaParam(unsigned short iCLayer, unsigned short i) const
    {
      return m_sigma[iCLayer][i];
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Contents of sigma db" << std::endl;
      std::cout << "parameters for sigma" << std::endl;

      for (unsigned short iCL = 0; iCL < c_nSLayers; ++iCL) {
        for (unsigned short i = 0; i < c_nSigmaParams; ++i) {
          std::cout << " " << m_sigma[iCL][i];
        }
        std::cout << " " << std::endl;
      }

    }

  private:

    float m_sigma[c_nSLayers][c_nSigmaParams];  /*!< sigma params. for each layer.*/

    ClassDef(CDCSigmas, 1); /**< ClassDef */
  };

} // end namespace Belle2
