/*
 * ColorMaps.h
 *
 *  Created on: Feb 26, 2015
 *      Author: dschneider
 */
#pragma once

#include <string>
#include <vector>


namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     * Contains functions to convert Colors given in the HLS-system into the RGB-system.
     */
    class HLSToRGB {
    public:
      /**
       * Transforms a Color given in the HLS System to RGB.
       */
      static double hueToRgb(const double p, const double q, double t);

      /**
       * Transforms a Color given in the HLS System to RGB.
       *
       * @param h Hue ranging from 0 to 1.
       * @param l Lighness.
       * @param s Saturation.
       * @return Vector containing rgb-values in the order red, green, blue.
       */
      static std::vector<double> hlsToRgb(const double h, const double l, const double s);
    };

    /**
     * Wrapper Class around std::vector<std::string>.
     *
     * Imitates the behavior of std::vector<std::string>.
     */ //Only needed in header because of ListColorsColorMap.
    class ListColors {
    public:

      /**
       * Constructor.
       *
       * Initializes m_listColors with different Colors (strings) and stores its size in m_length.
       */
      ListColors(): m_listColors(
      {"red",
        "blue",
        "green",
        "orange",
        "cyan",
        "olive",
        "lime",
        "maroon",
        "tomato",
        "turquoise",
        "mediumspringgreen",
        "darkgreen",
        "indigo",
      })
      {};

      /**
       * Index operator.
       */
      std::string operator[](const int index) const
      {
        return m_listColors[index % size()];
      }

      /**
       * Returns the length of m_listColors.
       */
      int size() const
      {
        return m_listColors.size();
      }

      /**
       * Returns an Iterator pointing to the first element in m_listColors.
       */
      std::vector<std::string>::iterator begin()
      {
        return m_listColors.begin();
      }

      /**
       * Returns an Iterator pointing to the past-the-end element in m_listColors.
       */
      std::vector<std::string>::iterator end()
      {
        return m_listColors.end();
      }

    private:
      /// List of colors to be cycled through
      std::vector<std::string> m_listColors;
    };

    /**
     * Class template for coloring objects in different Colors.
     *
     * This Class uses the color list defined in the ListColors class.
     */
    template<class AObject>
    class ListColorsColorMap {
    public:
      virtual ~ListColorsColorMap() = default;

      /**
       * Function call to map the AObject id and object to a color.
       */
      virtual std::string map(const int index, const AObject&)
      {
        return m_listColors[index % m_listColors.size()];
      }

      /**
       * Returns an informal string listing the colors defined in ListColors.
       */
      std::string info()
      {
        std::string message = "[";
        for (std::vector<std::string>::iterator colorIterator = m_listColors.begin();
             colorIterator != --(m_listColors.end()); ++colorIterator) {
          message += *(colorIterator) + ", ";
        }
        message += *(--(m_listColors.end()));
        message += "]";
        return message;
      }

    private:
      /// List of colors to be cycled through
      ListColors m_listColors;
    };

    /**
     * Class template for returning a string defined on instantiation.
     *
     * Used for explicitly defined drawing options, like "blue", "red" (in case of colors) or "0.2" (in case of stroke-width).
     *
     * Template arguments define the object-type and the Base ColorMapClass used for this object.
     */
    template<class AObject, class AColorMap>
    class ReturnInputString : public AColorMap {
    public:
      /**
       * Constructor initializes m_inputString with the given string.
       *
       * @param input The string to be returned by the map() function.
       */
      explicit ReturnInputString(const std::string& input) : m_inputString(input) {};

      /**
       * Function call to map the object id and object to a color/stroke-width.
       */
      std::string map(const int, const AObject&) override
      {
        return m_inputString;
      }

    private:
      /// Returned string on each invokation of the color map.
      std::string m_inputString;
    };

    /**
     * Class template for returning a string defined on instantiation.
     *
     * Used for explicitly defined drawing options, like "blue", "red" (in case of colors) or "0.2" (in case of stroke-width).
     *
     * Template argument defines the type of the object that is drawn.
     */
    template<class AObject>
    class ReturnInputValue {
    public:
      /**
       * Constructor initializes m_inputValue with the given string.
       *
       * @param input The string to be returned by the map() function.
       */
      explicit ReturnInputValue(const std::string& input) : m_inputValue(input) {};

      /**
       * Function call to map the object id and object to a color/stroke-width.
       */
      std::string map(const int, const AObject&) const
      {
        return m_inputValue;
      }

      /**
       * Informal string summarizing the translation variables to colors.
       */
      std::string info() const
      {
        return m_inputValue;
      }
    private:
      /// Returned string on each invokation of the map.
      std::string m_inputValue;
    };

  }
}

