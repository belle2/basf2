#ifndef FITTER3DUTILITY_H
#define FITTER3DUTILITY_H

#include "TLorentzVector.h"
#include "TVector3.h"
#include "TVector2.h"

int bitSize(int numberBits, int mode);

void changeInteger(int &integer, double real, double minValue, double maxValue, int bitSize);

void changeReal(double &real, int integer, double minValue, double maxValue, int bitSize);

int findSign(double *phi2);

void findExtreme(double &m_max, double &m_min, double value);

void rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0);

void rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS);

double calStAxPhi(int &mysign, double &anglest, double &ztostraw, double &rr, double &rho, double &myphi0);

double calDeltaPhi(int &mysign, double &anglest, double &ztostraw, double &rr, double &phi2, double &rho, double &myphi0);

double calZ(int &mysign, double &anglest, double &ztostraw, double &rr, double &phi2, double &rho, double &myphi0);

double calS(double &rho, double &rr);

double rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot);

void findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition);

#endif
