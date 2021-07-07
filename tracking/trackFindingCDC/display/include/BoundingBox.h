/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <algorithm>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A two dimensional rectangle that keeps track of the extend of a drawing
    class BoundingBox {

    public:

      /// Default constructor for ROOT compatibility. Cell weight defaults to 1
      BoundingBox() :
        m_left(NAN),
        m_bottom(NAN),
        m_right(NAN),
        m_top(NAN)
      {}

      /// Initialises the bounds of the rectangle to the given values. Automatically swaps the x or y values if they are given in a decreasing.
      BoundingBox(float x1, float y1, float x2, float y2) :
        m_left(std::min(x1, x2)),
        m_bottom(std::min(y1, y2)),
        m_right(std::max(x1, x2)),
        m_top(std::max(y1, y2))
      {}

      /// Expands the bounding box such that it also covers the given bounding box
      void operator&=(const BoundingBox& other)
      {

        m_left = std::isnan(getLeft()) ? other.getLeft() : std::min(getLeft(), other.getLeft());
        m_bottom = std::isnan(getBottom()) ? other.getBottom() : std::min(getBottom(), other.getBottom());

        m_right = std::isnan(getRight()) ? other.getRight() : std::max(getRight(), other.getRight());
        m_top = std::isnan(getTop()) ? other.getTop() : std::max(getTop(), other.getTop());

      }

      /// Expands the rectangle in each direction by the given value delta.
      void expand(float delta)
      {
        m_left -= delta;
        m_bottom -= delta;
        m_right += delta;
        m_top += delta;
      }

      /// Getter for the width of the bounding box rectangle.
      float getWidth() const
      { return getRight() - getLeft(); }

      /// Getter for the height of the bounding box rectangle.
      float getHeight() const
      { return getTop() - getBottom(); }

      /// Getter for the location of the left of the bounding box rectangle (lower x bound). NAN if unset.
      float getLeft() const
      { return m_left; }

      /// Getter for the location of the bottom of the bounding box rectangle (lower y bound). NAN if unset.
      float getBottom() const
      { return m_bottom; }

      /// Getter for the location of the right of the bounding box rectangle (upper x bound). NAN if unset.
      float getRight() const
      { return m_right; }

      /// Getter for the location of the top of the bounding box rectangle (upper y bound). NAN if unset.
      float getTop() const
      { return m_top; }

      /// Clears all bounds to NAN.
      void clear()
      {
        m_left = NAN;
        m_bottom = NAN;
        m_right = NAN;
        m_top = NAN;
      }

    private:
      /// Memory for the lower x bound of the rectangle.
      float m_left;

      /// Memory for the lower y bound of the rectangle.
      float m_bottom;

      /// Memory for the upper x bound of the rectangle.
      float m_right;

      /// Memory for the upper y bound of the rectangle.
      float m_top;

    };

  }
}
