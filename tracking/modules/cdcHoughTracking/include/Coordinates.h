#ifndef COORDINATES_H
#define COORDINATES_H

/*
 * Coordinates  in carthesian and polar form
 *
 *
 *
 */

#include <tracking/modules/cdcHoughTracking/Angle.h>
#include <functional>
namespace Belle2 {
  namespace Tracking {

    class Coordinates {
    public:
      Coordinates();
      Coordinates(double x, double y, bool isCarth = true);
      virtual ~Coordinates();

      const Angle& getPhi() const;
      double getR() const;
      double getX() const;
      double getY() const;
      void setPhi(double m_Phi);
      void setR(double m_R);
      void setX(double m_X);
      void setY(double m_Y);

    private:

      double m_X;
      double m_Y;
      double m_R;
      Angle m_Phi;
    };

//helper functor to sort Coordinates by radius, descending
    struct SortCoordinatesByRadius : public std::binary_function <Coordinates, Coordinates, bool> {
      bool operator()(const Coordinates& lhs, const Coordinates& rhs);
    };

  }
}
#endif /* COORDINATES_H */
