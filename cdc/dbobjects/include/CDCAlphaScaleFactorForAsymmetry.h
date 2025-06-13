/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <TObject.h>

namespace Belle2 {

  /**
   * Database object for scale factors on alpha for CDC hit charge asymmetry. Needed only for MC production.
   */
  class CDCAlphaScaleFactorForAsymmetry: public TObject {
  public:
    /**
     * Constants
     */
    enum {c_nLayers       = 56,  /**< no. of layers */
          c_maxNAlphaBins = 150, /**< max. no. of alpha angle bins */
         };


    /**
     * Default constructor
     */
    CDCAlphaScaleFactorForAsymmetry() {}


    /**
     * Set the factors in the list
     * @param iCLayer laerID(0-55) or wireID
     * @param factors factors
     */
    void setFactors(unsigned short iCLayer, const std::vector<float>& factors)
    {
      m_Scales.insert(std::pair<unsigned short, std::vector<float>>(iCLayer, factors));
    }

    /**
     * Get the no. of entries in the list
     */
    unsigned short getEntries() const
    {
      return m_Scales.size();
    }

    /**
     * Get the whole list
     */
    std::map<unsigned short, std::vector<float>> getFactors() const
    {
      return m_Scales;
    }

    /**
     * Get the factors for the iCLayer
     * @param  iCLayer layerID
     * @return fudge factors for the iCLayer
     */
    const std::vector<float>& getFactors(unsigned short iCLayer) const
    {
      std::map<unsigned short, std::vector<float>>::const_iterator it = m_Scales.find(iCLayer);
      if (it != m_Scales.end()) {
        return it->second;
      } else {
        B2FATAL("Specified iCLayer not found in getFactors !");
      }
    }

    /**
     * Print all contents
     */
    void dump() const
    {
      std::cout << " " << std::endl;
      std::cout << "Scale factor list" << std::endl;
      std::cout << "#entries= " << m_Scales.size() << std::endl;
      std::cout << "in order of iCLayer and factors" << std::endl;

      for (auto const& ent : m_Scales) {
        std::cout << ent.first;
        unsigned short np = (ent.second).size();
        for (unsigned short i = 0; i < np; ++i) {
          std::cout << "  " << (ent.second)[i];
        }
        std::cout << std::endl;
      }
    }

  private:
    std::map<unsigned short, std::vector<float>> m_Scales; /**< scale factors */

    ClassDef(CDCAlphaScaleFactorForAsymmetry, 1); /**< ClassDef */
  };

} // end namespace Belle2
