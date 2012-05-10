/*
 * Angle.cc
 *
 *  Created on: Mar 3, 2011
 *      Author: jbauer
 */

#include <tracking/modules/cdcHoughTracking/Angle.h>
#include <TMath.h>


using namespace Belle2;
using namespace Belle2::Tracking;
using TMath::Pi;

/**
 * @brief Default Constructor
 *
 * Creates Angle object with rad and deg set to 0.
 *
 */

Angle::Angle() :
  m_rad(0), m_deg(0)
{}

/**
 * @brief Constructor
 *
 * Takes angle in rad or degrees and initializes members for rad and deg.
 * @param rad input angle in radians or degrees
 * @param isRad true if rad is in radians, false for degrees
 */
Angle::Angle(float rad, bool isRad)
{
  if (isRad) {
    m_rad = checkRad(rad);
    m_deg = radToDeg(m_rad);
  } else {
    m_deg = checkDeg(rad);
    m_rad = degToRad(m_deg);
  }
}

Angle::~Angle()
{
}

Angle Angle::operator+ (const Angle& rhs)
{
  float rtmp = this->getRad() + rhs.getRad();
  rtmp = checkRad(rtmp);

  return Angle(rtmp);
}
/**
 * @brief Operator+
 *
 * adds float rhs to rad member of angle
 * @param rhs float value to be added
 * @return
 */
Angle Angle::operator+ (const float rhs)
{
  float rtmp = this->getRad() + rhs;
  rtmp = checkRad(rtmp);

  return Angle(rtmp);
}

Angle Angle::operator- (const Angle& rhs)
{
  float rtmp = this->getRad() - rhs.getRad();
  rtmp = checkRad(rtmp);

  return Angle(rtmp);
}

Angle Angle::operator- (const float rhs)
{
  float rtmp = this->getRad() - rhs;
  rtmp = checkRad(rtmp);

  return Angle(rtmp);
}

float Angle::getDeg() const
{
  return m_deg;
}

float Angle::getRad() const
{
  return m_rad;
}

float Angle::radToDeg(float rad)
{
  return rad / 2 / Pi() * 360;
}
/**
 * @brief Setter for grad member
 *
 * Also sets rad member accordingly
 * @param m_deg new value for grad member
 */
void Angle::setDeg(float m_deg)
{
  this->m_deg = m_deg;
  this->checkDeg();
  this->m_rad = degToRad(this->m_deg);
}

void Angle::setRad(float m_rad)
{
  this->m_rad = m_rad;
  this->checkRad();
  this->m_deg = radToDeg(this->m_rad);
}

float Angle::degToRad(float deg)
{
  return deg / 360 * 2 * Pi();
}

void Angle::checkRad()
{
  this->m_rad = checkRad(this->m_rad);
}

void Angle::checkDeg()
{
  this->m_deg = checkDeg(this->m_deg);
}

float Angle::checkRad(float rad)
{
  while (rad > 2 * Pi())
    rad -= (2 * Pi());
  while (rad < 0)
    rad += (2 * Pi());

  return rad;
}

float Angle::checkDeg(float deg)
{
  while (deg >= 360)
    deg -= 360;
  while (deg < 0)
    deg += 360;

  return deg;
}
