#ifndef BOOST_H
#define BOOST_H

#include "TLorentzRotation.h"
TLorentzRotation getBoost(double Eher, double Eler, double cross_angle, double angle);
double getBeamEnergySpreadCM(double E1, double dE1,
                             double E2, double dE2,
                             double crossing_angle);
#endif
