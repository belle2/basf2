/*
 * Colorizer.h
 *
 *  Created on: May 6, 2015
 *      Author: dschneider
 */

#ifndef COLORIZER_H_
#define COLORIZER_H_

#include <tracking/trackFindingCDC/display/ColorMaps.h>
#include <tracking/trackFindingCDC/display/CDCHitColorMaps.h>
#include <tracking/trackFindingCDC/display/CDCSegmentColorMaps.h>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Class template for Colorizer classes.
     *
     * This class is meant to work as a base class for interfaces to the ColorMap classes and
     * handles
     * the mapping from the colormapping-method name given as a string to the actual ColorMap class.
     * It needs to now the type of the Object to be drawn and the base ColorMap class for this
     * Object (defaults to CDCHitColorMap).
     */
    template <class AObject, class AColorMap = CDCHitColorMap>
    class Colorizer {

    private:
      /// A method pointer type of this Colorizer to access color maps.
      using MethodPtr = std::unique_ptr<AColorMap> (Colorizer::*)();

    public:
      /**
       * Default Constructor.
       *
       * Only needed in case there exists only one AColorMap class for this item. Initializes
       * m_listOfMappingNames with an empty map.
       */
      Colorizer()
        : m_strokeSet(false)
        , m_strokeWidthSet(false)
        , m_strokeMethodPtr(nullptr)
        , m_strokeWidthMethodPtr(nullptr)
        , m_listOfMappingNames( {})
      {
      }

      /**
       * Constructor.
       * @param map A string to MethodPtr map assigning a AColorMap class to its name given as a
       * string. \see m_listOfMappingNames
       */
      explicit Colorizer(const std::map<std::string, MethodPtr>& map)
        : m_strokeSet(false)
        , m_strokeWidthSet(false)
        , m_strokeMethodPtr(nullptr)
        , m_strokeWidthMethodPtr(nullptr)
        , m_listOfMappingNames(map)
      {
      }

      /**
       * Maps the object and its id to a color.
       */
      std::string mapStroke(const int objectID, const AObject& object)
      {
        return m_strokeMethodPtr->map(objectID, object);
      }

      /**
       * Maps the object and its id to a StrokeWidth.
       */
      std::string mapStrokeWidth(const int objectID, const AObject& object)
      {
        return m_strokeWidthMethodPtr->map(objectID, object);
      }

      /**
       * Checks if the stroke-method is set.
       */
      bool isStrokeSet()
      {
        return m_strokeSet;
      }

      /**
       * Checks if the strokeWidth-method is set.
       */
      bool isStrokeWidthSet()
      {
        return m_strokeWidthSet;
      }

      /**
       * Sets the method on how to match a color to a stroke.
       * @param methodName  The Name of the AColorMap class to be used. The translation from name
       *                    to AColorMap class must be defined in m_listOfMappingNames. If there is
       *                    no such translation, it is assumed that methodName already is the color
       *                    to be used.
       */
      void setStroke(const std::string& methodName)
      {
        m_strokeSet = true;
        if (m_listOfMappingNames.count(methodName) == 1) {
          m_strokeMethodPtr = (this->*m_listOfMappingNames[methodName])();
        } else {
          m_strokeMethodPtr =
            std::unique_ptr<AColorMap>(new ReturnInputString<AObject, AColorMap>(methodName));
        }
      }

      /**
       * Sets the method defining the stroke-width to be used.
       * @param methodName  The Name of the AColorMap class to be used. The translation from name
       *                    to AColorMap class must be defined in m_listOfMappingNames. If there is
       *                    no such translation, it is assumed that methodName already is the
       * stroke-width
       *                    to be used.
       */
      void setStrokeWidth(const std::string& methodName)
      {
        m_strokeWidthSet = true;
        if (m_listOfMappingNames.count(methodName) == 1) {
          m_strokeWidthMethodPtr = (this->*m_listOfMappingNames[methodName])();
        } else {
          m_strokeWidthMethodPtr =
            std::unique_ptr<AColorMap>(new ReturnInputString<AObject, AColorMap>(methodName));
        }
      }

      /**
       * Returns information on the translation from object and object id to color.
       */
      std::string printStrokeAttribute()
      {
        try {
          return m_strokeMethodPtr->info();
        } catch (...) {
          return "Could not print stroke attribute";
        }
      }

      /**
       * Returns information on the translation from object and object id to color.
       */
      std::string printStrokeWidthAttribute()
      {
        return m_strokeWidthMethodPtr->info();
      }

    protected:
      /**
       * Construct and return a pointer to the AColorMap.
       */
      template <class T>
      std::unique_ptr<AColorMap> constructMappingClass()
      {
        std::unique_ptr<AColorMap> mappingClass(new T);
        return (mappingClass);
      }

      /// Flag indicating if the stroke attribute method has been set
      bool m_strokeSet;

      /// Flag indicating if the strokeWidth attribute method has been set
      bool m_strokeWidthSet;

      /// Pointer to the method to determine the stroke
      std::unique_ptr<AColorMap> m_strokeMethodPtr;

      /// Pointer to the method to determine the strokeWidth
      std::unique_ptr<AColorMap> m_strokeWidthMethodPtr;

      /**
       * A string to pointer-to-member-function map.
       *
       * It maps the Name of a AColorMap to the corresponding constructMappingClass function.
       */
      std::map<std::string, MethodPtr> m_listOfMappingNames;
    };

    /**
     * Class template for coloring objects with colors defined manually with a string.
     */
    template <class AObject>
    class InputValueColorizer : public Colorizer<AObject, ReturnInputValue<AObject>> {
    public:
      /**
       * Sets the color to stroke.
       */
      void setStroke(std::string stroke)
      {
        Colorizer<AObject, ReturnInputValue<AObject>>::m_strokeSet = true;
        Colorizer<AObject, ReturnInputValue<AObject>>::m_strokeMethodPtr =
                                                     std::unique_ptr<ReturnInputValue<AObject>>(new ReturnInputValue<AObject>(stroke));
      }

      /**
       * Sets the stroke-width to strokeWidth.
       */
      void setStrokeWidth(std::string strokeWidth)
      {
        Colorizer<AObject, ReturnInputValue<AObject>>::m_strokeWidthSet = true;
        Colorizer<AObject, ReturnInputValue<AObject>>::m_strokeWidthMethodPtr =
                                                     std::unique_ptr<ReturnInputValue<AObject>>(new ReturnInputValue<AObject>(strokeWidth));
      }
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the
     * actual AColorMap for CDCSegments.
     */
    class CDCSegmentColorizer : public Colorizer<CDCRecoSegment2D, CDCSegmentColorMap> {
    public:
      /**
       * Constructor.
       *
       * Defines the name-to-AColorMap map for CDCSegments.
       */
      CDCSegmentColorizer();
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the
     * actual AColorMap for CDCHits.
     */
    class CDCHitColorizer : public Colorizer<CDCHit, CDCHitColorMap> {
    public:
      /**
       * Constructor.
       *
       * Defines the name-to-AColorMap map for CDCHits.
       */
      CDCHitColorizer();
    };

    /**
     * Class template for coloring objects with colors from ListColorsColorMap.
     */
    template <class AObject>
    class ListColorsColorizer : public Colorizer<AObject, ListColorsColorMap<AObject>> {
    public:
      /**
       * Constructor.
       *
       * Sets the Method on how to match a color to a stroke to ListColorsColorMap.
       */
      ListColorsColorizer()
        : Colorizer<AObject, ListColorsColorMap<AObject>>()
      {
        Colorizer<AObject, ListColorsColorMap<AObject>>::m_strokeMethodPtr =
                                                       std::unique_ptr<ListColorsColorMap<AObject>>(new ListColorsColorMap<AObject>());
        Colorizer<AObject, ListColorsColorMap<AObject>>::m_strokeSet = true;
      }
    };
  } // TrackFindingCDC
} // Belle2

#endif /* COLORIZER_H_ */
