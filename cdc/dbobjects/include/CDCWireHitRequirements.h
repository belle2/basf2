/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC and tracking group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <iostream>
#include <TObject.h>
#include <framework/logging/Logger.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Belle2 {
  /**
   * Database object containing cut values to filter CDCWireHits.
   * In the allowed range for ADC, TOT, ..., an upper value of -1 stands
   * for unbounded. For example, the ADC range {5,-1} accepts all the
   * hits with ADC>=5.
   */
  class CDCWireHitRequirements: public TObject {

    // Integer range
    typedef std::pair<short, short> intRange;
    // Float range
    typedef std::pair<float, float> floatRange;

  public:

    /**
     * Default constructor
     */
    CDCWireHitRequirements() = default;

    /**
     * Constructor using a boost::property_tree
     */
    explicit CDCWireHitRequirements(boost::property_tree::ptree tree)
    {

      try {

        m_ADCRangeFirstSuperLayer  = std::make_pair(tree.get<short>("FirstSuperLayer.ADC.min"),
                                                    tree.get<short>("FirstSuperLayer.ADC.max"));
        m_ADCRangeOuterSuperLayers = std::make_pair(tree.get<short>("OuterSuperLayers.ADC.min"),
                                                    tree.get<short>("OuterSuperLayers.ADC.max"));

        m_TOTRangeFirstSuperLayer = std::make_pair(tree.get<short>("FirstSuperLayer.TOT.min"),
                                                   tree.get<short>("FirstSuperLayer.TOT.max"));
        m_TOTRangeOuterSuperLayers = std::make_pair(tree.get<short>("OuterSuperLayers.TOT.min"),
                                                    tree.get<short>("OuterSuperLayers.TOT.max"));

        m_ADCOverTOTRangeFirstSuperLayer = std::make_pair(tree.get<float>("FirstSuperLayer.ADCOverTOT.min"),
                                                          tree.get<float>("FirstSuperLayer.ADCOverTOT.max"));
        m_ADCOverTOTRangeOuterSuperLayers = std::make_pair(tree.get<float>("OuterSuperLayers.ADCOverTOT.min"),
                                                           tree.get<float>("OuterSuperLayers.ADCOverTOT.max"));

        for (boost::property_tree::ptree::value_type& row : tree.get_child("FirstSuperLayer.MaxADCGivenTOT")) {
          m_MaxADCGivenTOTFirstSuperLayer.push_back(std::make_pair(row.second.get<short>("givenTOT"),
                                                                   row.second.get<short>("maxADC")));
        }
        for (boost::property_tree::ptree::value_type& row : tree.get_child("OuterSuperLayers.MaxADCGivenTOT")) {
          m_MaxADCGivenTOTOuterSuperLayers.push_back(std::make_pair(row.second.get<short>("givenTOT"),
                                                                    row.second.get<short>("maxADC")));
        }

      } catch (boost::property_tree::ptree_error& e) {
        // Any problem when parsing the tree?
        B2FATAL("Cannot parse CDCWireHit requirement information: " << e.what());
      }

    }

    /**
     * Destructor
     */
    ~CDCWireHitRequirements() = default;

    /// Get allowed ADC range (first super layer of CDC)
    const intRange& getADCRangeFirstSuperLayer() const
    {
      return m_ADCRangeFirstSuperLayer;
    }

    /// Set allowed ADC range (first super layer of CDC)
    void setADCRangeFirstSuperLayer(const intRange& ADCRangeFirstSuperLayer)
    {
      m_ADCRangeFirstSuperLayer =  ADCRangeFirstSuperLayer;
    }

    /// Get allowed ADC range (outer super layers of CDC)
    const intRange& getADCRangeOuterSuperLayers() const
    {
      return m_ADCRangeOuterSuperLayers;
    }

    /// Set allowed ADC range (outer super layers of CDC)
    void setADCRangeOuterSuperLayers(const intRange& ADCRangeOuterSuperLayers)
    {
      m_ADCRangeOuterSuperLayers =  ADCRangeOuterSuperLayers;
    }

    /// Get allowed TOT range (first super layer of CDC)
    const intRange& getTOTRangeFirstSuperLayer() const
    {
      return m_TOTRangeFirstSuperLayer;
    }

    /// Set allowed TOT range (first super layer of CDC)
    void setTOTRangeFirstSuperLayer(const intRange& TOTRangeFirstSuperLayer)
    {
      m_TOTRangeFirstSuperLayer =  TOTRangeFirstSuperLayer;
    }

    /// Get allowed TOT range (outer super layers of CDC)
    const intRange& getTOTRangeOuterSuperLayers() const
    {
      return m_TOTRangeOuterSuperLayers;
    }

    /// Set allowed TOT range (outer super layers of CDC)
    void setTOTRangeOuterSuperLayers(const intRange& TOTRangeOuterSuperLayers)
    {
      m_TOTRangeOuterSuperLayers =  TOTRangeOuterSuperLayers;
    }

    /// Get allowed ADC/TOT range (first super layer of CDC)
    const floatRange& getADCOverTOTRangeFirstSuperLayer() const
    {
      return m_ADCOverTOTRangeFirstSuperLayer;
    }

    /// Set allowed ADC/TOT range (first super layer of CDC)
    void setADCOverTOTRangeFirstSuperLayer(const floatRange& ADCOverTOTRangeFirstSuperLayer)
    {
      m_ADCOverTOTRangeFirstSuperLayer =  ADCOverTOTRangeFirstSuperLayer;
    }

    /// Get allowed ADC/TOT range (outer super layers of CDC)
    const floatRange& getADCOverTOTRangeOuterSuperLayers() const
    {
      return m_ADCOverTOTRangeOuterSuperLayers;
    }

    /// Set allowed ADC/TOT range (outer super layers of CDC)
    void setADCOverTOTRangeOuterSuperLayers(const floatRange& ADCOverTOTRangeOuterSuperLayers)
    {
      m_ADCOverTOTRangeOuterSuperLayers = ADCOverTOTRangeOuterSuperLayers;
    }

    /// Get vector of maximum allowed ADC given TOT (first super layer of CDC)
    const std::vector<std::pair<short, short> > getMaxADCGivenTOTFirstSuperLayer() const
    {
      return m_MaxADCGivenTOTFirstSuperLayer;
    }

    /// Set vector of maximum allowed ADC given TOT (first super layer of CDC)
    void setMaxADCGivenTOTFirstSuperLayer(const std::vector<std::pair<short, short> >& MaxADCGivenTOTFirstSuperLayer)
    {
      m_MaxADCGivenTOTFirstSuperLayer = MaxADCGivenTOTFirstSuperLayer;
    }

    /// Get vector of maximum allowed ADC given TOT (outer super layers of CDC)
    const std::vector<std::pair<short, short> > getMaxADCGivenTOTOuterSuperLayers() const
    {
      return m_MaxADCGivenTOTOuterSuperLayers;
    }

    /// Set vector of maximum allowed ADC given TOT (outer super layers of CDC)
    void setMaxADCGivenTOTOuterSuperLayers(const std::vector<std::pair<short, short> >& MaxADCGivenTOTOuterSuperLayers)
    {
      m_MaxADCGivenTOTOuterSuperLayers = MaxADCGivenTOTOuterSuperLayers;
    }

    /// Print content
    void dump() const
    {
      std::string MaxADCGivenTOTFirstSuperLayer = "";
      for (auto& v : getMaxADCGivenTOTFirstSuperLayer()) {
        MaxADCGivenTOTFirstSuperLayer += "If TOT == " + std::to_string(v.first) +
                                         ", then ADC <= " + std::to_string(v.second) + "\n";
      }

      std::string MaxADCGivenTOTOuterSuperLayers = "";
      for (auto& v : getMaxADCGivenTOTOuterSuperLayers()) {
        MaxADCGivenTOTOuterSuperLayers += "If TOT == " + std::to_string(v.first) +
                                          ", then ADC <= " + std::to_string(v.second) + "\n";
      }

      B2INFO(" " << std::endl
             << "=======================================================" << std::endl
             << "CDCWireHit requirements (-1 values stand for unbounded)" << std::endl
             << "=======================================================" << std::endl
             << "First Super Layer: " << std::endl
             << getADCRangeFirstSuperLayer().first << " <= ADC <= " << getADCRangeFirstSuperLayer().second << std::endl
             << getTOTRangeFirstSuperLayer().first << " <= TOT <= " << getTOTRangeFirstSuperLayer().second << std::endl
             << getADCOverTOTRangeFirstSuperLayer().first << " <= ADC/TOT <= " << getADCOverTOTRangeFirstSuperLayer().second << std::endl
             << MaxADCGivenTOTFirstSuperLayer << std::endl
             << "Outer Super Layers: " << std::endl
             << getADCRangeOuterSuperLayers().first << " <= ADC <= " << getADCRangeOuterSuperLayers().second << std::endl
             << getTOTRangeOuterSuperLayers().first << " <= TOT <= " << getTOTRangeOuterSuperLayers().second << std::endl
             << getADCOverTOTRangeOuterSuperLayers().first << " <= ADC/TOT <= " << getADCOverTOTRangeOuterSuperLayers().second << std::endl
             << MaxADCGivenTOTOuterSuperLayers
             << "=======================================================" << std::endl);
    }

  private:

    /// Allowed ADC range (first super layer of CDC)
    intRange m_ADCRangeFirstSuperLayer  = {1, -1}; // an upper value of -1 stands for unbounded.
    /// Allowed ADC range (outer super layers of CDC)
    intRange m_ADCRangeOuterSuperLayers = {1, -1};

    /// Allowed TOT range (first super layer of CDC)
    intRange m_TOTRangeFirstSuperLayer = {1, -1};
    /// Allowed TOT range (outer super layers of CDC)
    intRange m_TOTRangeOuterSuperLayers = {1, -1};

    /// Allowed ADC/TOT range (first super layer of CDC)
    floatRange m_ADCOverTOTRangeFirstSuperLayer = {0.0, -1.0};
    /// Allowed ADC/TOT range (outer super layers of CDC)
    floatRange m_ADCOverTOTRangeOuterSuperLayers = {0.0, -1.0};

    /// Vector of conditions of the type: if TOT==x, then ADC<=y (first super layer of CDC)
    std::vector<std::pair<short, short> > m_MaxADCGivenTOTFirstSuperLayer = {};
    /// Vector of conditions of the type: if TOT==x, then ADC<=y (outer super layers of CDC)
    std::vector<std::pair<short, short> > m_MaxADCGivenTOTOuterSuperLayers = {};

    /** ROOT ClassDef */
    ClassDef(CDCWireHitRequirements, 2);
  };

} // end namespace Belle2
