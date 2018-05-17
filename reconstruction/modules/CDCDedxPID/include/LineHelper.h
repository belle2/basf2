/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Const.h>

#include <cmath>
#include <iostream>
#include <vector>

#define PI 3.14159265
#define PI_2 1.57079632679

namespace Belle2 {

  /** A collection of classes that are useful for making a simple path
   * length correction to the dE/dx measurement for each hit in a
   * CDC cell */

  /** A class to hold the coordinates of a point
   *
   * This is useful for implementation of the DedxLine class below, which is
   * helpful for track path length corrections.
   */
  class DedxPoint {

  public:

    /** The default constructor sets the coordinates to zero. */
    DedxPoint() : m_x(0), m_y(0), m_valid(true) {}

    /** Construct a point DedxPoint object from x and y coordinates. */
    DedxPoint(double x, double y) : m_x(x), m_y(y), m_valid(true) {}

    /** Helper function to return the x-coordinates of a DedxPoint */
    double getX() const { return m_x; }

    /** Helper function to return the y-coordinates of a DedxPoint */
    double getY() const { return m_y; }

    /** Set the x and y coordinates of a DedxPoint */
    void setPoint(double x, double y)
    {
      m_x = x;
      m_y = y;
    }

    /** Calculate the distance between this and another point */
    double length(const DedxPoint& p)
    {
      // horizontal line
      if (m_y == p.getY()) return std::abs(m_x - p.getX());
      // vertical line
      else if (m_x == p.getX()) return std::abs(m_y - p.getY());
      // else use Pythagorean Theorem
      else return std::sqrt((m_x - p.getX()) * (m_x - p.getX()) +
                              (m_y - p.getY()) * (m_y - p.getY()));
    }

    /** Check whether this point lies within the endpoints of a line */
    bool isValid() const { return m_valid; }

    /** Mark this point as invalid (exists outside a DedxDriftCell) */
    void setInvalid() { m_valid = false; }

  private:

    double m_x; /**< the x-coordinate of the DedxPoint */
    double m_y; /**< the y-coordinate of the DedxPoint */
    bool m_valid; /**< used to check if the point lies inside a boundary */

  };

  /** A class to hold the endpoints and slope of a line.
   *
   * This class is useful in determining the path length of a track through
   * a CDC cell.
   */
  class DedxLine {

  public:

    /** A line is definied either by a point and a slope, or by endpoints.
     * If a single point is given, both endpoints are given by the same
     * point (length = 0). */

    /** Default constructor */
    DedxLine(): m_m(0), m_vertical(false) {}

    /** Construct a DedxLine from the given DedxPoint and slope */
    DedxLine(const DedxPoint& p, const double slope) : m_p1(p), m_p2(p), m_m(slope), m_vertical(false) {}

    /** Construct a DedxLine from two DedxPoints */
    DedxLine(const DedxPoint& p1, const DedxPoint& p2) : m_p1(p1), m_p2(p2), m_vertical(false)
    {
      //check if the line is vertical (set vertical flag if it is)
      if (p1.getX() == p2.getX()) {
        m_m = -10000.0;
        m_vertical = true;
      } else
        m_m = (p1.getY() - p2.getY()) / (p1.getX() - p2.getX());
    }

    /** Helper function to return the slope */
    double getSlope() const { return m_m; }

    /** Find the intersection of this and another line */
    DedxPoint intersection(const DedxLine& l)
    {
      double x, y;
      DedxPoint intP;

      // First check if the lines will intersect.
      // Also check for special cases of horizontal or
      // vertical lines.
      if (m_m == l.m_m) {
        B2WARNING("Lines are parallel, will not intersect...");
        intP.setInvalid();
        intP.setPoint(0, 0);
        return intP;
      } else if (m_m == 0.0) {
        y = m_p1.getY();
        x = (y - l.m_p1.getY()) / l.m_m + l.m_p1.getX();
      } else if (l.m_m == 0.0) {
        y = l.m_p1.getY();
        x = (y - m_p1.getY()) / m_m + m_p1.getX();
      } else if (m_vertical) {
        x = m_p1.getX();
        y = l.m_p1.getY() + l.m_m * (x - l.m_p1.getX());
      } else if (l.isVertical()) {
        x = l.m_p1.getX();
        y = m_p1.getY() + m_m * (x - m_p1.getX());
      } else {
        x = ((l.m_p1.getY() - l.m_m * l.m_p1.getX()) - (m_p1.getY() - m_m * m_p1.getX())) / (m_m - l.m_m);
        y = m_p1.getY() + m_m * (x - m_p1.getX());
      }

      // This is the point of intersection
      intP.setPoint(x, y);

      // Check if the point falls outside the endpoints of the line.
      // This is useful for calculating the track length (dx).
      if ((x > m_p1.getX() && x > m_p2.getX()) ||
          (x < m_p1.getX() && x < m_p2.getX()))
        intP.setInvalid();
      if ((y > m_p1.getY() && y > m_p2.getY()) ||
          (y < m_p1.getY() && y < m_p2.getY()))
        intP.setInvalid();

      // Check if the DedxLine was made from a point and make sure the
      // point is not outside the cell
      if (l.m_p1.getX() == l.m_p2.getX() && l.m_p1.getY() == l.m_p2.getY() &&
          ((l.m_p1.getX() > m_p1.getX() && l.m_p1.getX() > m_p2.getX()) ||
           (l.m_p1.getX() < m_p1.getX() && l.m_p1.getX() < m_p2.getX())) &&
          ((l.m_p1.getY() > m_p1.getY() && l.m_p1.getY() > m_p2.getY()) ||
           (l.m_p1.getY() < m_p1.getY() && l.m_p1.getY() < m_p2.getY())))
        intP.setInvalid();

      return intP;
    }

    /** The m_vertical flag is used for the special case of a vertical line */
    bool isVertical() const { return m_vertical; }

    /** Mark this DedxLine as being vertical */
    void setVertical() { m_vertical = true; }

  private:

    DedxPoint m_p1 ;    /**< First endpoint */
    DedxPoint m_p2;     /**< Second endpoint */
    double m_m;     /**< Slope of the line */
    int m_vertical; /**< If the line is vertical, this is set to true. */

  };

  /** A class to hold the geometry of a cell
   *
   * This class is used to actually calculate the path length of a track
   * moving through a given cell.
   */
  class DedxDriftCell {

  public:

    /** Construct a DedxDriftCell from four different DedxLines (sides) */
    DedxDriftCell(const DedxLine& left, const DedxLine& top, const DedxLine& right, const DedxLine& bot) : m_Left(left), m_Top(top),
      m_Right(right), m_Bot(bot), m_isValid(true) {}

    /** Construct a DedxDriftCell from four different DedxPoints (corners) */
    DedxDriftCell(const DedxPoint& tl, const DedxPoint& tr, const DedxPoint& br, const DedxPoint& bl) :  m_Left(bl, tl), m_Top(tl, tr),
      m_Right(tr, br), m_Bot(br, bl), m_isValid(true) {}

    /** Check if this is a valid calculation (number of intersections = 2) */
    bool isValid() { return m_isValid; }

    /** Calculate the path length through this cell for a track with a given
     * DedxPoint Of Closest Approach (poca) and entrance angle (entAng) */
    double dx(const DedxPoint& poca, double entAng)
    {
      // The path length (dx) is the length of the track in this cell.
      double Dx = 0;

      // The DedxPoint Of Closest Approach (poca) is useful for a reference point
      // to construct a line that represents the track.
      DedxLine track = DedxLine(poca, std::tan(PI_2 - entAng));

      // Find the points of intersection with each cell boundary
      DedxPoint intLeft = m_Left.intersection(track);
      DedxPoint intTop = m_Top.intersection(track);
      DedxPoint intRight = m_Right.intersection(track);
      DedxPoint intBot = m_Bot.intersection(track);

      std::vector<DedxPoint> endpoints;
      if (intLeft.isValid())
        endpoints.push_back(intLeft);
      if (intTop.isValid())
        endpoints.push_back(intTop);
      if (intRight.isValid())
        endpoints.push_back(intRight);
      if (intBot.isValid())
        endpoints.push_back(intBot);

      // Make sure we only get two intersections!
      if (endpoints.size() == 2) {
        Dx = endpoints[0].length(endpoints[1]);
        if (Dx == 0)
          m_isValid = false;
      } else
        m_isValid = false;

      return Dx;
    }

    /** Calculate the path length through this cell for a track with a given
     * Distance Of Closest Approach (doca) and entrance angle (entAng) */
    double dx(double doca, double entAng)
    {
      // The path length (dx) is the length of the track in this cell.
      double Dx = 0;

      // The DedxPoint Of Closest Approach (poca) is useful for a reference point
      // to construct a line that represents the track.
      const DedxPoint poca = DedxPoint(doca * std::abs(std::cos(entAng)), -1.0 * doca * std::sin(entAng));
      DedxLine track = DedxLine(poca, std::tan(PI_2 - entAng));

      // Find the points of intersection with each cell boundary
      DedxPoint intLeft = m_Left.intersection(track);
      DedxPoint intTop = m_Top.intersection(track);
      DedxPoint intRight = m_Right.intersection(track);
      DedxPoint intBot = m_Bot.intersection(track);

      std::vector< DedxPoint > endpoints;
      if (intLeft.isValid())
        endpoints.push_back(intLeft);
      if (intTop.isValid())
        endpoints.push_back(intTop);
      if (intRight.isValid())
        endpoints.push_back(intRight);
      if (intBot.isValid())
        endpoints.push_back(intBot);

      // Make sure we only get two intersections!
      if (endpoints.size() == 2) {
        Dx = endpoints[0].length(endpoints[1]);
        if (Dx == 0)
          m_isValid = false;
      } else
        m_isValid = false;

      return Dx;
    }

  private:

    DedxLine m_Left;    /**< the left boundary of the cell */
    DedxLine m_Top;     /**< the left boundary of the cell */
    DedxLine m_Right;   /**< the left boundary of the cell */
    DedxLine m_Bot;     /**< the left boundary of the cell */
    bool m_isValid; /**< does the hit land in this cell */
  };
} // Belle2 namespace
