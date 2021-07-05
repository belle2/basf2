/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/display/ColorMapping.h>
#include <tracking/trackFindingCDC/display/Mapping.h>
#include <tracking/trackFindingCDC/display/AttributeMap.h>


#include <map>
#include <sstream>
#include <memory>

namespace Belle2 {
  class RecoTrack;
  class CDCHit;

  namespace TrackFindingCDC {
    class CDCSegment2D;

    /// Interface for a mapping of object and an index to styling attributes
    template <class AObject>
    class Styling {

    public:
      /// Make destructor of interface virtual
      virtual ~Styling() = default;

      /// Maps the object at the given index to attribute values
      virtual AttributeMap map(int index __attribute__((unused)),
                               AObject& object __attribute__((unused)))
      {
        return {};
      }

      /// Informal string summarizing the translation from the object to the styling attributes.
      virtual std::string info()
      {
        return "(no info available)\n";
      }
    };

    /// Implementation of a styling from fixed attribute map.
    template <class AObject>
    class FixedStyling : public Styling<AObject> {

    public:
      /// Return the fixed attributes on each invocation
      AttributeMap map(int index __attribute__((unused)),
                       AObject& object __attribute__((unused))) override
      {
        return m_attributeMap;
      }

      /// Informal string summarizing the translation from the object to the styling attributes.
      std::string info() override
      {
        std::ostringstream oss;
        for (const auto& keyValue : m_attributeMap) {
          oss << keyValue.first << ": " << keyValue.second << '\n';
        }
        return oss.str();
      }

      /// Sets the given attribute to the fixed value
      void set(const std::string& key, const std::string& value)
      {
        m_attributeMap.emplace(key, value);
      }

      /// Legacy - Sets the stroke color to the fixed value
      void setStroke(const std::string& value)
      {
        set("stroke", value);
      }

      /// Legacy - Sets the stroke width to the fixed value
      void setStrokeWidth(const std::string& value)
      {
        set("stroke-width", value);
      }

    private:
      /// Memory for the fixed attribute values
      AttributeMap m_attributeMap;
    };

    /// Implementation of a styling composed from several predefined mappings chooseable by their name.
    template <class AObject>
    class ChooseableStyling : public Styling<AObject> {

    public:
      /// Mapping for the object type
      using ObjectMapping = Mapping<AObject>;

    public:
      /// Create a map of attributes from the stored attribute maps
      AttributeMap map(int index, AObject& object) override
      {
        AttributeMap attributeMap;
        for (const auto& keyMapping : m_mappingsByKey) {
          const std::string& key = keyMapping.first;
          const std::unique_ptr<ObjectMapping>& mapping = keyMapping.second;
          attributeMap[key] = mapping->map(index, object);
        }
        return attributeMap;
      }

      /// Returns informal string about the currently set mappings
      std::string info() override
      {
        std::ostringstream oss;
        for (const auto& keyMapping : m_mappingsByKey) {
          const std::string& key = keyMapping.first;
          const std::unique_ptr<ObjectMapping>& mapping = keyMapping.second;
          oss << key << ": " << mapping->info();
        }
        return oss.str();
      }

      /**
       *  Create a mapping for the object from a name.
       *
       *  Can be overriden by the subclass to introduce names with specific meanings.
       *  The default is to use the name as a fixed attribute value.
       */
      virtual std::unique_ptr<ObjectMapping> createMapping(const std::string& mappingName)
      {
        return std::make_unique<ConstantMapping<AObject>>(mappingName);
      }

      /// Sets the given attribute to the attribute mapping
      void set(const std::string& key, std::unique_ptr<ObjectMapping> mapping)
      {
        if (mapping) {
          m_mappingsByKey[key] = std::move(mapping);
        }
      }

      /// Sets the given attribute to the fixed value
      void set(const std::string& key, const std::string& mappingName)
      {
        std::unique_ptr<ObjectMapping> objectMapping = this->createMapping(mappingName);
        if (objectMapping) {
          set(key, std::move(objectMapping));
        }
      }

      /**
       * Legacy method to set the mapping on how to match a object to the stroke color.
       * @param mappingName The Name of the Mapping to be used. The translation from name
       *                    to Mapping class must be defined in createMapping.
       */
      void setStroke(const std::string& mappingName)
      {
        set("stroke", mappingName);
      }

      /**
       * Legacy method to set the mapping on how to match a object to the stroke width.
       * @param mappingName The Name of the Mapping to be used. The translation from name
       *                    to Mapping class must be defined in createMapping.
       */
      void setStrokeWidth(const std::string& mappingName)
      {
        set("stroke-width", mappingName);
      }

    protected:
      /// Map of attribute keys to mappings to be used.
      std::map<std::string, std::unique_ptr<ObjectMapping> > m_mappingsByKey;
    };

    /// Class template for coloring objects with stroke colors prepared to be the default color cycle.
    template <class AObject>
    class DefaultColorCycleStyling : public ChooseableStyling<AObject> {
    public:
      /// Constructor. Sets the stroke mapping the the default color cycle.
      DefaultColorCycleStyling() : ChooseableStyling<AObject>()
      {
        this->set("stroke", std::make_unique<DefaultColorCycleMapping<AObject>>());
      }
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the
     * actual color mapping for RecoTrack.
     */
    class ChooseableRecoTrackStyling : public ChooseableStyling<const RecoTrack> {

    private:
      /// Type of the base class
      using Super = ChooseableStyling<const RecoTrack>;

      /// Mapping to be constructed
      using Super::ObjectMapping;

    public:
      /// Method defining the available mapping names
      std::unique_ptr<ObjectMapping> createMapping(const std::string& mappingName) override;
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the
     * actual AColorMap for CDCSegments.
     */
    class ChooseableSegmentStyling : public ChooseableStyling<const CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = ChooseableStyling<const CDCSegment2D>;

      /// Mapping to be constructed
      using Super::ObjectMapping;

    public:
      /// Method defining the available mapping names
      std::unique_ptr<ObjectMapping> createMapping(const std::string& mappingName) override;
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the
     * actual color mapping for CDCHits.
     */
    class ChooseableHitStyling : public ChooseableStyling<const CDCHit> {

    private:
      /// Type of the base class
      using Super = ChooseableStyling<const CDCHit>;

      /// Mapping to be constructed
      using Super::ObjectMapping;

    public:
      /// Method defining the available mapping names
      std::unique_ptr<ObjectMapping> createMapping(const std::string& mappingName) override;
    };
  }
}
