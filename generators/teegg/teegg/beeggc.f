
CDECK  ID>, BEEGGC. 
*.
*...BEEGGC   calculates dbl radiative Bhabha according to CALKUL collab.
*.
*.  BEEGGC is a double precision function that calcualtes the squared
*.  matrix element for,
*.
*.    +  _      +  _
*.   e  e  --> e  e  gamma gamma
*.
*.using the equations supplied by Berends et al. Nucl.Phys.B264(1986)265
*.
*. COMMON    : TEVQUA
*. CALLS     : BCOLL BEEGGM TPRD
*. CALLED    : T4BODY
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
C DOUBLE PRECISION FUNCTION BEEGGC: Calculates the matrix element for,
C     +  _      +  _
C    e  e  --> e  e  gamma gamma
C
C using the equations supplied by Berends et al. Nucl.Phys.B264(1986)265
C-----------------------------------------------------------------------

      DOUBLE PRECISION FUNCTION BEEGGC(M,EB,QP,QM,K1,K2
     >,            S1,S2,T,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,            K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM
     >,            BPMK1,BPMK2,BQMK1,BQMK2,BK1K2)
      IMPLICIT NONE

      DOUBLE PRECISION ALPHA,PI
      PARAMETER
     >(           ALPHA = 1.D0/137.036D0
     >,           PI    = 3.14159265358979D0
     >)

      DOUBLE PRECISION TA,TP,SP,U,UP
      COMMON/TEVQUA/TA,TP,SP,U,UP

C Input parameters
      DOUBLE PRECISION M,EB,PP(4),PM(4),QP(4),QM(4),K1(4),K2(4)
     >,            S1,S2,T,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,            K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM
     >,            BPMK1,BPMK2,BQMK1,BQMK2,BK1K2

C Real quantities
      DOUBLE PRECISION S,QPK1,QPK2,PPK1,PPK2
     >,      V1Q(4),V1P(4),V1QV1P(4),V2Q(4),V2P(4),V2QV2P(4)
     >,      ABSQM,COLCUT,CQMK2,CPMK2,SP2,TP2,UP2,KPP2M,KPP2P
     >,      TERM1,TERM2,TERM3

C Externals
      DOUBLE PRECISION
     >         TPRD,BEEGGM,BCOLL
      EXTERNAL TPRD,BEEGGM,BCOLL

      PP(1)=0.D0
      PP(2)=0.D0
      PP(3)=SQRT(EB**2 - M**2)
      PP(4)=EB

      PM(1)=0.D0
      PM(2)=0.D0
      PM(3)=-SQRT(EB**2 - M**2)
      PM(4)=EB

      S = 4.D0*EB**2

      QPK1 = TPRD(QP,K1)
      PPK1 = TPRD(PP,K1)

      V1Q(1)=QP(1)/QPK1 - QM(1)/BQMK1
      V1Q(2)=QP(2)/QPK1 - QM(2)/BQMK1
      V1Q(3)=QP(3)/QPK1 - QM(3)/BQMK1
      V1Q(4)=QP(4)/QPK1 - QM(4)/BQMK1

      V1P(1)=PP(1)/PPK1 - PM(1)/BPMK1
      V1P(2)=PP(2)/PPK1 - PM(2)/BPMK1
      V1P(3)=PP(3)/PPK1 - PM(3)/BPMK1
      V1P(4)=PP(4)/PPK1 - PM(4)/BPMK1

      V1QV1P(1) = V1Q(1) - V1P(1)
      V1QV1P(2) = V1Q(2) - V1P(2)
      V1QV1P(3) = V1Q(3) - V1P(3)
      V1QV1P(4) = V1Q(4) - V1P(4)

      QPK2 = TPRD(QP,K2)
      PPK2 = TPRD(PP,K2)

      V2Q(1)=QP(1)/QPK2 - QM(1)/BQMK2
      V2Q(2)=QP(2)/QPK2 - QM(2)/BQMK2
      V2Q(3)=QP(3)/QPK2 - QM(3)/BQMK2
      V2Q(4)=QP(4)/QPK2 - QM(4)/BQMK2

      V2P(1)=PP(1)/PPK2 - PM(1)/BPMK2
      V2P(2)=PP(2)/PPK2 - PM(2)/BPMK2
      V2P(3)=PP(3)/PPK2 - PM(3)/BPMK2
      V2P(4)=PP(4)/PPK2 - PM(4)/BPMK2

      V2QV2P(1) = V2Q(1) - V2P(1)
      V2QV2P(2) = V2Q(2) - V2P(2)
      V2QV2P(3) = V2Q(3) - V2P(3)
      V2QV2P(4) = V2Q(4) - V2P(4)

      ABSQM = SQRT(QM(4)**2-M**2)
      COLCUT=SQRT(M*EB)

C Check if a collinear cross section should be used...
C if K2 is collinear with both p- and q-, choose the smaller coll

      CPMK2=BCOLL(PM,K2)
      CQMK2=BCOLL(QM,K2)
      IF(MAX(CPMK2,CQMK2).LT.COLCUT)THEN
         IF(CPMK2.LT.CQMK2)THEN
            CQMK2=COLCUT*10.D0
         ELSE
            CPMK2=COLCUT*10.D0
         ENDIF
      ENDIF

      IF(CQMK2.LT.COLCUT)THEN

         SP2 = S1
         TP2 = T1
         UP2 = U1
         KPP2M = (BQMK2 - K2(4)*(QM(4)-ABSQM))/ABSQM
         KPP2P = 2.D0*K2(4)-KPP2M
         BEEGGC = -ALPHA**4/2.D0/PI**4/S
     >             * TPRD(V1QV1P,V1QV1P)
     >             * QM(4)*KPP2M/(2.D0*QM(4)+KPP2P)/KPP2P/BQMK2**2
     >             * ( 4.D0*QM(4)**2 + (2.D0*QM(4)+KPP2P)**2
     >                + M**2*KPP2P**3/4.D0/QM(4)**2/KPP2M )
     >             * ( S*SP2*(S**2+SP2**2) + T*TP2*(T**2+TP2**2)
     >                +U2*UP*(U2**2+UP**2) )
     >             / (S*SP2*T*TP2)

      ELSE IF(CPMK2.LT.COLCUT)THEN

         SP2 = S1
         TP2 = T1
         UP2 = U1
         BEEGGC = -ALPHA**4/2.D0/PI**4/S
     >             * TPRD(V1QV1P,V1QV1P)
     >             * EB*K2P/(2.D0*EB-K2M)/K2M/BPMK2**2
     >             * ( S + (2.D0*EB-K2M)**2 + M**2*K2M**3/S/K2P )
     >             * ( S2*SP*(S2**2+SP**2) + T*TP2*(T**2+TP2**2)
     >                +U*UP2*(U**2+UP2**2) )
     >             / (S2*SP*T*TP2)

      ELSE

         TERM1 =ALPHA**4/4.D0/PI**4/S
     >          * TPRD(V1QV1P,V1QV1P) * TPRD(V2QV2P,V2QV2P)
     >          *(S*SP*(S**2+SP**2)+T*TP*(T**2+TP**2)+U*UP*(U**2+UP**2))
     >          /(S*SP*T*TP)
         TERM2 =  4.D0*ALPHA**4 / PI**4 /S
     >             * BEEGGM(M,EB,PP,PM,QP,QM,K1,K2
     >,               S1,S2,T1,T2,U1,U2,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,               K1P,K1M,K2P,K2M,QPP,QPM,QMP,QMM)
         TERM3 =  4.D0*ALPHA**4 / PI**4 /S
     >             * BEEGGM(M,EB,PP,PM,QP,QM,K2,K1
     >,               S2,S1,T2,T1,U2,U1,PPK1K2,PMK1K2,QPK1K2,QMK1K2
     >,               K2P,K2M,K1P,K1M,QPP,QPM,QMP,QMM)
         BEEGGC=TERM1+TERM2+TERM3

      ENDIF

      RETURN
      END
