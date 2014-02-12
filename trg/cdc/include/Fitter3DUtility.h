#ifndef FITTER3DUTILITY_H
#define FITTER3DUTILITY_H

#include "TVectorD.h"
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

void rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot,double &zchi2);

void rSFit2(double *iezz21, double *iezz22, double *arcS, double *zz,int *lutv,  double &z0, double &cot,double &zchi2);

void findImpactPosition(TVector3 * mcPosition, TLorentzVector * mcMomentum, int charge, TVector2 & helixCenter, TVector3 & impactPosition);

// HelixParameters: dR, phi0, keppa, dz, tanLambda
void calHelixParameters(TVector3* position, TVector3* momentum, int charge, TVectorD* helixParameters); 

void calPositionAtR(TVectorD& helixParameters, double radius, TVector3* position, TVector3* momentum);

#endif
