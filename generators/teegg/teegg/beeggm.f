
CDECK  ID>, BEEGGM. 
*.
*...BEEGGM   calculates the squared matrix element for the CAKCUL calc.
*.
*. COMMON    : TEVQUA
*. CALLED    : BEEGGC
*.
*. AUTHOR    : Dean Karlen
*. VERSION   : 7.1
*. CREATED   : 29-Jun-87
*. LAST MOD  : 28-Sep-88
*.
*. Modification Log.
*. 28-Sep-88   Dean Karlen   Change to conform to OPAL standards.
*.
*.**********************************************************************

C-----------------------------------------------------------------------
C DOUBLE PRECISION FUNCTION BEEGGM: Calculates the squared matrix elemnt
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION BEEGGM(M,EB,PP,PM,QP,QM,K1,K2
     >,            S1,S2,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,            K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM)
      IMPLICIT NONE

      COMPLEX*16 I
      PARAMETER( I = (0.D0,1.D0) )

      DOUBLE PRECISION T,TP,SP,U,UP
      COMMON/TEVQUA/T,TP,SP,U,UP

C Input parameters
      DOUBLE PRECISION M,EB,PP(4),PM(4),QP(4),QM(4),K1(4),K2(4)
     >,            S1,S2,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,            K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM

      INTEGER   J

C Real quantities
      DOUBLE PRECISION ECM,S

C Complex quantities
      COMPLEX*16 QPT,QMT,K1T,K2T,WP,WM,W1,W2
     >,          A,A1,B,B1,C,C1,C11,C111,D,D1
     >,          M1,M1TERM(12),M2,M2TERM(8),M3,M3TERM(8)

C Externals
      DOUBLE PRECISION TPRD
      EXTERNAL TPRD

      ECM=2.D0*EB
      S=ECM**2

      QPT = QP(1) + I*QP(2)
      QMT = QM(1) + I*QM(2)
      K1T = K1(1) + I*K1(2)
      K2T = K2(1) + I*K2(2)

      WP = QPT/QPP
      WM = QMT/QMP
      W1 = K1T/K1P
      W2 = K2T/K2P

      A = QPP * CONJG(WP) + K1P * CONJG(W1)
      A1= QMP * CONJG(WM) + K1P * CONJG(W1)

      B = ECM - K1M + CONJG(K1T)*WP + K1T*CONJG(WM) - K1P*WP*CONJG(WM)
      B1= ECM - K1M + CONJG(K1T)*WM + K1T*CONJG(WP) - K1P*WM*CONJG(WP)

      C   = ECM - K1M + CONJG(K1T)*WP
      C1  = ECM - K1M + CONJG(K1T)*WM
      C11 = ECM - K2M + CONJG(K2T)*WP
      C111= ECM - K2M + CONJG(K2T)*WM

      D = (ECM-K2P)*CONJG(WM) + CONJG(K2T)
      D1= (ECM-K2P)*CONJG(WP) + CONJG(K2T)

      M1TERM(1) = SQRT(QPP/QMP) / (S1*K1P*K2P) * C/CONJG(W1)
     >            * C/(WP-W2)/(WM-W2)

      M1TERM(2) = SQRT(QMP/QPP) / (S2*K1P*K2P) * D/W2
     >            * D/CONJG(WP-W1)/CONJG(WM-W1)

      M1TERM(3) = ECM / SQRT(QPP*QMP) / (S*K1P*K2P) * C/(WP-W2)/(WM-W2)
     >            * D/CONJG(WP-W1)/CONJG(WM-W1)

      M1TERM(4) = SQRT(QPP*QMP) / ECM / (SP*K1P*K2P) * C/CONJG(W1)*D/W2

      M1TERM(5) = ECM / SQRT(QPP*QMP) / (T1*K1P*K2P)
     >            * A/CONJG(W1)/CONJG(WP-W1) * A/(WM-W2)

      M1TERM(6) = SQRT(QPP*QMP) / ECM / (T2*K1P*K2P)
     >            * B/CONJG(WM-W1) * B/W2/(WP-W2)

      M1TERM(7) =-SQRT(QPP/QMP) / (T*K1P*K2P) * A/(WM-W2)
     >            * B/CONJG(WM-W1)

      M1TERM(8) =-SQRT(QMP/QPP) / (TP*K1P*K2P)
     >            * A/CONJG(W1)/CONJG(WP-W1) *B/W2/(WP-W2)

      M1TERM(9) =-SQRT(QPP*QMP) / PPK1K2 * (1.D0/SP + 1.D0/TP)
     >            * CONJG(W2)/CONJG(W1)/W2 * ( B + (WP-W1)*D )

      M1TERM(10)= SQRT(QPP*QMP) / PMK1K2 * (1.D0/SP + 1.D0/T )
     >            *( CONJG(W2)*B + CONJG(WM-W2)*C )

      M1TERM(11)=-SQRT(QMP/QPP) * ECM/QPK1K2 * (1.D0/S + 1.D0/TP)
     >            * (WP-W1)/CONJG(WP-W1)/(WP-W2)
     >            *( CONJG(W2)*D + CONJG(W2-WM)*A )

      M1TERM(12)= SQRT(QPP/QMP) * ECM/QMK1K2 * (1.D0/S + 1.D0/T)
     >            * CONJG(WM-W2)/CONJG(WM-W1)/(WM-W2)
     >            *( C + (W1-WP)*A )

      M1 = (0.D0,0.D0)
      DO 1 J=1,12
 1    M1 = M1 + M1TERM(J)

      M2TERM(1) =-SQRT(QMP/QPP) / (S1*K1P*K2P) * C1/CONJG(W1)
     >            * C1/(WM-W2)/(WP-W2)

      M2TERM(2) =-SQRT(QPP/QMP) / (S2*K1P*K2P) * D1/W2
     >            * D1/CONJG(WM-W1)/CONJG(WP-W1)

      M2TERM(3) = ECM / SQRT(QPP*QMP) / (S*K1P*K2P) * C1/(WM-W2)/(WP-W2)
     >            * D1/CONJG(WM-W1)/CONJG(WP-W1)

      M2TERM(4) = SQRT(QPP*QMP)/ECM  / (SP*K1P*K2P) * C1/CONJG(W1)*D1/W2

      M2TERM(5) =-SQRT(QPP*QMP) / PPK1K2 * (1.D0/SP)
     >            * CONJG(W2)/CONJG(W1)/W2 * ( B1 + (WM-W1)*D1 )

      M2TERM(6) = SQRT(QPP*QMP) / PMK1K2 * (1.D0/SP)
     >            *( CONJG(W2)*B1 + CONJG(WP-W2)*C1)

      M2TERM(7) =-SQRT(QPP/QMP) * ECM/QMK1K2 * (1.D0/S)
     >            * (WM-W1)/CONJG(WM-W1)/(WM-W2)
     >            *( CONJG(W2)*D1 + CONJG(W2-WP)*A1 )

      M2TERM(8) = SQRT(QMP/QPP) * ECM/QPK1K2 * (1.D0/S)
     >            * CONJG(WP-W2)/CONJG(WP-W1)/(WP-W2)
     >            *( C1 + (W1-WM)*A1 )

      M2 = (0.D0,0.D0)
      DO 2 J=1,8
 2    M2 = M2 + M2TERM(J)

      M3TERM(1) =-SQRT(QPP/QMP) / (T2*K1P*K2P)
     >            * CONJG(D1)/CONJG(WM-W1) * CONJG(D1)/W2/(WP-W2)

      M3TERM(2) =-SQRT(QMP/QPP) / (T1*K1P*K2P)
     >            * C111/(WM-W2) * C111/CONJG(W1)/CONJG(WP-W1)

      M3TERM(3) = SQRT(QPP/QMP) / (T*K1P*K2P)
     >            * CONJG(D1)/CONJG(WM-W1) * C111/(WM-W2)

      M3TERM(4) = SQRT(QMP/QPP) / (TP*K1P*K2P)
     >            * CONJG(D1)/W2/(WP-W2) * C111/CONJG(W1)/CONJG(WP-W1)

      M3TERM(5) = SQRT(QPP*QMP) / PPK1K2 * (1.D0/TP)
     >            * CONJG(W2)/CONJG(W1)/W2
     >            * ( (WP-W1)*CONJG(D) + (W1-WM)*CONJG(D1) )

      M3TERM(6) =-SQRT(QPP*QMP) / PMK1K2 * (1.D0/T)
     >            *( (W1-WP)*C111 + (WM-W1)*C11 )

      M3TERM(7) = SQRT(QMP/QPP) * ECM/QPK1K2 * (1.D0/TP)
     >            * (WP-W1)/(WP-W2)/CONJG(WP-W1)
     >            *( C111 + CONJG(W2)*CONJG(D) )

      M3TERM(8) =-SQRT(QPP/QMP) * ECM/QMK1K2 * (1.D0/T)
     >            * (WM-W1)/CONJG(WM-W1)/(WM-W2)
     >            *( C11 +  CONJG(W2)*CONJG(D1) )

      M3 = (0.D0,0.D0)
      DO 3 J=1,8
 3    M3 = M3 + M3TERM(J)

      BEEGGM = ABS(M1)**2 + ABS(M2)**2 + ABS(M3)**2

      RETURN
      END
