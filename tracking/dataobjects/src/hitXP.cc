/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/dataobjects/hitXP.h>
#include <framework/gearbox/Const.h>

using namespace Belle2;

double hitXP::omega(ROOT::Math::XYZVector xx, ROOT::Math::XYZVector p, double charge) const
{
  ROOT::Math::XYZVector x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * Const::speedOfLight / 100.;
  double aux = Bze / sqrt(p.X() * p.X() + p.Y() * p.Y());
  aux = aux / 100;
  return aux;
}


double hitXP::tanLambda(ROOT::Math::XYZVector xx, ROOT::Math::XYZVector p) const
{
  ROOT::Math::XYZVector x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double aux = p.Z() / sqrt(p.X() * p.X() + p.Y() * p.Y());
  return aux;
}


double hitXP::d0(ROOT::Math::XYZVector xx, ROOT::Math::XYZVector p, double charge) const
{
  ROOT::Math::XYZVector x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * Const::speedOfLight / 100.;
  double aux = sqrt((p.Y() / (Bze) + x.X()) * (p.Y() / (Bze) + x.X()) + (x.Y() - p.X() / (Bze)) * (x.Y() - p.X() / (Bze))) - sqrt((
                 p.X() * p.X() + p.Y() * p.Y()) / (Bze * Bze));
  if (Bze < 0) {
    aux = -aux;
  }
  aux = 100 * aux;
  return aux;
}

double hitXP::phi0(const ROOT::Math::XYZVector& xx, ROOT::Math::XYZVector p, double charge) const
{
  ROOT::Math::XYZVector x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * Const::speedOfLight / 100.;
  double aux;
  double chi;
  if (Bze > 0) {
    chi = atan2((-p.X() * x.X() - p.Y() * x.Y()), ((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
  } else {
    chi = atan2(-(-p.X() * x.X() - p.Y() * x.Y()), -((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
  }
  aux = atan2(p.Y(), p.X()) - chi;
  return aux;
}

double hitXP::z0(ROOT::Math::XYZVector xx, ROOT::Math::XYZVector p, double charge) const
{
  ROOT::Math::XYZVector x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * Const::speedOfLight / 100.;
  double aux;
  double chi;
  if (Bze > 0) {
    chi = atan2((-p.X() * x.X() - p.Y() * x.Y()), ((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
  } else {
    chi = atan2(-(-p.X() * x.X() - p.Y() * x.Y()), -((p.X() * p.X() + p.Y() * p.Y()) / (Bze) + p.Y() * x.X() - p.X() * x.Y()));
  }
  aux = p.Z() * chi / (Bze) + x.Z();
  aux = 100 * aux;
  return aux;
}
