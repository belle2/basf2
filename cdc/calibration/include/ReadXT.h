void newReadXT(double m_XT[56][2][18][10][8], TString filename,
               double m_alphaPoints[18],
               double bl_alpha[18], double bu_alpha[18],
               double m_thetaPoints[10],
               double bl_theta[10], double bu_theta[10],
               unsigned m_xtParamMode)
{

  ifstream ifs;
  ifs.open(filename);

  //read alpha bin info.
  unsigned short nAlphaBins = 0;
  ifs >> nAlphaBins;

  //double alpha0, alpha1, alpha2;
  for (unsigned short i = 0; i < nAlphaBins; ++i) {
    ifs >> bl_alpha[i] >> bu_alpha[i] >> m_alphaPoints[i];
    // ifs >> alpha0 >> alpha1 >> alpha2;
    //m_alphaPoints[i] = alpha2;
  }

  //read theta bin info.
  unsigned short nThetaBins = 0;
  ifs >> nThetaBins;

  //double theta0, theta1, theta2;

  for (unsigned short i = 0; i < nThetaBins; ++i) {
    ifs >> bl_theta[i] >> bu_theta[i] >> m_thetaPoints[i];
    //ifs >> theta0 >> theta1 >> theta2;
    //m_thetaPoints[i] = theta2;
  }

  short np = 0;
  unsigned short iCL, iLR;
  //  const unsigned short npx = nXTParams - 1;
  double xtc[npar];
  double theta, alpha, dummy1;
  unsigned nRead = 0;
  unsigned m_xtParamMode;
  ifs >> m_xtParamMode >> np;

  const double epsi = 0.1;

  while (ifs >> iCL) {
    ifs >> theta >> alpha >> dummy1 >> iLR;
    for (int i = 0; i < np; ++i) {
      ifs >> xtc[i];
    }
    ++nRead;

    int itheta = -99;
    for (unsigned short i = 0; i < nThetaBins; ++i) {
      if (fabs(theta - m_thetaPoints[i]) < epsi) {
        itheta = i;
        break;
      }
    }
    if (itheta < 0) {
      gSystem->Exec("echo xt_theta error binning>> error");
    }

    int ialpha = -99;
    for (unsigned short i = 0; i < nAlphaBins; ++i) {
      if (fabs(alpha - m_alphaPoints[i]) < epsi) {
        ialpha = i;
        break;
      }
    }
    if (ialpha < 0) {
      gSystem->Exec("echo xt_alpha error binning>> error");
    }

    for (int i = 0; i < np; ++i) {
      m_XT[iCL][iLR][ialpha][itheta][i] = xtc[i];
    }


  }  //end of while loop

  //convert unit
  /*
  const double degrad = M_PI / 180.;
  for (unsigned i = 0; i < nAlphaBins; ++i) {
    m_alphaPoints[i] *= degrad;
  }
  for (unsigned i = 0; i < nThetaBins; ++i) {
    m_thetaPoints[i] *= degrad;
  }
  */
}
