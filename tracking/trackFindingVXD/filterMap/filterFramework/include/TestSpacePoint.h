/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/** helper struct for testing purposes providing the necessary coordinate accessors
 * NOTE: this is only temporarily stored in a separate header!
 */
#pragma once

namespace VXDTFFilterTest {
  /// Helper struct for SpacePoint Tests
  struct TestSpacePoint {
    /** Constructor
     * @param x : x coordinate
     * @param y : y coordinate
     * @param z : z coordinate
     */
    TestSpacePoint(double x, double y, double z) : x(x), y(y), z(z) { ; }

    double x; /**< x coordinate of the SP */
    double y; /**< y coordinate of the SP */
    double z; /**< z coordinate of the SP */

    /// Getter for x
    double X() const { return x; }
    /// Getter for y
    double Y() const { return y; }
    /// Getter for z
    double Z() const { return z; }
  };
}
