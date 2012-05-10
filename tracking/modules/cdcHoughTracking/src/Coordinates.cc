#include <tracking/modules/cdcHoughTracking/Coordinates.h>
#include <math.h>

using namespace Belle2;
using namespace std;

Tracking::Coordinates::Coordinates() :
  m_X(0), m_Y(0), m_R(0), m_Phi(Tracking::Angle(0))
{}

Tracking::Coordinates::Coordinates(double x, double y, bool isCarth)
{
  if (isCarth) {
    m_X   = x;
    m_Y   = y;
    m_R   = sqrt(pow(x, 2) + pow(y, 2));
    m_Phi = Tracking::Angle(atan2(y, x));
  } else {
    m_R = x;
    m_Phi = Tracking::Angle(y);
    m_X = m_R * cos(m_Phi.getRad());
    m_Y = m_R * sin(m_Phi.getRad());
  }

}

Tracking::Coordinates CoordinatesByRadial(double r, double phi)
{
  Tracking::Coordinates ret = Tracking::Coordinates();
  ret.setPhi(phi);
  ret.setR(r);
  return ret;
}


Tracking::Coordinates::~Coordinates()
{}

const Tracking::Angle& Tracking::Coordinates::getPhi() const
{
  return m_Phi;
}

double Tracking::Coordinates::getR() const
{
  return m_R;
}

double Tracking::Coordinates::getX() const
{
  return m_X;
}

double Tracking::Coordinates::getY() const
{
  return m_Y;
}

void Tracking::Coordinates::setPhi(double m_Phi)
{
  this->m_Phi.setRad(m_Phi);
  this->m_X = this->m_R * cos(this->m_Phi.getRad());
  this->m_Y = this->m_R * sin(this->m_Phi.getRad());
}

void Tracking::Coordinates::setR(double m_R)
{
  this->m_R = m_R;
  this->m_X = this->m_R * cos(this->m_Phi.getRad());
  this->m_Y = this->m_R * sin(this->m_Phi.getRad());
}

void Tracking::Coordinates::setX(double m_X)
{
  this->m_X = m_X;
  this->m_R = sqrt(pow(this->m_X, 2) + pow(this->m_Y, 2));
  float phi = atan2(this->m_Y, this->m_X);
  this->m_Phi.setRad(phi);
}

void Tracking::Coordinates::setY(double m_Y)
{
  this->m_Y = m_Y;
  this->m_R = sqrt(pow(this->m_X, 2) + pow(this->m_Y, 2));
  float phi = atan2(this->m_Y, this->m_X);
  this->m_Phi.setRad(phi);
}

bool Tracking::SortCoordinatesByRadius::operator()(const Tracking::Coordinates& lhs, const Tracking::Coordinates& rhs)
{
  return (lhs.getR() > rhs.getR());
}

