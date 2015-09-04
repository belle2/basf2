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
     * This class is meant to work as a base class for interfaces to the ColorMap classes and handles
     * the mapping from the colormapping-method name given as a string to the actual ColorMap class.
     * It needs to now the type of the Object to be drawn and the base ColorMap class for this Object (defaults to CDCHitColorMap).
     */
    template <class Object, class ColorMap = CDCHitColorMap>
    class Colorizer {
      typedef std::unique_ptr<ColorMap>(Colorizer::*MethodPtr)();
    public:
      /**
       * Default Constructor.
       *
       * Only needed in case there exists only one ColorMap class for this item. Initializes m_listOfMappingNames with an empty map.
       */
      Colorizer(): m_strokeSet(false), m_strokeWidthSet(false), m_strokeMethodPtr(nullptr), m_strokeWidthMethodPtr(nullptr),
        m_listOfMappingNames( {}) {}

      /**
       * Constructor.
       * @param map A string to MethodPtr map assigning a ColorMap class to its name given as a string. \see m_listOfMappingNames
       */
      explicit Colorizer(std::map<std::string, MethodPtr> map): m_strokeSet(false), m_strokeWidthSet(false), m_strokeMethodPtr(nullptr),
        m_strokeWidthMethodPtr(nullptr),
        m_listOfMappingNames(map) {}

      /**
       * Maps the object and its id to a color.
       */
      std::string mapStroke(const int objectID, const Object& object)
      {
        return m_strokeMethodPtr->map(objectID, object);
      }

      /**
       * Maps the object and its id to a StrokeWidth.
       */
      std::string mapStrokeWidth(const int objectID, const Object& object)
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
       * @param methodName  The Name of the ColorMap class to be used. The translation from name
       *                    to ColorMap class must be defined in m_listOfMappingNames. If there is
       *                    no such translation, it is assumed that methodName already is the color
       *                    to be used.
       */
      void setStroke(const std::string& methodName)
      {
        m_strokeSet = true;
        if (m_listOfMappingNames.count(methodName) == 1) {
          m_strokeMethodPtr = (this->*m_listOfMappingNames[methodName])();
        } else m_strokeMethodPtr = std::unique_ptr<ColorMap>(new ReturnInputString<Object, ColorMap>(methodName));
      }

      /**
       * Sets the method defining the stroke-width to be used.
       * @param methodName  The Name of the ColorMap class to be used. The translation from name
       *                    to ColorMap class must be defined in m_listOfMappingNames. If there is
       *                    no such translation, it is assumed that methodName already is the stroke-width
       *                    to be used.
       */
      void setStrokeWidth(const std::string& methodName)
      {
        m_strokeWidthSet = true;
        if (m_listOfMappingNames.count(methodName) == 1) {
          m_strokeWidthMethodPtr = (this->*m_listOfMappingNames[methodName])();
        } else m_strokeWidthMethodPtr = std::unique_ptr<ColorMap>(new ReturnInputString<Object, ColorMap>(methodName));
      }

      /**
       * Returns information on the translation from object and object id to color.
       */
      std:: string printStrokeAttribute()
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
      std:: string printStrokeWidthAttribute()
      {
        return m_strokeWidthMethodPtr->info();
      }


    protected:
      /**
       * Construct and return a pointer to the ColorMap.
       */
      template<class T>
      std::unique_ptr<ColorMap> constructMappingClass()
      {
        std::unique_ptr<ColorMap> mappingClass(new T);
        return (mappingClass);
      }

      bool m_strokeSet;
      bool m_strokeWidthSet;
      std::unique_ptr<ColorMap> m_strokeMethodPtr;
      std::unique_ptr<ColorMap> m_strokeWidthMethodPtr;
      /**
       * A string to pointer-to-member-function map.
       *
       * It maps the Name of a ColorMap to the corresponding constructMappingClass function.
       */
      std::map<std::string, MethodPtr> m_listOfMappingNames;
    };

    /**
     * Class template for coloring objects with colors defined manually with a string.
     */
    template <class Object>
    class InputValueColorizer : public Colorizer<Object, ReturnInputValue<Object>> {
    public:
      /**
       * Sets the color to stroke.
       */
      void setStroke(std::string stroke)
      {
        Colorizer<Object, ReturnInputValue<Object>>::m_strokeSet = true;
        Colorizer<Object, ReturnInputValue<Object>>::m_strokeMethodPtr = std::unique_ptr<ReturnInputValue<Object>>
                                                 (new ReturnInputValue<Object>(stroke));
      }

      /**
       * Sets the stroke-width to strokeWidth.
       */
      void setStrokeWidth(std::string strokeWidth)
      {
        Colorizer<Object, ReturnInputValue<Object>>::m_strokeWidthSet = true;
        Colorizer<Object, ReturnInputValue<Object>>::m_strokeWidthMethodPtr = std::unique_ptr<ReturnInputValue<Object>>
                                                 (new ReturnInputValue<Object>(strokeWidth));
      }
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the actual ColorMap for CDCSegments.
     */
    class CDCSegmentColorizer : public Colorizer<CDCRecoSegment2D, CDCSegmentColorMap> {
    public:
      /**
       * Constructor.
       *
       * Defines the name-to-ColorMap map for CDCSegments.
       */
      CDCSegmentColorizer();
    };

    /**
     * This Class handles the mapping from the colormapping-method name given as a string to the actual ColorMap for CDCHits.
     */
    class CDCHitColorizer : public Colorizer<CDCHit, CDCHitColorMap> {
    public:
      /**
       * Constructor.
       *
       * Defines the name-to-ColorMap map for CDCHits.
       */
      CDCHitColorizer();
    };

    /**
     * Class template for coloring objects with colors from ListColorsColorMap.
     */
    template<class Object>
    class ListColorsColorizer : public Colorizer<Object, ListColorsColorMap<Object>> {
    public:
      /**
       * Constructor.
       *
       * Sets the Method on how to match a color to a stroke to ListColorsColorMap.
       */
      ListColorsColorizer() : Colorizer<Object, ListColorsColorMap<Object>>()
      {
        Colorizer<Object, ListColorsColorMap<Object>>::m_strokeMethodPtr = std::unique_ptr<ListColorsColorMap<Object>>
                                                   (new ListColorsColorMap<Object>());
        Colorizer<Object, ListColorsColorMap<Object>>::m_strokeSet = true;
      }
    };
  }//TrackFindingCDC
}//Belle2



#endif /* COLORIZER_H_ */
