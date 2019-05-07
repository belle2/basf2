/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "MplTrackRep.h"

#include <framework/logging/Logger.h>
#include <FieldManager.h>
#include <TDatabasePDG.h>

#include <math.h>

using namespace genfit;

MplTrackRep::MplTrackRep(int pdgCode, float magCharge, char propDir) :
  RKTrackRep(pdgCode, propDir),
  m_magCharge(magCharge),
  m_mass(TDatabasePDG::Instance()->GetParticle(pdgCode)->Mass()) // We could ofc use AbsTrackRep::getMass(state) but we have no state here to call on
{
  B2RESULT("Monopole representation is created");
}

MplTrackRep::~MplTrackRep()
{
}

double MplTrackRep::RKPropagate(M1x7& state7,
                                M7x7* jacobianT,
                                M1x3& SA,
                                double S,
                                bool varField,
                                bool calcOnlyLastRowOfJ) const
{
  B2RESULT("Propagation with monopole representation is called");
  // The algorithm is
  //  E Lund et al 2009 JINST 4 P04001 doi:10.1088/1748-0221/4/04/P04001
  //  "Track parameter propagation through the application of a new adaptive Runge-Kutta-Nyström method in the ATLAS experiment"
  //  http://inspirehep.net/search?ln=en&ln=en&p=10.1088/1748-0221/4/04/P04001&of=hb&action_search=Search&sf=earliestdate&so=d&rm=&rg=25&sc=0
  // where the transport of the Jacobian is described in
  //   L. Bugge, J. Myrheim  Nucl.Instrum.Meth. 160 (1979) 43-48
  //   "A Fast Runge-kutta Method For Fitting Tracks In A Magnetic Field"
  //   http://inspirehep.net/record/145692
  // and
  //   L. Bugge, J. Myrheim  Nucl.Instrum.Meth. 179 (1981) 365-381
  //   "Tracking And Track Fitting"
  //   http://inspirehep.net/record/160548

  // important fixed numbers
  static const double EC  ( 0.000149896229 );  // c/(2*10^12) resp. c/2Tera FIXME this 1/2 here is super sneaky
  static const double P3  ( 1./3. );           // 1/3
  static const double DLT ( .0002 );           // max. deviation for approximation-quality test
  // Aux parameters
  M1x3&   R           = *((M1x3*) &state7[0]);       // Start coordinates  [cm]  (x,  y,  z)
  M1x3&   A           = *((M1x3*) &state7[3]);       // Start directions         (ax, ay, az);   ax^2+ay^2+az^2=1
  double  S3(0), S4(0), PS2(0);
  M1x3     H0 = {{0.,0.,0.}}, H1 = {{0.,0.,0.}}, H2 = {{0.,0.,0.}};
  M1x3     r = {{0.,0.,0.}};
  // Variables for Runge Kutta solver
  double   A0(0), A1(0), A2(0), A3(0), A4(0), A5(0), A6(0);
  double   B0(0), B1(0), B2(0), B3(0), B4(0), B5(0), B6(0);
  double   C0(0), C1(0), C2(0), C3(0), C4(0), C5(0), C6(0);
  // Additional variables for momentum evolution FIXME these are all cryptic in accordance with the rest of the code around
  double   D0(0), D1(0), D2(0), D3(0), D4(0);
  double   F0(0), F1(0), F2(0), F3(0);
  double   AH0(0), AH1(0), AH2(0), AH3(0);

  //
  // Runge Kutta Extrapolation
  //
  S3 = P3*S;
  S4 = 0.25*S;
  PS2 = m_magCharge * EC * S * (state7[6] > 0 ? 1 : -1);

 // First point
  r[0] = R[0];           r[1] = R[1];           r[2]=R[2];
  FieldManager::getInstance()->getFieldVal(r[0], r[1], r[2], H0[0], H0[1], H0[2]);       // magnetic field in 10^-1 T = kGauss
  H0[0] *= PS2; H0[1] *= PS2; H0[2] *= PS2;     // H0 is PS2*(Hx, Hy, Hz) @ R0; effectively this is h/2 * Force
  D0 = fabs(1.0/state7[6]); // p_n
  F0 = std::sqrt(m_mass * m_mass + D0 * D0) / (D0 * D0); // E / p^2
  AH0 = A[0]*H0[0] + A[1]*H0[1] + A[2]*H0[2]; // A dot Force

  A0 = F0 * (H0[0] - A[0] * AH0); B0 = F0 * (H0[1] - A[1] * AH0); C0 = F0 * (H0[2] - A[2] * AH0); // h/2 * k_1
  A2 = A[0]+A0              ; B2 = A[1]+B0              ; C2 = A[2]+C0              ; // r'_n + h/2 * k_1
  A1 = A2+A[0]              ; B1 = B2+A[1]              ; C1 = C2+A[2]              ; // 2*r'_n + h/2 * k_1

  // Second point
  if (varField) {
    r[0] += A1*S4;         r[1] += B1*S4;         r[2] += C1*S4;
    FieldManager::getInstance()->getFieldVal(r[0], r[1], r[2], H1[0], H1[1], H1[2]);
    H1[0] *= PS2; H1[1] *= PS2; H1[2] *= PS2; // H1 is PS2*(Hx, Hy, Hz) @ (x, y, z) + 0.25*S * [(A0, B0, C0) + 2*(ax, ay, az)]
  }
  else { H1 = H0; };
  D1 = D0 + F0 * D0 * AH0; // p_n + h/2 * l_1
  F1 = std::sqrt(m_mass * m_mass + D1 * D1) / (D1 * D1); // E / p^2
  AH1 = A2*H1[0] + B2*H1[1] + C2*H1[2]; // A dot Force

  A3 = A[0] + F1*(H1[0] - A2*AH1); B3 = A[1] + F1*(H1[1] - B2*AH1); C3 = A[2] + F1*(H1[2] - C2*AH1); // r'_n + h/2 * k_2
  D2 = D0 + F1 * D1 * AH1; // p_n + h/2 * l_2
  F2 = std::sqrt(m_mass * m_mass + D2 * D2) / (D2 * D2); // E / p^2
  AH2 = A3*H1[0] + B3*H1[1] + C3*H1[2]; // A dot Force

  A4 = A[0] + F2*(H1[0] - A3*AH2); B4 = A[1] + F2*(H1[1] - B3*AH2); C4 = A[2] + F2*(H1[2] - C3*AH2); // r'_n + h/2 * k_3
  A5 = A4-A[0]+A4            ; B5 = B4-A[1]+B4            ; C5 = C4-A[2]+C4            ; //    r'_n + h * k_3
  D3 = D0 + 2.0 * F2 * D2 * AH2; // p_n + h * l_3
  F3 = std::sqrt(m_mass * m_mass + D3 * D3) / (D3 * D3); // E / p^2
  AH3 = A4*H1[0] + B4*H1[1] + C4*H1[2]; // A dot Force

  // Last point
  if (varField) {
    r[0]=R[0]+S*A4;         r[1]=R[1]+S*B4;         r[2]=R[2]+S*C4;  //setup.Field(r,H2);
    FieldManager::getInstance()->getFieldVal(r[0], r[1], r[2], H2[0], H2[1], H2[2]);
    H2[0] *= PS2; H2[1] *= PS2; H2[2] *= PS2; // H2 is PS2*(Hx, Hy, Hz) @ (x, y, z) + 0.25*S * (A4, B4, C4)
  }
  else { H2 = H0; };
  A6 = F3 * (H2[0] - A5*AH3); B6 = F3 * (H2[1] - B5*AH3); C6 = F3 * (H2[2] - C5*AH3); // h/2 * k_4
  D4 = F3 * D3 * AH3 - D0; // h/2 * l_4 - p_n

  //
  // Derivatives of track parameters
  //
  if(jacobianT != nullptr){

    // jacobianT
    // 1 0 0 0 0 0 0  x
    // 0 1 0 0 0 0 0  y
    // 0 0 1 0 0 0 0  z
    // x x x x x x 0  a_x
    // x x x x x x 0  a_y
    // x x x x x x 0  a_z
    // x x x x x x 1  q/p
    M7x7& J = *jacobianT;

    double   dA0(0), dA2(0), dA3(0), dA4(0), dA5(0), dA6(0);
    double   dB0(0), dB2(0), dB3(0), dB4(0), dB5(0), dB6(0);
    double   dC0(0), dC2(0), dC3(0), dC4(0), dC5(0), dC6(0);

    int start(0);

    if (!calcOnlyLastRowOfJ) {

      if (!varField) {
        // d(x, y, z)/d(x, y, z) submatrix is unit matrix
        J(0, 0) = 1;  J(1, 1) = 1;  J(2, 2) = 1;
        // d(ax, ay, az)/d(ax, ay, az) submatrix is 0
        // start with d(x, y, z)/d(ax, ay, az)
        start = 3;
      }

      for(int i=start; i<6; ++i) {

        //first point
        dA0 = H0[2]*J(i, 4)-H0[1]*J(i, 5);    // dA0/dp }
        dB0 = H0[0]*J(i, 5)-H0[2]*J(i, 3);    // dB0/dp  } = dA x H0
        dC0 = H0[1]*J(i, 3)-H0[0]*J(i, 4);    // dC0/dp }

        dA2 = dA0+J(i, 3);        // }
        dB2 = dB0+J(i, 4);        //  } = (dA0, dB0, dC0) + dA
        dC2 = dC0+J(i, 5);        // }

        //second point
        dA3 = J(i, 3)+dB2*H1[2]-dC2*H1[1];    // dA3/dp }
        dB3 = J(i, 4)+dC2*H1[0]-dA2*H1[2];    // dB3/dp  } = dA + (dA2, dB2, dC2) x H1
        dC3 = J(i, 5)+dA2*H1[1]-dB2*H1[0];    // dC3/dp }

        dA4 = J(i, 3)+dB3*H1[2]-dC3*H1[1];    // dA4/dp }
        dB4 = J(i, 4)+dC3*H1[0]-dA3*H1[2];    // dB4/dp  } = dA + (dA3, dB3, dC3) x H1
        dC4 = J(i, 5)+dA3*H1[1]-dB3*H1[0];    // dC4/dp }

        //last point
        dA5 = dA4+dA4-J(i, 3);      // }
        dB5 = dB4+dB4-J(i, 4);      //  } =  2*(dA4, dB4, dC4) - dA
        dC5 = dC4+dC4-J(i, 5);      // }

        dA6 = dB5*H2[2]-dC5*H2[1];      // dA6/dp }
        dB6 = dC5*H2[0]-dA5*H2[2];      // dB6/dp  } = (dA5, dB5, dC5) x H2
        dC6 = dA5*H2[1]-dB5*H2[0];      // dC6/dp }

        // this gives the same results as multiplying the old with the new Jacobian
        J(i, 0) += (dA2+dA3+dA4)*S3;  J(i, 3) = ((dA0+2.*dA3)+(dA5+dA6))*P3; // dR := dR + S3*[(dA2, dB2, dC2) +   (dA3, dB3, dC3) + (dA4, dB4, dC4)]
        J(i, 1) += (dB2+dB3+dB4)*S3;  J(i, 4) = ((dB0+2.*dB3)+(dB5+dB6))*P3; // dA :=     1/3*[(dA0, dB0, dC0) + 2*(dA3, dB3, dC3) + (dA5, dB5, dC5) + (dA6, dB6, dC6)]
        J(i, 2) += (dC2+dC3+dC4)*S3;  J(i, 5) = ((dC0+2.*dC3)+(dC5+dC6))*P3;
      }

    } // end if (!calcOnlyLastRowOfJ)

    J(6, 3) *= state7[6]; J(6, 4) *= state7[6]; J(6, 5) *= state7[6];

    //first point
    dA0 = H0[2]*J(6, 4)-H0[1]*J(6, 5) + A0;    // dA0/dp }
    dB0 = H0[0]*J(6, 5)-H0[2]*J(6, 3) + B0;    // dB0/dp  } = dA x H0 + (A0, B0, C0)
    dC0 = H0[1]*J(6, 3)-H0[0]*J(6, 4) + C0;    // dC0/dp }

    dA2 = dA0+J(6, 3);        // }
    dB2 = dB0+J(6, 4);        //  } = (dA0, dB0, dC0) + dA
    dC2 = dC0+J(6, 5);        // }

    //second point
    dA3 = J(6, 3)+dB2*H1[2]-dC2*H1[1] + (A3-A[0]);    // dA3/dp }
    dB3 = J(6, 4)+dC2*H1[0]-dA2*H1[2] + (B3-A[1]);    // dB3/dp  } = dA + (dA2, dB2, dC2) x H1
    dC3 = J(6, 5)+dA2*H1[1]-dB2*H1[0] + (C3-A[2]);    // dC3/dp }

    dA4 = J(6, 3)+dB3*H1[2]-dC3*H1[1] + (A4-A[0]);    // dA4/dp }
    dB4 = J(6, 4)+dC3*H1[0]-dA3*H1[2] + (B4-A[1]);    // dB4/dp  } = dA + (dA3, dB3, dC3) x H1
    dC4 = J(6, 5)+dA3*H1[1]-dB3*H1[0] + (C4-A[2]);    // dC4/dp }

    //last point
    dA5 = dA4+dA4-J(6, 3);      // }
    dB5 = dB4+dB4-J(6, 4);      //  } =  2*(dA4, dB4, dC4) - dA
    dC5 = dC4+dC4-J(6, 5);      // }

    dA6 = dB5*H2[2]-dC5*H2[1] + A6;      // dA6/dp }
    dB6 = dC5*H2[0]-dA5*H2[2] + B6;      // dB6/dp  } = (dA5, dB5, dC5) x H2 + (A6, B6, C6)
    dC6 = dA5*H2[1]-dB5*H2[0] + C6;      // dC6/dp }

    // this gives the same results as multiplying the old with the new Jacobian
    J(6, 0) += (dA2+dA3+dA4)*S3/state7[6];  J(6, 3) = ((dA0+2.*dA3)+(dA5+dA6))*P3/state7[6]; // dR := dR + S3*[(dA2, dB2, dC2) +   (dA3, dB3, dC3) + (dA4, dB4, dC4)]
    J(6, 1) += (dB2+dB3+dB4)*S3/state7[6];  J(6, 4) = ((dB0+2.*dB3)+(dB5+dB6))*P3/state7[6]; // dA :=     1/3*[(dA0, dB0, dC0) + 2*(dA3, dB3, dC3) + (dA5, dB5, dC5) + (dA6, dB6, dC6)]
    J(6, 2) += (dC2+dC3+dC4)*S3/state7[6];  J(6, 5) = ((dC0+2.*dC3)+(dC5+dC6))*P3/state7[6];

  }

  //
  // Track parameters in last point
  //
  R[0] += (A2+A3+A4)*S3;   A[0] += (SA[0]=((A0+2.*A3)+(A5+A6))*P3-A[0]);  // R  = R0 + S3*[(A2, B2, C2) +   (A3, B3, C3) + (A4, B4, C4)]
  R[1] += (B2+B3+B4)*S3;   A[1] += (SA[1]=((B0+2.*B3)+(B5+B6))*P3-A[1]);  // A  =     1/3*[(A0, B0, C0) + 2*(A3, B3, C3) + (A5, B5, C5) + (A6, B6, C6)]
  R[2] += (C2+C3+C4)*S3;   A[2] += (SA[2]=((C0+2.*C3)+(C5+C6))*P3-A[2]);  // SA = A_new - A_old
  state7[6] = m_magCharge * (state7[6] > 0 ? 1 : -1) / P3 / (D1 + 2*D2 + D3 + D4); // g / p_n+1 = g / (1/3 (D1 + 2*D2 +D3 + D4))

  // normalize A
  double CBA ( 1./sqrt(A[0]*A[0]+A[1]*A[1]+A[2]*A[2]) ); // 1/|A|
  A[0] *= CBA; A[1] *= CBA; A[2] *= CBA;


  // Test approximation quality on given step
  double EST ( fabs((A1+A6)-(A3+A4)) +
               fabs((B1+B6)-(B3+B4)) +
               fabs((C1+C6)-(C3+C4))  );  // EST = ||(ABC1+ABC6)-(ABC3+ABC4)||_1  =  ||(axzy x H0 + ABC5 x H2) - (ABC2 x H1 + ABC3 x H1)||_1
  if (debugLvl_ > 0) {
    debugOut << "    RKTrackRep::RKPropagate. Step = "<< S << "; quality EST = " << EST  << " \n";
  }

  // Prevent the step length increase from getting too large, this is
  // just the point where it becomes 10.
  if (EST < DLT*1e-5)
    return 10;

  // Step length increase for a fifth order Runge-Kutta, see e.g. 17.2
  // in Numerical Recipes.  FIXME: move to caller.
  return pow(DLT/EST, 1./5.);
}
