/**
 * @file
 * @author Jan Bauer (jbauer@ekp.uni-karlsruhe.de)
 * @section DESCRIPTION
 *
 *This Class represents angles in rad and grad. Operators grant that they stay
 *between 0 - 2 pi and 0 - 360 respectively.
 *
 */


#ifndef ANGLE_H
#define ANGLE_H

namespace Belle2 {
  namespace Tracking {
    class Angle {
    public:

      Angle();
      virtual ~Angle();
      Angle(float rad, bool isRad = true);

      float getDeg() const;
      float getRad() const;
      void setRad(float m_rad);
      void setDeg(float m_deg);

      Angle operator+(const Angle& rhs);
      Angle operator-(const Angle& rhs);
      Angle operator+(const float rhs);
      Angle operator-(const float rhs);

    private:
      float m_rad;
      float m_deg;

      void checkRad();  //checks if rad is between 0 and 2*pi
      void checkDeg();  //same for deg (0-360) // use on angle objects
      float checkRad(float rad);  //checks if rad is between 0 and 2*pi
      float checkDeg(float deg);  //same for deg (0-360)
      float radToDeg(float rad);  //convert rad to degrees
      float degToRad(float deg);  //and vice versa - used internally

    };
  }
}
#endif /* ANGLE_H_ */
