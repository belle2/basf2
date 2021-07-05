/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Valerio Bertacchi                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/hitXP.h>

using namespace Belle2;

double hitXP::omega(TVector3 xx, TVector3 p, double charge) const
{
  TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * 0.299792458;
  double aux = Bze / sqrt(p.X() * p.X() + p.Y() * p.Y());
  aux = aux / 100;
  return aux;
}


double hitXP::tanLambda(TVector3 xx, TVector3 p) const
{
  TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double aux = p.Z() / sqrt(p.X() * p.X() + p.Y() * p.Y());
  return aux;
}


double hitXP::d0(TVector3 xx, TVector3 p, double charge) const
{
  TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * 0.299792458;
  double aux = sqrt((p.Y() / (Bze) + x.X()) * (p.Y() / (Bze) + x.X()) + (x.Y() - p.X() / (Bze)) * (x.Y() - p.X() / (Bze))) - sqrt((
                 p.X() * p.X() + p.Y() * p.Y()) / (Bze * Bze));
  if (Bze < 0) {
    aux = -aux;
  }
  aux = 100 * aux;
  return aux;
}

double hitXP::phi0(const TVector3& xx, TVector3 p, double charge) const
{
  TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * 0.299792458;
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

double hitXP::z0(TVector3 xx, TVector3 p, double charge) const
{
  TVector3 x(0.01 * xx.X(), 0.01 * xx.Y(), 0.01 * xx.Z());
  double Bz = 1.5;
  double Bze = Bz * charge * 0.299792458;
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
