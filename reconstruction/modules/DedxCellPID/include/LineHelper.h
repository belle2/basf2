/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef LINEHELPER_H
#define LINEHELPER_H

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
   * This is useful for implementation of the Line class below, which is
   * helpful for track path length corrections.
   */
  class Point {

  public:

    /** The default constructor sets the coordinates to zero. */
    Point() : m_x(0), m_y(0), m_valid(true) {}

    /** Construct a point Point object from x and y coordinates. */
    Point(double x, double y) : m_x(x), m_y(y), m_valid(true) {}

    /** Helper function to return the x-coordinates of a Point */
    double getX() const { return m_x; }

    /** Helper function to return the y-coordinates of a Point */
    double getY() const { return m_y; }

    /** Set the x and y coordinates of a Point */
    void setPoint(double x, double y) {
      m_x = x;
      m_y = y;
    }

    /** Calculate the distance between this and another point */
    double length(const Point& p) {
      // horizontal line
      if (m_y == p.getY()) return abs(m_x - p.getX());
      // vertical line
      else if (m_x == p.getX()) return abs(m_y - p.getY());
      // else use Pythagorean Theorem
      else return sqrt((m_x - p.getX()) * (m_x - p.getX()) +
                         (m_y - p.getY()) * (m_y - p.getY()));
    }

    /** Check whether this point lies within the endpoints of a line */
    bool isValid() const { return m_valid; }

    /** Mark this point as invalid (exists outside a Cell) */
    void setInvalid() { m_valid = false; }

  private:

    double m_x, m_y; /** The coordinates of the Point */
    bool m_valid;   /** Used to check if the point lies inside a boundary */

  };


  /** A class to hold the endpoints and slope of a line.
   *
   * This class is useful in determining the path length of a track through
   * a CDC cell.
   */
  class Line {

  public:

    /** A line is definied either by a point and a slope, or by endpoints.
     * If a single point is given, both endpoints are given by the same
     * point (length = 0). */

    /** Default constructor */
    Line(): m_m(0), m_vertical(false) {}

    /** Construct a Line from the given Point and slope */
    Line(const Point& p, const double slope) : m_p1(p), m_p2(p), m_m(slope), m_vertical(false) {}

    /** Construct a Line from two Points */
    Line(const Point& p1, const Point& p2) : m_p1(p1), m_p2(p2), m_vertical(false) {
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
    Point intersection(const Line& l) {
      double x, y;
      Point intP;

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

      // Check if the point falls outside the endpoints of each line.
      // This is useful for calculating the track length (dx).
      if ((x > m_p1.getX() && x > m_p2.getX()) ||
          (x < m_p1.getX() && x < m_p2.getX()))
        intP.setInvalid();
      if ((y > m_p1.getY() && y > m_p2.getY()) ||
          (y < m_p1.getY() && y < m_p2.getY()))
        intP.setInvalid();
      return intP;
    }

    /** The m_vertical flag is used for the special case of a vertical line */
    bool isVertical() const { return m_vertical; }

    /** Mark this Line as being vertical */
    void setVertical() { m_vertical = true; }

  private:

    Point m_p1;      /** First endpoint */
    Point m_p2;      /** Second endpoint */
    double m_m;      /** Slope of the line */
    int m_vertical;  /** If the line is vertical, this is set to true. */

  };


  /** A class to hold the geometry of a cell
   *
   * This class is used to actually calculate the path length of a track
   * moving through a given cell.
   */
  class Cell {

  public:

    /** Construct a Cell from four different Lines (sides) */
    Cell(const Line left, const Line top, const Line right, const Line bot) : m_Left(left), m_Top(top), m_Right(right), m_Bot(bot), m_isValid(true) {}

    /** Construct a Cell from four different Points (corners) */
    Cell(const Point tl, const Point tr, const Point br, const Point bl) {
      m_Left = Line(bl, tl);
      m_Top = Line(tl, tr);
      m_Right = Line(tr, br);
      m_Bot = Line(br, bl);
      m_isValid = true;
    }

    /** Check if this is a valid calculation (number of intersections = 2) */
    bool isValid() { return m_isValid; }

    /** Calculate the path length through this cell for a track with a given
     * Point Of Closest Approach (poca) and entrance angle (entAng) */
    double dx(const Point poca, double entAng) {
      // The path length (dx) is the length of the track in this cell.
      double Dx = 0;

      // The Point Of Closest Approach (poca) is useful for a reference point
      // to construct a line that represents the track.
      Line track = Line(poca, tan(PI_2 - entAng));

      // Find the points of intersection with each cell boundary
      Point intLeft = m_Left.intersection(track);
      Point intTop = m_Top.intersection(track);
      Point intRight = m_Right.intersection(track);
      Point intBot = m_Bot.intersection(track);

      // Make sure we only get two intersections! The function isValid() returns
      // false if the intersection occurs outside the cell.
      int index = 0;
      std::vector< Point > endpoints;
      if (intLeft.isValid()) {
        endpoints.push_back(intLeft); index++;
      }
      if (intTop.isValid()) {
        endpoints.push_back(intTop); index++;
      }
      if (intRight.isValid()) {
        endpoints.push_back(intRight); index++;
      }
      if (intBot.isValid()) {
        endpoints.push_back(intBot); index++;
      }

      Dx = endpoints[0].length(endpoints[1]);

      if (index != 2 || Dx == 0)
        m_isValid = false;

      return Dx;
    }

    /** Calculate the path length through this cell for a track with a given
     * Distance Of Closest Approach (doca) and entrance angle (entAng) */
    double dx(double doca, double entAng) {
      // The path length (dx) is the length of the track in this cell.
      double Dx = 0;

      // The Point Of Closest Approach (poca) is useful for a reference point
      // to construct a line that represents the track.
      const Point poca = Point(doca * TMath::Abs(cos(entAng)), -1.0 * doca * sin(entAng));
      Line track = Line(poca, tan(PI_2 - entAng));

      // Find the points of intersection with each cell boundary
      Point intLeft = m_Left.intersection(track);
      Point intTop = m_Top.intersection(track);
      Point intRight = m_Right.intersection(track);
      Point intBot = m_Bot.intersection(track);

      // Make sure we only get two intersections! The function isValid() returns
      // false if the intersection occurs outside the cell.
      int index = 0;
      std::vector< Point > endpoints;
      if (intLeft.isValid()) {
        endpoints.push_back(intLeft); index++;
      }
      if (intTop.isValid()) {
        endpoints.push_back(intTop); index++;
      }
      if (intRight.isValid()) {
        endpoints.push_back(intRight); index++;
      }
      if (intBot.isValid()) {
        endpoints.push_back(intBot); index++;
      }

      if (index != 2)
        m_isValid = false;
      else
        Dx = endpoints[0].length(endpoints[1]);

      if (Dx == 0)
        m_isValid = false;

      return Dx;
    }

  private:

    Line m_Left;    /** the left boundary of the cell */
    Line m_Top;     /** the left boundary of the cell */
    Line m_Right;   /** the left boundary of the cell */
    Line m_Bot;     /** the left boundary of the cell */
    bool m_isValid; /** does the hit land in this cell */

  };
}
#endif
