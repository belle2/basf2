/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/logging/Logger.h>

#include <TObject.h>
#include <cmath>

namespace Belle2 {

  /**
   *   dE/dx 1D cell correction calibration constants
   */

  class CDCDedx1DCell: public TObject {

  public:

    /**
     * Default constructor
     */
    CDCDedx1DCell(): m_version(0), m_onedgains(), m_layerToGroup() {};

    /**
    * Constructor keep this untill we modfity CDCDedx1DCellAlgorithm.cc code
    */
    CDCDedx1DCell(short version, const std::vector<std::vector<double>>& onedgains);

    /** Constructor with grouped constants */
    CDCDedx1DCell(short version,
                  const std::vector<std::vector<double>>& groupGains,
                  const std::vector<unsigned int>& layerToGroup);

    /**
     * Destructor
     */
    ~CDCDedx1DCell() {};

    /**
     * Combine payloads
     **/
    CDCDedx1DCell& operator*=(CDCDedx1DCell const& rhs);

    /**
    * Get the version for the 1D cleanup
     */
    short getVersion() const { return m_version; };

    /** Number of groups */
    unsigned int getNGroups() const {return m_onedgains.size();};

    /** Number of layers */
    unsigned int getNLayers() const
    {
      if (!m_layerToGroup.empty()) return m_layerToGroup.size();

      if (m_onedgains.size() == 2) return 56;

      return 0;
    };

    /** Get group index for layer */
    unsigned int getGroup(unsigned int layer) const
    {
      if (layer >= getNLayers()) {
        B2ERROR("Layer out of range");
        return std::numeric_limits<unsigned int>::max();
      }

      if (!m_layerToGroup.empty()) {
        return m_layerToGroup[layer];
      }

      if (m_onedgains.size() == 2) {
        return (layer < 8) ? 0 : 1;
      }

      B2ERROR("Invalid 1DCell payload state");
      return std::numeric_limits<unsigned int>::max();
    };

    /** Get the number of bins for the entrance angle correction
    */
    unsigned int getNBins(unsigned int layer) const
    {
      const unsigned int group = getGroup(layer);

      if (group >= m_onedgains.size()) return 0;

      return m_onedgains[group].size();
    };

    /**
        * Get the grouped calibration constants
        * @return grouped calibration constants
        */
    const std::vector<std::vector<double>>& getOneDCell() const
    {
      return m_onedgains;
    }

    /**
       * Get the Layer-to-group map
       * @return layer map
       */

    const std::vector<unsigned int>& getLayerMap() const
    {
      return m_layerToGroup;
    }

    /** Return dE/dx mean value for the given bin
     * @param layer is layer number between 0-55
     * @param bin is enta bin number
     */
    double getMean(unsigned int layer, unsigned int bin) const;


    /** Reset dE/dx mean value for the given bin
     * @param layer is layer number between 0-55
     * @param bin is enta bin number
     * @param value is constant for requested entabin and layer
    */
    void setMean(unsigned int layer, unsigned int bin, double value);


    /** Return dE/dx mean value for given entrance angle
     * @param layer continuous layer number
     * @param enta entrance angle (-pi/2 to pi/2)
     */
    double getMean(unsigned int layer, double enta) const;


  private:

    /** Payload validation */
    bool isValidGroupedPayload() const;

    /** dE/dx cleanup correction versus entrance angle
    may be different for different layers, so store as a vector of vectors
    keep a version number to identify which layers are valid */
    short m_version; /**< version number for 1D cleanup correction */
    std::vector<std::vector<double>> m_onedgains; /**< dE/dx means in entrance angle bins */
    std::vector<unsigned int> m_layerToGroup;  /** layer to group map */

    ClassDef(CDCDedx1DCell, 6); /**< ClassDef */
  };
} // end namespace Belle2
