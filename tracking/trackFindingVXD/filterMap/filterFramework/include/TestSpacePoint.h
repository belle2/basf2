/** helper struct for testing purposes providing the necessary coordinate accessors
 * NOTE: this is only temporaryly stored in a separate header!
 */
#pragma once

namespace VXDTFFilterTest {
  struct TestSpacePoint {
    TestSpacePoint(double x, double y, double z) : x(x), y(y), z(z) { ; }
    double x, y, z;
    double X() const { return x; }
    double Y() const { return y; }
    double Z() const { return z; }
  };
}
