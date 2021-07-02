/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <TObject.h>
#include <framework/logging/Logger.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace Belle2 {
  /**
   * Database object containing cut values to filter CDCWireHits.
   * The set of conditions is:
   *   {minADC<=ADC<=maxADC,
   *    minTOT<=TOT<=maxTOT,
   *    minADCOverTOT<=ADC/TOT<=maxACDOverTOT,
   *    if TOT==x, then ADC<=y (std::map)}.
   * The first super layer of the CDC can have specific requirements.
   * The other super layers of the CDC (outer super layers) share a single set of conditions.
   * In the allowed range for ADC, TOT, ..., an upper value of -1 stands
   * for unbounded. For example, the ADC range {5,-1} accepts all the
   * hits with ADC>=5.
   */
  class CDCWireHitRequirements: public TObject {

    /**
     *  Integer range. The first value is the lower bound, the second value is the upper bound.
     *  The two bounds are included in the allowed range.
     *  An upper value of -1 stands for unbounded.
     */
    typedef std::pair<short, short> intRange_t;
    /**
     *  Float range. The first value is the lower bound, the second value is the upper bound.
     *  The two bounds are included in the allowed range.
     *  An upper value of -1 stands for unbounded.
     */
    typedef std::pair<float, float> floatRange_t;

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
          m_MaxADCGivenTOTFirstSuperLayer.insert(std::make_pair(row.second.get<short>("givenTOT"),
                                                                row.second.get<short>("maxADC")));
        }
        for (boost::property_tree::ptree::value_type& row : tree.get_child("OuterSuperLayers.MaxADCGivenTOT")) {
          m_MaxADCGivenTOTOuterSuperLayers.insert(std::make_pair(row.second.get<short>("givenTOT"),
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

    /// Get allowed ADC range (first super layer of CDC). An upper value of -1 stands for unbounded
    const intRange_t& getADCRangeFirstSuperLayer() const
    {
      return m_ADCRangeFirstSuperLayer;
    }

    /// Set allowed ADC range (first super layer of CDC). An upper value of -1 stands for unbounded.
    void setADCRangeFirstSuperLayer(const intRange_t& ADCRangeFirstSuperLayer)
    {
      m_ADCRangeFirstSuperLayer =  ADCRangeFirstSuperLayer;
    }

    /// Get allowed ADC range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    const intRange_t& getADCRangeOuterSuperLayers() const
    {
      return m_ADCRangeOuterSuperLayers;
    }

    /// Set allowed ADC range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    void setADCRangeOuterSuperLayers(const intRange_t& ADCRangeOuterSuperLayers)
    {
      m_ADCRangeOuterSuperLayers =  ADCRangeOuterSuperLayers;
    }

    /// Get allowed TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    const intRange_t& getTOTRangeFirstSuperLayer() const
    {
      return m_TOTRangeFirstSuperLayer;
    }

    /// Set allowed TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    void setTOTRangeFirstSuperLayer(const intRange_t& TOTRangeFirstSuperLayer)
    {
      m_TOTRangeFirstSuperLayer =  TOTRangeFirstSuperLayer;
    }

    /// Get allowed TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    const intRange_t& getTOTRangeOuterSuperLayers() const
    {
      return m_TOTRangeOuterSuperLayers;
    }

    /// Set allowed TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    void setTOTRangeOuterSuperLayers(const intRange_t& TOTRangeOuterSuperLayers)
    {
      m_TOTRangeOuterSuperLayers =  TOTRangeOuterSuperLayers;
    }

    /// Get allowed ADC/TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    const floatRange_t& getADCOverTOTRangeFirstSuperLayer() const
    {
      return m_ADCOverTOTRangeFirstSuperLayer;
    }

    /// Set allowed ADC/TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    void setADCOverTOTRangeFirstSuperLayer(const floatRange_t& ADCOverTOTRangeFirstSuperLayer)
    {
      m_ADCOverTOTRangeFirstSuperLayer =  ADCOverTOTRangeFirstSuperLayer;
    }

    /// Get allowed ADC/TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    const floatRange_t& getADCOverTOTRangeOuterSuperLayers() const
    {
      return m_ADCOverTOTRangeOuterSuperLayers;
    }

    /// Set allowed ADC/TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    void setADCOverTOTRangeOuterSuperLayers(const floatRange_t& ADCOverTOTRangeOuterSuperLayers)
    {
      m_ADCOverTOTRangeOuterSuperLayers = ADCOverTOTRangeOuterSuperLayers;
    }

    /// Get map of maximum allowed ADC given TOT (first super layer of CDC). An upper value of -1 stands for unbounded.
    const std::map<short, short> getMaxADCGivenTOTFirstSuperLayer() const
    {
      return m_MaxADCGivenTOTFirstSuperLayer;
    }

    /// Get maximum allowed ADC given TOT (first super layer of CDC). An upper value of -1 stands for unbounded.
    short getMaxADCGivenTOTFirstSuperLayer(short tot) const
    {
      // Find maxADC given TOT
      const auto it = m_MaxADCGivenTOTFirstSuperLayer.find(tot);

      // Check if TOT exists in map or not
      if (it != m_MaxADCGivenTOTFirstSuperLayer.end()) {
        return it->second;
      } else {
        // By convention, -1 stands for unbounded
        return -1;
      }
    }

    /// Set map of maximum allowed ADC given TOT (first super layer of CDC). An upper value of -1 stands for unbounded.
    void setMaxADCGivenTOTFirstSuperLayer(const std::map<short, short>& MaxADCGivenTOTFirstSuperLayer)
    {
      m_MaxADCGivenTOTFirstSuperLayer = MaxADCGivenTOTFirstSuperLayer;
    }

    /// Get map of maximum allowed ADC given TOT (outer super layers of CDC). An upper value of -1 stands for unbounded.
    const std::map<short, short> getMaxADCGivenTOTOuterSuperLayers() const
    {
      return m_MaxADCGivenTOTOuterSuperLayers;
    }

    /// Get maximum allowed ADC given TOT (outer super layers of CDC). An upper value of -1 stands for unbounded.
    short getMaxADCGivenTOTOuterSuperLayers(short tot) const
    {
      // Find maxADC given TOT
      const auto it = m_MaxADCGivenTOTOuterSuperLayers.find(tot);

      // Check if TOT exists in map or not
      if (it != m_MaxADCGivenTOTOuterSuperLayers.end()) {
        return it->second;
      } else {
        // By convention, -1 stands for unbounded
        return -1;
      }
    }

    /// Set map of maximum allowed ADC given TOT (outer super layers of CDC). An upper value of -1 stands for unbounded.
    void setMaxADCGivenTOTOuterSuperLayers(const std::map<short, short>& MaxADCGivenTOTOuterSuperLayers)
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

    /// Allowed ADC range (first super layer of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    intRange_t m_ADCRangeFirstSuperLayer  = {1, -1};
    /// Allowed ADC range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    intRange_t m_ADCRangeOuterSuperLayers = {1, -1};

    /// Allowed TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    intRange_t m_TOTRangeFirstSuperLayer = {1, -1};
    /// Allowed TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    intRange_t m_TOTRangeOuterSuperLayers = {1, -1};

    /// Allowed ADC/TOT range (first super layer of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    floatRange_t m_ADCOverTOTRangeFirstSuperLayer = {0.0, -1.0};
    /// Allowed ADC/TOT range (outer super layers of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the lower bound, the second value is the upper bound.
    /// The two bounds are included in the allowed range.
    floatRange_t m_ADCOverTOTRangeOuterSuperLayers = {0.0, -1.0};

    /// Map of conditions of the type: if TOT==x, then ADC<=y (first super layer of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the given TOT, the second value is the maximum ADC.
    /// The upper bound is included in the allowed range.
    std::map<short, short> m_MaxADCGivenTOTFirstSuperLayer = {};
    /// Map of conditions of the type: if TOT==x, then ADC<=y (outer super layers of CDC). An upper value of -1 stands for unbounded.
    /// The first value is the given TOT, the second value is the maximum ADC.
    /// The upper bound is included in the allowed range.
    std::map<short, short> m_MaxADCGivenTOTOuterSuperLayers = {};

    /** ROOT ClassDef */
    ClassDef(CDCWireHitRequirements, 2);
  };

} // end namespace Belle2
