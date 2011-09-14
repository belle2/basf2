!-----------------------------------------------------------------------
! TOP counter for Belle: ray tracing inside Q-bar
! M. Staric, sept-2008, jul-2009, nov-2009, sept-2011
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! all vectors expressed in the Q-bar frame
!-----------------------------------------------------------------------

! pozor: klici RND_NUM so zakomentirani!


      SUBROUTINE TOP_RAYTRACE(RF,DIR,POL,EPH,MDN,TOP,LR,NX,NY,OK)

      IMPLICIT NONE
      REAL RF(3),DIR(3),POL(3)  ! point, direction, polarization [in/out]
      REAL EPH                  ! photon energy (eV) [in]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TOP                  ! time-of-propagation (ns) [out]
      INTEGER LR                ! 0/1=left/right exit window [out]
      INTEGER NX(3), NY(3)      ! signed num. of reflections [out]
      LOGICAL OK                ! status [out]

#include "TOP_CONS.fi"
      INTEGER KAM, NN, NX1, NY1, I
      REAL TLEN
      REAL GROUP_INDEX, PHASE_INDEX, ABSPT_LENG, RND_NUM

      OK=ABS(DIR(3)).GT.1E-5
      IF(.NOT.OK) RETURN

      TOP=0.
      DO I=1,3
         NX(I)=0
         NY(I)=0
      ENDDO

      NN=0
      CALL KJE_FOTON(RF,MDN,KAM,LR)
      DO WHILE(.TRUE.)
         IF(KAM.EQ.1) THEN
            CALL RAYTRACE_QBAR(RF,DIR,POL,EPH,MDN,TLEN,NX1,NY1,KAM,LR)
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
         ELSE IF(KAM.EQ.2) THEN
            CALL RAYTRACE_MIRR(RF,DIR,POL,EPH,MDN,TLEN,NX1,NY1,KAM,LR)
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
            NX(2)=NX(1)
            NY(2)=NY(1)
         ELSE IF(KAM.EQ.3) THEN
            CALL RAYTRACE_EVOL(RF,DIR,POL,EPH,MDN,TLEN,NX1,NY1,KAM,LR)
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
            NX(3)=NX1
            NY(3)=NY1
         ELSE
            EXIT
         ENDIF
cc         IF(TLEN.GT.-ABSPT_LENG(EPH)*LOG(RND_NUM())) KAM=-1 ! bulk absorption
         TOP=TOP+TLEN/C0*GROUP_INDEX(EPH)
         NN=NN+1
         IF(NN.GT.100) THEN
            PRINT*,'top_raytrace_: loop limit reached, n=',NN
            EXIT
         ENDIF
      ENDDO
      OK=KAM.EQ.0

      END


      SUBROUTINE KJE_FOTON(RF,MDN,KAM,LR)

      IMPLICIT NONE
      REAL RF(3)
      INTEGER MDN, KAM, LR
#include "TOP_GEO.fi"

      LR=0
      IF(RF(3).GT.0) LR=1

      IF(RF(3).GE.QBAR_ZL(MDN).AND.RF(3).LE.QBAR_ZR(MDN)) THEN
         KAM=1
      ELSE IF(ABS(RF(3)).LE.QBAR_C(MDN)/2) THEN
         KAM=2
         IF(LIN_OPT) KAM=1
      ELSE
         KAM=3
      ENDIF

      END


      SUBROUTINE RAYTRACE_QBAR(RF,DIR,POL,EPH,MDN,TLEN,NX,NY,KAM,LR)

      IMPLICIT NONE
      REAL RF(3),DIR(3),POL(3)  ! point, direction, polarization [in/out]
      REAL EPH                  ! photon energy (eV) [in]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! length-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections in x and y [out]
      INTEGER KAM               ! next object
      INTEGER LR                ! on left/right

#include "TOP_GEO.fi"
      REAL XW, YW, ZW
      INTEGER I, J, ITIP
      REAL X, Y, P
      REAL PHASE_INDEX
      LOGICAL OK

      IF(KAM.NE.1) STOP 'KAM<>1'

      NX=0
      NY=0
      KAM=-1
      IF(DIR(3).GT.0) THEN
         LR=1
         ZW=QBAR_ZR(MDN)
         IF(LIN_OPT) ZW=QBAR_C(MDN)/2
         ITIP=QBAR_RSIDE(MDN)
      ELSE
         LR=0
         ZW=QBAR_ZL(MDN)
         IF(LIN_OPT) ZW=-QBAR_C(MDN)/2
         ITIP=QBAR_LSIDE(MDN)
      ENDIF
      TLEN=(ZW-RF(3))/DIR(3)
      IF(TLEN.LT.0) RETURN
      IF(TLEN.GT.10000) RETURN

!     photon on exit plane

      X=RF(1)+TLEN*DIR(1)
      Y=RF(2)+TLEN*DIR(2)
      I=NINT(X/QBAR_A(MDN))
      J=NINT(Y/QBAR_B(MDN))

!     total reflection requirement

      P=SQRT(1.-1./PHASE_INDEX(EPH)**2)
      OK=(ABS(DIR(1)).LT.P.OR.I.EQ.0).AND.(ABS(DIR(2)).LT.P.OR.J.EQ.0)
      IF(.NOT.OK) RETURN

!     absorption on edges

      CALL EDGE_ABSORB(RF,DIR,MDN,I,J,OK)
      IF(.NOT.OK) RETURN

!     exit window

      XW=X-QBAR_A(MDN)*I
      IF(MOD(ABS(I),2).EQ.1) THEN
         XW=-XW
         DIR(1)=-DIR(1)
      ENDIF
      YW=Y-QBAR_B(MDN)*J
      IF(MOD(ABS(J),2).EQ.1) THEN
         YW=-YW
         DIR(2)=-DIR(2)
      ENDIF
      RF(1)=XW
      RF(2)=YW
      RF(3)=ZW
      NX=I
      NY=J

      KAM=0
      IF(ITIP.GT.1) KAM=2
      IF(QBAR_EXT(LR,MDN).GT.0) KAM=3

      END


      SUBROUTINE RAYTRACE_MIRR(RF,DIR,POL,EPH,MDN,TLEN,NX,NY,KAM,LR)

      IMPLICIT NONE
      REAL RF(3),DIR(3),POL(3)  ! point, direction, polarization [in/out]
      REAL EPH                  ! photon energy (eV) [in]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! lenght-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections [out]
      INTEGER KAM               ! next object
      INTEGER LR                ! on left/right

#include "TOP_GEO.fi"
      INTEGER ITIP
      REAL KX, KY, P
      REAL PHASE_INDEX, RND_NUM, MIRROR_REFL
      LOGICAL OK

      IF(KAM.NE.2) STOP 'KAM<>2'

      TLEN=0
      NX=0
      NY=0

      KAM=1
      IF(LR.EQ.0) THEN
         IF(DIR(3).GT.0) RETURN
         ITIP=QBAR_LSIDE(MDN)
      ELSE
         IF(DIR(3).LT.0) RETURN
         ITIP=QBAR_RSIDE(MDN)
      ENDIF

cc      KAM=-1
cc      OK=RND_NUM().LT.MIRROR_REFL(EPH)
cc      IF(.NOT.OK) RETURN

      IF(ITIP.EQ.2) THEN        ! planar mirror
         DIR(3)=-DIR(3)
      ELSE IF(ITIP.EQ.3.OR.ITIP.EQ.4) THEN ! cylindrical or spherical mirror
         KX=DIR(1)
         KY=DIR(2)
         CALL FOC_MIRROR(RF,DIR,MDN,ITIP,TLEN,NX,NY,OK)
         IF(OK) THEN            ! check for total reflection
            P=SQRT(1.-1./PHASE_INDEX(EPH)**2)
            OK=(ABS(KX).LT.P.OR.NX.EQ.0).AND.(ABS(KY).LT.P.OR.NY.EQ.0)
         ENDIF
      ELSE
         STOP 'napaka: tip ni zrcalo!!'
      ENDIF
      IF(.NOT.OK) RETURN
      KAM=1

      END


      SUBROUTINE RAYTRACE_EVOL(RF,DIR,POL,EPH,MDN,TLEN,NX,NY,KAM,LR)

      IMPLICIT NONE
      REAL RF(3),DIR(3),POL(3)  ! point, direction, polarization [in/out]
      REAL EPH                  ! photon energy (eV) [in]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! lenght-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections [out]
      INTEGER KAM               ! next object [in/out]
      INTEGER LR                ! on left/right [in]

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      INTEGER ID, I, I0, NN
      REAL PATH, PATH1, R(3), DIRNOR, DIRNOR1, PTOT, AH, BH, KX, KY
      REAL RNOR
      REAL S_PROD, RND_NUM
      REAL PHASE_INDEX, MIRROR_REFL
      real PATH_MAX
      PARAMETER (PATH_MAX=1.E10)
      REAL R_PL(3,10), NORM(3,10)
      INTEGER ITIP(10), N_PL
      INTEGER ID_OLD /0/, MDN_OLD /-1/
      SAVE R_PL, NORM, ITIP, N_PL, ID_OLD, MDN_OLD

      IF(KAM.NE.3) STOP 'KAM<>3'

      TLEN=0
      NX=0
      NY=0
      KAM=-1

      ID=QBAR_EXT(LR,MDN)
      IF(ID.EQ.0) RETURN

      IF(ID.NE.ID_OLD.OR.MDN.NE.MDN_OLD) THEN
         ID_OLD=ID
         MDN_OLD=MDN
         CALL SET_EVOL(MDN,ID,LR,R_PL,NORM,ITIP,N_PL)
      ENDIF
      PTOT=SQRT(1.-1./PHASE_INDEX(EPH)**2)
      AH=QBAR_A(MDN)/2
      BH=QBAR_B(MDN)/2
      KX=DIR(1)
      KY=DIR(2)

      I0=0
      NN=0
      DO WHILE(TLEN.LT.1000)    ! 50 ns max. time
         PATH=PATH_MAX
         DO I=1,N_PL
            IF(I.EQ.I0) CYCLE
            DIRNOR1=S_PROD(DIR,NORM(1,I))
            IF(DIRNOR1.LT.0) THEN
               R(1)=RF(1)-R_PL(1,I)
               R(2)=RF(2)-R_PL(2,I)
               R(3)=RF(3)-R_PL(3,I)
               RNOR=S_PROD(R,NORM(1,I))
               IF(RNOR.LT.-PATH_MAX*DIRNOR1) THEN
                  PATH1=-RNOR/DIRNOR1
                  IF(PATH1.GE.0.AND.PATH1.LT.PATH) THEN
                     PATH=PATH1
                     DIRNOR=DIRNOR1
                     I0=I
                  ENDIF
               ENDIF
            ENDIF
         ENDDO
         IF(I0.EQ.0) THEN
            PRINT*,'trace_evol_ error: I0=0'
            RETURN
         ENDIF
         CALL NEW_POINT(RF,DIR,PATH,RF)
         TLEN=TLEN+PATH

         IF(ITIP(I0).EQ.0) THEN
            KAM=0
            EXIT
         ELSE IF(ITIP(I0).EQ.1) THEN
            IF(ABS(RF(1)).LE.AH.AND.ABS(RF(2)).LE.BH) KAM=1
            EXIT
         ELSE IF(ITIP(I0).EQ.2) THEN
            IF(ABS(DIRNOR).GT.PTOT) EXIT
            DO I=1,3
               DIR(I)=DIR(I)-2*DIRNOR*NORM(I,I0)
            ENDDO
            IF(NORM(1,I0).NE.0) THEN
               NX=NX+1
            ELSE
               NY=NY+1
            ENDIF
         ELSE IF(ITIP(I0).EQ.3) THEN
cc            IF(RND_NUM().GT.MIRROR_REFL(EPH)) EXIT
            DO I=1,3
               DIR(I)=DIR(I)-2*DIRNOR*NORM(I,I0)
            ENDDO
            IF(NORM(1,I0).NE.0) THEN
               NX=NX+1
            ELSE
               NY=NY+1
            ENDIF
         ELSE
            EXIT
         ENDIF
         NN=NN+1
         IF(NN.GT.10000) THEN
            PRINT*,'raytrace_evol_: loop limit reached, n=',NN
            EXIT
         ENDIF
      ENDDO
      IF(KX.LT.0) NX=-NX
      IF(KY.LT.0) NY=-NY

      END


      SUBROUTINE SET_EVOL(MDN,ID,LR,R_PL,NORM,ITIP,N_PL)

c--   surface types (ITIP)
c       -1 no reflection
c        0 exit to PMT
c        1 exit to bar
c        2 total reflection
c        3 mirror

      IMPLICIT NONE
      INTEGER MDN, ID, LR
      REAL R_PL(3,*), NORM(3,*)
      INTEGER ITIP(*), N_PL
#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      REAL ZIN, ZOU, AH, BH, PHI
      INTEGER I

      ZIN=QBAR_C(MDN)/2
      ZOU=ZIN+EXT_DZ(ID)
      AH=QBAR_A(MDN)/2
      BH=QBAR_B(MDN)/2

      N_PL=1
      R_PL(1,N_PL)=0.
      R_PL(2,N_PL)=0.
      R_PL(3,N_PL)=ZIN
      NORM(1,N_PL)=0.
      NORM(2,N_PL)=0.
      NORM(3,N_PL)=1.
      ITIP(N_PL)=1

      N_PL=N_PL+1
      R_PL(1,N_PL)=0.
      R_PL(2,N_PL)=0.
      R_PL(3,N_PL)=ZOU
      NORM(1,N_PL)=0.
      NORM(2,N_PL)=0.
      NORM(3,N_PL)=-1.
      ITIP(N_PL)=0

      N_PL=N_PL+1
      R_PL(1,N_PL)=-AH
      R_PL(2,N_PL)=0.
      R_PL(3,N_PL)=ZOU
      NORM(1,N_PL)=1.
      NORM(2,N_PL)=0.
      NORM(3,N_PL)=0.
      ITIP(N_PL)=2

      N_PL=N_PL+1
      R_PL(1,N_PL)=AH
      R_PL(2,N_PL)=0.
      R_PL(3,N_PL)=ZOU
      NORM(1,N_PL)=-1.
      NORM(2,N_PL)=0.
      NORM(3,N_PL)=0.
      ITIP(N_PL)=2

      N_PL=N_PL+1
      R_PL(1,N_PL)=0.
      R_PL(2,N_PL)=EXT_YDN(ID)
      R_PL(3,N_PL)=ZOU
      IF(EXT_SHAPE(ID).EQ.1) THEN
         NORM(1,N_PL)=0.
         NORM(2,N_PL)=1.
         NORM(3,N_PL)=0.
      ELSE
         PHI=ATAN2(EXT_YDN(ID)-(-BH),EXT_DZ(ID))
         NORM(1,N_PL)=0.
         NORM(2,N_PL)=COS(PHI)
         NORM(3,N_PL)=-SIN(PHI)
      ENDIF
      ITIP(N_PL)=2
      IF(EXT_SHAPE(ID).EQ.0) ITIP(N_PL)=-1

      N_PL=N_PL+1
      R_PL(1,N_PL)=0.
      R_PL(2,N_PL)=EXT_YUP(ID)
      R_PL(3,N_PL)=ZOU
      IF(EXT_SHAPE(ID).EQ.1) THEN
         NORM(1,N_PL)=0.
         NORM(2,N_PL)=-1.
         NORM(3,N_PL)=0.
      ELSE
         PHI=ATAN2(EXT_YUP(ID)-BH,EXT_DZ(ID))
         NORM(1,N_PL)=0.
         NORM(2,N_PL)=-COS(PHI)
         NORM(3,N_PL)=SIN(PHI)
      ENDIF
      ITIP(N_PL)=2
      IF(EXT_SHAPE(ID).EQ.0) ITIP(N_PL)=-1

      IF(LR.EQ.0) THEN
         DO I=1,N_PL
            R_PL(3,I)=-R_PL(3,I)
            NORM(3,I)=-NORM(3,I)
         ENDDO
      ENDIF

      END


      SUBROUTINE FOC_MIRROR(R0,DIR,MDN,ITIP,TLEN,NX,NY,OK)

      IMPLICIT NONE
      REAL R0(3), DIR(3)        ! point, direction [in/out]
      INTEGER MDN               ! Q-bar module number [in]
      INTEGER ITIP              ! focusing mirror type
      REAL TLEN                 ! length-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed number of reflections [out]
      LOGICAL OK                ! status [out]

#include "TOP_GEO.fi"

      REAL PATH, PATH1, PATH2, R(3), A, X, AH, BH, KX, KY
      REAL DIR1(3), RC(3), RMI, FOCL
      INTEGER I, NN

      TLEN=0.
      NX=0
      NY=0

      RC(1)=QBAR_XCMI(MDN)
      RC(2)=QBAR_YCMI(MDN)
      RC(3)=QBAR_ZCMI(MDN)
      RMI=QBAR_RMI(MDN)

      IF(LIN_OPT) THEN
         FOCL=QBAR_FOCL(MDN)
         OK=FOCL.NE.0
         IF(.NOT.OK) RETURN
         CALL LINEAR_OPTICS(R0,DIR,RC,FOCL,ITIP)
         RETURN
      ENDIF

      AH=QBAR_A(MDN)/2
      BH=QBAR_B(MDN)/2
      KX=DIR(1)
      KY=DIR(2)

      NN=0
      DO WHILE(TLEN.LT.1000)    ! 50 ns max. time
         CALL MIRROR_HIT(R0,DIR,RC,RMI,ITIP,PATH,OK)
         IF(.NOT.OK) RETURN
         CALL NEW_POINT(R0,DIR,PATH,R)

         IF(ABS(R(1)).LE.AH.AND.ABS(R(2)).LE.BH) THEN
            TLEN=TLEN+PATH
            CALL MIRROR_REFLECT(R,DIR,RC,RMI,ITIP,DIR1,A)
            DO I=1,3
               R0(I)=R(I)
               DIR(I)=DIR1(I)
            ENDDO
            IF(KX.LT.0) NX=-NX
            IF(KY.LT.0) NY=-NY
            RETURN
         ENDIF

         IF(ABS(DIR(1)).GT.1E-10) THEN
            PATH1=(SIGN(AH,DIR(1))-R0(1))/DIR(1)
         ELSE
            PATH1=1.E10
         ENDIF
         IF(ABS(DIR(2)).GT.1E-10) THEN
            PATH2=(SIGN(BH,DIR(2))-R0(2))/DIR(2)
         ELSE
            PATH2=1.E10
         ENDIF
         IF(PATH1.LT.PATH2) THEN
            PATH=PATH1
            NX=NX+1
            CALL NEW_POINT(R0,DIR,PATH,R0)
            DIR(1)=-DIR(1)
         ELSE
            PATH=PATH2
            NY=NY+1
            CALL NEW_POINT(R0,DIR,PATH,R0)
            DIR(2)=-DIR(2)
         ENDIF
         TLEN=TLEN+PATH
         NN=NN+1
         IF(NN.GT.10000) THEN
            PRINT*,'foc_mirror_: loop limit reached, n=',NN
            EXIT
         ENDIF
      ENDDO
      OK=.FALSE.

      END


      SUBROUTINE EDGE_ABSORB(R0,DIR,MDN,I,J,OK)

      IMPLICIT NONE
      REAL R0(3), DIR(3)        ! starting point and direction
      INTEGER MDN               ! Qbar number
      INTEGER I, J              ! num. of reflections in x and y (signed)
      LOGICAL OK                ! T= no absorption
#include "TOP_GEO.fi"
      LOGICAL QBAR_EDGE

      IF(QBAR_REDG.LE.0) RETURN

      OK=.NOT.QBAR_EDGE(R0(1),R0(2),DIR(1),DIR(2),QBAR_A(MDN),
     &     QBAR_B(MDN),QBAR_REDG,I)
      IF(.NOT.OK) RETURN

      OK=.NOT.QBAR_EDGE(R0(2),R0(1),DIR(2),DIR(1),QBAR_B(MDN),
     &     QBAR_A(MDN),QBAR_REDG,J)

      END


      LOGICAL FUNCTION QBAR_EDGE(X0,Y0,KX,KY,A,B,R,N)

      IMPLICIT NONE
      REAL X0, Y0, KX, KY       ! start point, direction
      REAL A, B                 ! cell dimensions 
      REAL R                    ! sharpness
      INTEGER N                 ! number of reflections in x

      REAL X, Y, DX, DY, YW, YEDG
      INTEGER I, J

      QBAR_EDGE=.FALSE.
      IF(N.EQ.0) RETURN
      IF(ABS(KX).LT.1E-6) RETURN

      IF(N.GT.0) THEN
         X=A/2
         DX=A
      ELSE
         X=-A/2
         DX=-A
      ENDIF
      Y=Y0+(X-X0)*KY/KX
      DY=DX*KY/KX
      YEDG=B/2-R
      DO I=1,ABS(N)
         J=NINT(Y/B)
         YW=Y-B*J
         IF(ABS(YW).GT.YEDG) THEN
            QBAR_EDGE=.TRUE.
            RETURN
         ENDIF
         X=X+DX
         Y=Y+DY
      ENDDO

      END


c--   raytracing used in reconstruction (R0 must be inside Qbar)


      SUBROUTINE TOPF_TRACE(RF,DIR,MDN,TLEN,LR,XD,YD,YB,NX,NY,OK)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! photon point, direction [in/out]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! length-of-propagation (cm) [out]
      INTEGER LR                ! 0/1=left/right exit window [out]
      REAL XD, YD, YB           ! unfolded coordinates [out]
      INTEGER NX(3), NY(3)      ! (signed) number of reflections [out]
      LOGICAL OK                ! status [out]

#include "TOP_CONS.fi"
#include "TOP_GEO.fi"
      INTEGER KAM, KJE, I, NN, NX1, NY1, ITIP
      REAL TL
      INTEGER N

      OK=ABS(DIR(3)).GT.1E-5
      IF(.NOT.OK) RETURN

      TLEN=0.
      DO I=1,3
         NX(I)=0
         NY(I)=0
      ENDDO

      NN=0
      CALL UNFOLDED_RESET
      CALL KJE_FOTON(RF,MDN,KAM,LR)
      DO WHILE(.TRUE.)
         KJE=KAM
         IF(KAM.EQ.1) THEN
            CALL TRACE_QBAR(RF,DIR,MDN,TL,NX1,NY1,KAM,LR)
            ITIP=0
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
         ELSE IF(KAM.EQ.2) THEN
            CALL TRACE_MIRR(RF,DIR,MDN,TL,NX1,NY1,KAM,LR,ITIP)
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
            NX(2)=NX(1)
            NY(2)=NY(1)
         ELSE IF(KAM.EQ.3) THEN
            CALL TRACE_EVOL(RF,DIR,MDN,TL,NX1,NY1,KAM,LR)
            ITIP=0
            CALL ADD_NUMREFL(NX(1),NX1)
            CALL ADD_NUMREFL(NY(1),NY1)
            NX(3)=NX1
            NY(3)=NY1
         ELSE
            EXIT
         ENDIF
         TLEN=TLEN+TL
         CALL UNFOLDED_PUSH(KJE,RF,NX1,NY1,MDN,LR,ITIP)
         NN=NN+1
         IF(NN.GT.100) THEN
            PRINT*,'topf_trace_: loop limit reached, n=',NN
            EXIT
         ENDIF
      ENDDO
      OK=KAM.EQ.0
      IF(.NOT.OK) RETURN

      CALL UNFOLDED_GET(XD,YD,YB)

      END


      SUBROUTINE UNFOLDED_PUSH(KJE,RF,NX,NY,MDN,LR,ITIP)

      IMPLICIT NONE
      INTEGER KJE
      REAL RF(3)
      INTEGER NX, NY, MDN, LR, ITIP
      REAL XD, YD, YB

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      INTEGER ID, I, K
      REAL XX(2)

      integer NB
      PARAMETER (NB=10)
      INTEGER NT
      REAL X(2,NB), X0(2,NB), A(2,NB)
      INTEGER N(2,NB), TIP(NB)
      SAVE NT,X,X0,A,N,TIP

      NT=NT+1
      IF(NT.GT.NB) RETURN
      X(1,NT)=RF(1)
      X(2,NT)=RF(2)
      N(1,NT)=NX
      N(2,NT)=NY
      TIP(NT)=KJE
      IF(KJE.EQ.1) THEN
         A(1,NT)=QBAR_A(MDN)
         A(2,NT)=QBAR_B(MDN)
         X0(1,NT)=0.
         X0(2,NT)=0.
      ELSE IF(KJE.EQ.2) THEN
         IF(ITIP.EQ.4) THEN
            DO I=1,NT
               A(1,I)=0.
               A(2,I)=0.
               X0(1,I)=QBAR_XCMI(MDN)
               X0(2,I)=QBAR_YCMI(MDN)
            ENDDO
         ELSE IF(ITIP.EQ.3) THEN
            A(1,NT)=QBAR_A(MDN)
            X0(1,NT)=0.
            DO I=1,NT
               A(2,I)=0.
               X0(2,I)=QBAR_YCMI(MDN)
            ENDDO
         ELSE
            A(1,NT)=QBAR_A(MDN)
            A(2,NT)=QBAR_B(MDN)
            X0(1,NT)=0.
            X0(2,NT)=0.
         ENDIF
      ELSE IF(KJE.EQ.3) THEN
         ID=QBAR_EXT(LR,MDN)
         IF(ID.EQ.0) RETURN
         A(1,NT)=QBAR_A(MDN)
         A(2,NT)=EXT_YUP(ID)-EXT_YDN(ID)
         X0(1,NT)=0.
         X0(2,NT)=(EXT_YUP(ID)+EXT_YDN(ID))/2
      ENDIF

      RETURN

      ENTRY UNFOLDED_RESET

      NT=0
      RETURN

      ENTRY UNFOLDED_GET(XD,YD,YB)

      DO I=1,2
         XX(I)=X(I,NT)
         DO K=NT,1,-1
            XX(I)=XX(I)-X0(I,K)
            IF(MOD(N(I,K),2).EQ.0) THEN
               XX(I)=A(I,K)*N(I,K)+XX(I)
            ELSE
               XX(I)=A(I,K)*N(I,K)-XX(I)
            ENDIF
            XX(I)=XX(I)+X0(I,K)
         ENDDO
      ENDDO
      XD=XX(1)
      YD=XX(2)
      YB=YD
      IF(TIP(NT).NE.3) RETURN

      YB=X(2,NT-1)
      DO K=NT-1,1,-1
         YB=YB-X0(2,K)
         IF(MOD(N(2,K),2).EQ.0) THEN
            YB=A(2,K)*N(2,K)+YB
         ELSE
            YB=A(2,K)*N(2,K)-YB
         ENDIF
         YB=YB+X0(2,K)
      ENDDO

      END



      SUBROUTINE TRACE_QBAR(RF,DIR,MDN,TLEN,NX,NY,KAM,LR)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! point, direction [in/out]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! length-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections [out]
      INTEGER KAM               ! next object [in/out]
      INTEGER LR                ! on left/right [out]

#include "TOP_GEO.fi"
      REAL XW, YW, ZW
      INTEGER I, J, ITIP
      REAL X, Y

      IF(KAM.NE.1) STOP 'KAM<>1'
      KAM=-1

      IF(DIR(3).GT.0) THEN
         LR=1
         ZW=QBAR_ZR(MDN)
         IF(LIN_OPT) ZW=QBAR_C(MDN)/2
         ITIP=QBAR_RSIDE(MDN)
      ELSE
         LR=0
         ZW=QBAR_ZL(MDN)
         IF(LIN_OPT) ZW=-QBAR_C(MDN)/2
         ITIP=QBAR_LSIDE(MDN)
      ENDIF
      TLEN=(ZW-RF(3))/DIR(3)
      IF(TLEN.LT.0) RETURN
      IF(TLEN.GT.10000) RETURN

!     photon on exit plane

      X=RF(1)+TLEN*DIR(1)
      Y=RF(2)+TLEN*DIR(2)

      I=NINT(X/QBAR_A(MDN))
      J=NINT(Y/QBAR_B(MDN))

!     exit window

      XW=X-QBAR_A(MDN)*I
      IF(MOD(I,2).NE.0) THEN
         XW=-XW
         DIR(1)=-DIR(1)
      ENDIF
      YW=Y-QBAR_B(MDN)*J
      IF(MOD(J,2).NE.0) THEN
         YW=-YW
         DIR(2)=-DIR(2)
      ENDIF
      RF(1)=XW
      RF(2)=YW
      RF(3)=ZW
      NX=I
      NY=J

      KAM=0
      IF(ITIP.GT.1) KAM=2
      IF(QBAR_EXT(LR,MDN).GT.0) KAM=3

      END


      SUBROUTINE TRACE_MIRR(RF,DIR,MDN,TLEN,NX,NY,KAM,LR,ITIP)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! point, direction [in/out]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! lenght-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections [out]
      INTEGER KAM               ! next object [in/out]
      INTEGER LR                ! on left/right [in]
      INTEGER ITIP

#include "TOP_GEO.fi"
      LOGICAL OK

      IF(KAM.NE.2) STOP 'KAM<>2'

      TLEN=0
      NX=0
      NY=0
      KAM=1
      IF(LR.EQ.0) THEN
         IF(DIR(3).GT.0) RETURN
         ITIP=QBAR_LSIDE(MDN)
      ELSE
         IF(DIR(3).LT.0) RETURN
         ITIP=QBAR_RSIDE(MDN)
      ENDIF

      KAM=-1
      IF(ITIP.EQ.2) THEN        ! planar mirror
         DIR(3)=-DIR(3)
      ELSE IF(ITIP.EQ.3.OR.ITIP.EQ.4) THEN ! cylindrical or spherical mirror
         CALL FOC_MIRROR(RF,DIR,MDN,ITIP,TLEN,NX,NY,OK)
         IF(.NOT.OK) RETURN
      ELSE
         STOP 'napaka: tip ni zrcalo!!'
      ENDIF
      KAM=1

      END


      SUBROUTINE TRACE_EVOL(RF,DIR,MDN,TLEN,NX,NY,KAM,LR)

      IMPLICIT NONE
      REAL RF(3), DIR(3)        ! point, direction [in/out]
      INTEGER MDN               ! Q-bar module number [in]
      REAL TLEN                 ! lenght-of-propagation (cm) [out]
      INTEGER NX, NY            ! signed num. of reflections [out]
      INTEGER KAM               ! next object [in/out]
      INTEGER LR                ! on left/right [in]

#include "TOP_GEO.fi"
#include "TOP_EXT.fi"
      INTEGER ID, I, I0, NN
      REAL PATH, PATH1, R(3), DIRNOR, DIRNOR1, AH, BH, KX, KY
      REAL RNOR
      REAL S_PROD
      real PATH_MAX
      PARAMETER (PATH_MAX=1.E10)
      REAL R_PL(3,10), NORM(3,10)
      INTEGER ITIP(10), N_PL
      INTEGER ID_OLD /0/, MDN_OLD /-1/
      SAVE R_PL, NORM, ITIP, N_PL, ID_OLD, MDN_OLD

      IF(KAM.NE.3) STOP 'KAM<>3'

      TLEN=0
      NX=0
      NY=0
      KAM=-1

      ID=QBAR_EXT(LR,MDN)
      IF(ID.EQ.0) RETURN

      IF(ID.NE.ID_OLD.OR.MDN.NE.MDN_OLD) THEN
         ID_OLD=ID
         MDN_OLD=MDN
         CALL SET_EVOL(MDN,ID,LR,R_PL,NORM,ITIP,N_PL)
      ENDIF
      AH=QBAR_A(MDN)/2
      BH=QBAR_B(MDN)/2
      KX=DIR(1)
      KY=DIR(2)

      I0=0
      NN=0
      DO WHILE(TLEN.LT.1000)    ! 50 ns max. time
         PATH=1.E10
         DO I=1,N_PL
            IF(I.EQ.I0) CYCLE
            DIRNOR1=S_PROD(DIR,NORM(1,I))
            IF(DIRNOR1.LT.0) THEN
               R(1)=RF(1)-R_PL(1,I)
               R(2)=RF(2)-R_PL(2,I)
               R(3)=RF(3)-R_PL(3,I)
               RNOR=S_PROD(R,NORM(1,I))
               IF(RNOR.LT.-PATH_MAX*DIRNOR1) THEN
                  PATH1=-RNOR/DIRNOR1
                  IF(PATH1.GE.0.AND.PATH1.LT.PATH) THEN
                     PATH=PATH1
                     DIRNOR=DIRNOR1
                     I0=I
                  ENDIF
               ENDIF
            ENDIF
         ENDDO
         IF(I0.EQ.0) RETURN
         CALL NEW_POINT(RF,DIR,PATH,RF)
         TLEN=TLEN+PATH

         IF(ITIP(I0).EQ.0) THEN
            KAM=0
            EXIT
         ELSE IF(ITIP(I0).EQ.1) THEN
            IF(ABS(RF(1)).LE.AH.AND.ABS(RF(2)).LE.BH) KAM=1
            EXIT
         ELSE IF(ITIP(I0).EQ.2.OR.ITIP(I0).EQ.3) THEN
            DO I=1,3
               DIR(I)=DIR(I)-2*DIRNOR*NORM(I,I0)
            ENDDO
            IF(NORM(1,I0).NE.0) THEN
               NX=NX+1
            ELSE
               NY=NY+1
            ENDIF
         ELSE
            EXIT
         ENDIF
         NN=NN+1
         IF(NN.GT.10000) THEN
            PRINT*,'trace_evol_: loop limit reached, n=',NN
            EXIT
         ENDIF
      ENDDO
      IF(KX.LT.0) NX=-NX
      IF(KY.LT.0) NY=-NY

      END


      SUBROUTINE ADD_NUMREFL(N,I)

      IMPLICIT NONE
      INTEGER N, I

      IF(N.EQ.0) THEN
         N=I
      ELSE
         N=N+SIGN(I,N)
      ENDIF

      END


c--   basic raytracing -------------------------------------------------

      SUBROUTINE NEW_POINT(R,DIR,PATH,RH)

! moves the point R() along DIR() by PATH

      IMPLICIT NONE
      REAL R(3), DIR(3), PATH   ! [in]
      REAL RH(3)                ! [out]

      INTEGER I

      DO I=1,3
         RH(I)=R(I)+PATH*DIR(I)
      ENDDO

      RETURN
      END


      SUBROUTINE MIRROR_HIT(R0,DIR,RC,RMI,ITIP,PATH,OK)

      IMPLICIT NONE
      REAL R0(3), DIR(3)        ! photon [in]
      REAL RC(3), RMI           ! mirror center and radius [in]
      INTEGER ITIP              ! mirror type [in]
      REAL PATH                 ! length from R0 to hit [out]
      LOGICAL OK                ! true, if solution exists  [out]

      REAL R(3), RDIR, RR, D, SS

      IF(ITIP.EQ.3) THEN        ! cylindric
         SS=DIR(2)**2+DIR(3)**2
         OK=SS.GT.0
         IF(.NOT.OK) RETURN
         R(2)=R0(2)-RC(2)
         R(3)=R0(3)-RC(3)
         RDIR=R(2)*DIR(2)+R(3)*DIR(3)
         RR=R(2)**2+R(3)**2
         D=RDIR**2+(RMI**2-RR)*SS
         OK=D.GE.0
         IF(.NOT.OK) RETURN
         D=SQRT(D)
         PATH=(D-RDIR)/SS
      ELSE                      ! spheric
         R(1)=R0(1)-RC(1)
         R(2)=R0(2)-RC(2)
         R(3)=R0(3)-RC(3)
         RDIR=R(1)*DIR(1)+R(2)*DIR(2)+R(3)*DIR(3)
         RR=R(1)**2+R(2)**2+R(3)**2
         D=RDIR**2+RMI**2-RR
         OK=D.GE.0
         IF(.NOT.OK) RETURN
         D=SQRT(D)
         PATH=D-RDIR
      ENDIF

      END


      SUBROUTINE MIRROR_REFLECT(R0,DIR,RC,RMI,ITIP,DIR_REFL,A)

c--   R0 must be on the mirror surface!

      IMPLICIT NONE
      REAL R0(3), DIR(3)        ! photon (point on the mirror) [in]
      REAL RC(3), RMI           ! mirror center and radius [in]
      INTEGER ITIP              ! mirror type [in]
      REAL DIR_REFL(3)          ! reflected ray [out]
      REAL A                    ! if A<0, hit into surface with refl. coating

      REAL NORM(3)
      INTEGER I

      IF(ITIP.EQ.3) THEN        ! cylindric
         NORM(1)=0.
         NORM(2)=(RC(2)-R0(2))/RMI
         NORM(3)=(RC(3)-R0(3))/RMI
      ELSE                      ! spheric
         NORM(1)=(RC(1)-R0(1))/RMI
         NORM(2)=(RC(2)-R0(2))/RMI
         NORM(3)=(RC(3)-R0(3))/RMI
      ENDIF
      A=DIR(1)*NORM(1)+DIR(2)*NORM(2)+DIR(3)*NORM(3)
      DO I=1,3
         DIR_REFL(I)=DIR(I)-2.*A*NORM(I)
      ENDDO

      END


      SUBROUTINE LINEAR_OPTICS(R0,DIR,RC,F,ITIP)

c--   R0 must be on the bar exit surface!

      IMPLICIT NONE
      REAL R0(3), DIR(3), RC(3), F
      INTEGER ITIP

      LOGICAL START /.TRUE./
      SAVE START

      IF(START) THEN
         IF(ITIP.EQ.3) THEN
            PRINT*,'raytrace with linear optics (cylin. mirror), f=',F
         ELSE
            PRINT*,'raytrace with linear optics (spher. mirror), f=',F
         ENDIF
         START=.FALSE.
      ENDIF
      
      IF(ITIP.EQ.3) THEN
         DIR(2)=DIR(2)-(R0(2)-RC(2))/F*DIR(3)
         DIR(3)=-DIR(3)
      ELSE
         DIR(1)=DIR(1)-(R0(1)-RC(1))/F*DIR(3)
         DIR(2)=DIR(2)-(R0(2)-RC(2))/F*DIR(3)
         DIR(3)=-DIR(3)
      ENDIF
      CALL ENOTNI_VEKT(DIR)

      END


