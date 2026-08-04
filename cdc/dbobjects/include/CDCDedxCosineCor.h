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
  *   dE/dx cosine gain calibration constants
  */

  class CDCDedxCosineCor: public TObject {

  public:

    /**
    * Default constructor
    */
    CDCDedxCosineCor(): m_cosgains(), m_groupCosgains(), m_layerToGroup() {};

    /**
    * Old payloads Constructor
    * @param cosgains vector of calibration constants
    */
    explicit CDCDedxCosineCor(const std::vector<double>& cosgains): m_cosgains(cosgains), m_groupCosgains(),
      m_layerToGroup() {};

    /**
    * New-style constructor:
    * grouped cosine corrections with layer-to-group mapping
    * @param groupCosgains grouped calibration constants [group][bin]
    * @param layerToGroup map from layer index to group index
    */

    CDCDedxCosineCor(const std::vector<std::vector<double>>& groupCosgains,
                     const std::vector<unsigned int>& layerToGroup);

    /**
    * Destructor
    */
    ~CDCDedxCosineCor() {};

    /**
    * Combine payloads
    **/
    CDCDedxCosineCor& operator*=(const CDCDedxCosineCor& rhs);

    /**
    * Check whether grouped mode is used
    * @return true if grouped constants are available
    */
    bool isGrouped() const
    {
      return !m_groupCosgains.empty();
    }

    /**
    * Get number of groups in grouped mode
    * @return number of groups
    */
    unsigned int getNGroups() const
    {
      if (isGrouped()) return m_groupCosgains.size();
      return (m_cosgains.empty() ? 0 : 1);
    }

    /** Number of layers */
    unsigned int getNLayers() const
    {
      if (!m_layerToGroup.empty()) return m_layerToGroup.size();

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

      B2INFO("Invalid cosine correction payload state");
      return std::numeric_limits<unsigned int>::max();
    };

    /**
    * Get the number of bins
    * @return number of bins
    */
    unsigned int getSize(unsigned int layer) const
    {
      if (isGrouped()) {
        const unsigned int group = getGroup(layer);
        return m_groupCosgains[group].size();
      }

      return m_cosgains.size();
    }

    /**
    * Get the old-style calibration constants
    * @return calibration constants
    */
    const std::vector<double>& getCosCor() const
    {
      return m_cosgains;
    }

    /**
    * Get the grouped calibration constants
    * @return grouped calibration constants
    */
    const std::vector<std::vector<double>>& getGroupCosCor() const
    {
      return m_groupCosgains;
    }

    /**
    * Get the Layer-to-group map
    * @return layer map
    */
    const std::vector<unsigned int>& getLayerMap() const
    {
      return m_layerToGroup;
    }

    /**
    * Set old-style cosine correction
    * @param bin bin number
    * @param value value to be set
    */
    void setCosCor(unsigned int bin, double value);

    /**
    * Set grouped cosine correction
    * @param group group number
    * @param bin bin number
    * @param value value to be set
    */
    void setCosCor(unsigned int group, unsigned int bin, double value);

    /**
    * Return dE/dx mean value for the given bin in old mode
    * @param bin cosine bin
    * @return mean value
    */
    double getMean(unsigned int bin) const;

    /**
    * Return dE/dx mean value for given cos(theta) in old mode
    * @param costh cos(theta)
    * @return mean value
    */
    double getMean(double costh) const;

    /**
    * Return dE/dx mean value for the given layer and bin
    * @param layer layer index
    * @param bin cosine bin
    * @return mean value
    */
    double getMean(unsigned int layer, unsigned int bin) const;

    /**
    * Return dE/dx mean value for given layer and cos(theta)
    * @param layer layer index
    * @param costh cos(theta)
    * @return mean value
    */
    double getMean(unsigned int layer, double costh) const;

  private:

    /**
    * Helper to interpolate/extrapolate from one vector of gains
    * @param gains vector of gains
    * @param costh cos(theta)
    * @return interpolated mean value
    */
    double getMeanFromVector(const std::vector<double>& gains, double costh) const;

    /**
    * Validate grouped payload content
    * @return true if grouped payload is consistent
    */
    bool isValidGroupedPayload() const;

    /**
    * Multiply lhs by rhs with possible rebinning.
    * If rhs has fewer bins, each rhs bin is applied to a block of lhs bins.
    * This allows combining payloads with different binning (e.g. coarse => fine).
    */
    bool multiplyGains(std::vector<double>& lhs, const std::vector<double>& rhs) const;

    std::vector<double> m_cosgains; /**< old-style dE/dx gains in cos(theta) bins */
    std::vector<std::vector<double>> m_groupCosgains; /**< grouped dE/dx gains [group][bin] */
    std::vector<unsigned int> m_layerToGroup; /**< map from layer index to group index */

    ClassDef(CDCDedxCosineCor, 9); /**< ClassDef */
  };
} // end namespace Belle2
