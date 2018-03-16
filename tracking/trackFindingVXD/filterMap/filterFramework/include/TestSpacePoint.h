/** helper struct for testing purposes providing the necessary coordinate accessors
 * NOTE: this is only temporaryly stored in a separate header!
 */
#pragma once

namespace VXDTFFilterTest {
  /// Helper struct for SpacePoint Tests
  struct TestSpacePoint {
    /** Constructor
     * @param x : x coordiante
     * @param y : y coordiante
     * @param z : z coordiante
     */
    TestSpacePoint(double x, double y, double z) : x(x), y(y), z(z) { ; }

    double x; /**< x coordiante of the SP */
    double y; /**< y coordiante of the SP */
    double z; /**< z coordiante of the SP */

    /// Getter for x
    double X() const { return x; }
    /// Getter for y
    double Y() const { return y; }
    /// Getter for z
    double Z() const { return z; }
  };
}
