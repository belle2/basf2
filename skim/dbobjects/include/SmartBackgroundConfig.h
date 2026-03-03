/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>

#include <unordered_map>
#include <vector>


namespace Belle2 {

  /**
   * Config class for the Smart Background module.
   * Saves mappings of pdg numbers, skim codes and activation function paramters so they can be saved to
   * and loaded from the conditions database.
   */
  class SmartBackgroundConfig : public TObject {

  public:

    /**
     * Default constructor
     */
    SmartBackgroundConfig() { }

    /**
     * Constructor setting all three maps
     */
    SmartBackgroundConfig(const std::unordered_map<int, int>& pdgMapping,
                          const std::unordered_map<int, uint16_t>& skimcodesMapping,
                          const std::unordered_map<int, std::vector<float>>& paramMapping,
                          const std::unordered_map<int, std::string>& skimnamesMapping,
                          const std::unordered_map<std::string, int32_t>& eventtypeMapping,
                          const float maxWeight) : m_pdgMapping(pdgMapping), m_skimcodesMapping(skimcodesMapping),
      m_paramMapping(paramMapping), m_skimnamesMapping(skimnamesMapping), m_eventtypeMapping(eventtypeMapping),
      m_maxWeight(maxWeight) { }

    /**
     * Return pdg mapping.
     */
    std::unordered_map<int, int> getPdgMapping() const
    {
      return m_pdgMapping;
    }

    /**
     * Return skimcodes mapping.
     */
    std::unordered_map<int, uint16_t> getSkimcodesMapping() const
    {
      return m_skimcodesMapping;
    }

    /**
     * Return parameter mapping.
     */
    std::unordered_map<int, std::vector<float>> getParameterMapping() const
    {
      return m_paramMapping;
    }

    /**
     * Return skim names mapping
     */
    std::unordered_map<int, std::string> getSkimnamesMapping() const
    {
      return m_skimnamesMapping;
    }

    /**
     * Return event type mapping
     */
    std::unordered_map<std::string, int32_t> getEventtypeMapping() const
    {
      return m_eventtypeMapping;
    }

    /**
    * Return maximum possible weight
    */
    float getMaxWeight() const
    {
      return m_maxWeight;
    }

  private:

    /**
     * Mapping of PDG numbers to consecutive integers.
     */
    std::unordered_map<int, int> m_pdgMapping;

    /**
     * Mapping of skimcodes to output indices
     */
    std::unordered_map<int, uint16_t> m_skimcodesMapping;

    /**
     * Mapping of skimcodes to activation function parameters optimized for speedup
     */
    std::unordered_map<int, std::vector<float>> m_paramMapping;

    /**
     * Mapping of skimcodes to skim names
     */
    std::unordered_map<int, std::string> m_skimnamesMapping;

    /**
     * Mapping of event type names to integers
     */
    std::unordered_map<std::string, int32_t> m_eventtypeMapping;

    /**
     * Maximum possible event weight after importance sampling
     */
    float m_maxWeight;

    /**
     * Classdef
     */
    ClassDef(SmartBackgroundConfig, 1);
  };

}