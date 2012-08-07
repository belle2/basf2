#ifndef FITTER3DUTILITY_H
#define FITTER3DUTILITY_H

int findSign(double *phi2);

void rPhiFit(double *rr, double *phi2, double *phierror, double &rho, double &myphi0);

void rPhiFit2(double *rr, double *phi2, double *phierror, double &rho, double &myphi0, int nTS);

double calZ(int &mysign, double &anglest, double &ztostraw, double &rr, double &phi2, double &rho, double &myphi0);

double calS(double &rho, double &rr);

void rSFit(double *iezz2, double *arcS, double *zz, double &z0, double &cot);

#endif
