!-----------------------------------------------------------------------
! TOP counter for Belle: initialization of/interface to TOP_PMT, TOP_QEFF
! M. Staric, mar-2009, sept-2011, 
! units: GeV, GeV/c, cm, ns, cm/ns, T, nm
! mar-2017: change in some commons
!-----------------------------------------------------------------------

      SUBROUTINE SET_PMT(A,B,AA,BB,NX,NY,TTS)

      IMPLICIT NONE
      REAL A, B                 ! PMT dimensions:  x-size, y-size [cm]
      REAL AA, BB               ! PMT active area: x-size, y-size [cm]
      INTEGER NX, NY            ! PMT number of channels in x, y
      REAL TTS                  ! PMT time resolution [ns] (single Gaussian)
#include "TOP_PMT.fi"
      DATA PMT_INI /.FALSE./

      PMT_A=A
      PMT_B=B
      PMT_AA=AA
      PMT_BB=BB
      PMT_NX=NX
      PMT_NY=NY
      PMT_DXCH=PMT_AA/PMT_NX
      PMT_DYCH=PMT_BB/PMT_NY
      PMT_DELX=PMT_AA/2.
      PMT_DELY=PMT_BB/2.
      SIG_XCH=PMT_DXCH/SQRT(12.)
      SIG_YCH=PMT_DYCH/SQRT(12.)
      NMA_PMT=0
      TTS_NG=1
      TTS_FRAC(1)=1.
      TTS_T0(1)=0.
      TTS_SIG(1)=TTS
      PMT_INI=.TRUE.

      END


      SUBROUTINE SET_TTS(NG,FRAC,T0,SIG)

      IMPLICIT NONE
      INTEGER NG                ! number of Gaussians
      REAL FRAC(*)              ! fractions
      REAL T0(*)                ! positions [ns]
      REAL SIG(*)               ! sigmas [ns]
#include "TOP_PMT.fi"
      INTEGER I
      REAL S
      
      S=0.
      DO I=1,MIN(NG,MAX_G)
         S=S+FRAC(I)
      ENDDO
      IF(S.EQ.0) RETURN
      DO I=1,MIN(NG,MAX_G)
         TTS_NG=I
         TTS_FRAC(I)=FRAC(I)/S
         TTS_T0(I)=T0(I)
         TTS_SIG(I)=SIG(I)
      ENDDO

      END


      SUBROUTINE READ_QEFFI(QE_FILE, CE)

      IMPLICIT NONE
      CHARACTER*(*) QE_FILE     ! datafile with quantum effi. table (lam,qe) 
      REAL CE                   ! collection efficiency
#include "TOP_QEFF.fi"      
      DATA QE_INIT /.FALSE./

      integer NSIZ
      PARAMETER (NSIZ=1000)
      INTEGER NT
      REAL LAM(NSIZ), QEF(NSIZ)
      INTEGER IO, L
      REAL X,Y

      NT=0
      OPEN(1,FILE=QE_FILE,STATUS='OLD',IOSTAT=IO)
      IF(IO.NE.0) THEN
         PRINT*,'**** error QE_READ: file ', QE_FILE,' not found'
         RETURN
      ENDIF
      DO WHILE(.TRUE.)
 1       CONTINUE
         READ(1,*,ERR=1,END=2) X,Y
         NT=NT+1
         IF(NT.GT.NSIZ) THEN
            PRINT*,'**** warning: QE_READ - too many points'
            EXIT
         ENDIF
         LAM(NT)=X
         QEF(NT)=Y
      ENDDO
 2    CLOSE(1)

      L=INDEX(QE_FILE,' ')
      IF(L.EQ.0) L=LEN(QE_FILE)
      PRINT*,'PMT QE: ',NT,' datapoints read from ',QE_FILE(1:L)

      CALL SET_QEFFI(LAM,QEF,NT,CE)

      END


      SUBROUTINE SET_QEFFI(LAM,QEF,NT,CE)

      IMPLICIT NONE
      REAL LAM(*), QEF(*)       ! QE datapoints: lambda [nm], QE
      INTEGER NT                ! number of datapoints
      REAL CE                   ! collection efficiency
#include "TOP_QEFF.fi"      
      REAL X, S, LIN_INTERP
      INTEGER I

      IF(NT.EQ.0) RETURN

      DO I=1,NT
         IF(QEF(I).GT.0) EXIT
      ENDDO
      I=MAX(I-1,1)
      EMA=1240./LAM(I)

      DO I=NT,1,-1
         IF(QEF(I).GT.0) EXIT
      ENDDO
      I=MIN(I+1,NT)
      EMI=1240./LAM(I)

      IF(EMA.LE.EMI) RETURN
      E_STEP=(EMA-EMI)/(MAX_NT-1)
      X=EMI
      S=0.
      QEMAX=0.
      DO I=1,MAX_NT
         QE(I)=MAX(LIN_INTERP(LAM,QEF,NT,1240./X),0.)*CE
         QEMAX=MAX(QEMAX,QE(I))
         S=S+QE(I)
         QE_E(I)=X
         QE_S(I)=QE(I)
         QE_SE(I)=QE(I)*X
         QE_SEE(I)=QE(I)*X**2
         X=X+E_STEP
      ENDDO
      IF(S.EQ.0) RETURN
      DO I=1,MAX_NT
         QE_S(I)=QE_S(I)/S
         QE_SE(I)=QE_SE(I)/S
         QE_SEE(I)=QE_SEE(I)/S
      ENDDO
      NT_XQE=MAX_NT
      DO I=1,NT_XQE
         XQE_E(I)=QE_E(I)
         XQE_S(I)=QE_S(I)
         XQE_SE(I)=QE_SE(I)
         XQE_SEE(I)=QE_SEE(I)
      ENDDO
      XQE_FLAG=.TRUE.
      EMIN=XQE_E(1)
      EMAX=XQE_E(NT_XQE)
      XQE_MAXSEP=1./E_STEP
c      PRINT*,'PMT max(CE*QE):',QEMAX,' CE: ',CE
c      PRINT*,'Emin, Emax (eV): ',EMI,EMA
      PRINT*
      QE_INIT=.TRUE.

      END


      REAL FUNCTION LIN_INTERP(XT,YT,NT,X)

      IMPLICIT NONE
      REAL XT(*), YT(*), X
      INTEGER NT
      INTEGER I

      IF(X.LE.XT(1).OR.X.GT.XT(NT)) THEN
         LIN_INTERP=0.
      ELSE
         DO I=2,NT
            IF(X.LE.XT(I)) EXIT
         ENDDO
         LIN_INTERP=YT(I)+(YT(I)-YT(I-1))/(XT(I)-XT(I-1))*(X-XT(I))
      ENDIF

      END


c---- quantum efficiency: functions ------------------------------------

      REAL FUNCTION QE_PMT(E)

      IMPLICIT NONE
      REAL E                    ! photon energy (eV)
#include "TOP_QEFF.fi"      
      INTEGER I

      QE_PMT=0.
      IF(.NOT.QE_INIT) RETURN
      I=NINT((E-EMI)/E_STEP)+1
      IF(I.LT.1.OR.I.GT.MAX_NT) RETURN
      QE_PMT=QE(I)

      END

      
      SUBROUTINE QE_RANGE(E_MIN,E_MAX)
      
      IMPLICIT NONE
      REAL E_MIN, E_MAX         ! PMT sensitivity range (eV) 
#include "TOP_QEFF.fi"      

      E_MIN=EMI
      E_MAX=EMA

      END


      REAL FUNCTION QE_MAX()

      IMPLICIT NONE
#include "TOP_QEFF.fi"      

      QE_MAX=QEMAX

      END


      REAL FUNCTION QE_NZERO()

      IMPLICIT NONE
#include "TOP_QEFF.fi"      
      INTEGER I

      QE_NZERO=0.
      IF(.NOT.QE_INIT) RETURN

      DO I=1,MAX_NT
         QE_NZERO=QE_NZERO+QE(I)
      ENDDO
      QE_NZERO=370.*QE_NZERO*E_STEP

      END


      SUBROUTINE QE_STAT(E1,E2,MEAN,SIG2,FRAC)

      IMPLICIT NONE
      REAL E1, E2               ! range in eV [in]
      REAL MEAN                 ! mean [out]
      REAL SIG2                 ! rms**2 [out]
      REAL FRAC                 ! fraction [out]
#include "TOP_QEFF.fi"      
      INTEGER I1, I2, I
      REAL X, X2

      IF(.NOT.QE_INIT) RETURN

      IF(E1.LT.E2) THEN
         I1=NINT((E1-EMI)/E_STEP)+1
         I2=NINT((E2-EMI)/E_STEP)+1
      ELSE
         I2=NINT((E1-EMI)/E_STEP)+1
         I1=NINT((E2-EMI)/E_STEP)+1
      ENDIF

      I1=MAX(I1,1)
      I2=MIN(I2,MAX_NT)
      IF(I1.EQ.I2) THEN
         MEAN=(E1+E2)/2
         SIG2=(E2-E1)**2/12
         FRAC=QE_S(I1)*ABS(E2-E1)/E_STEP
      ELSE
         FRAC=0
         X=0
         X2=0
         DO I=I1,I2
            FRAC=FRAC+QE_S(I)
            X=X+QE_SE(I)
            X2=X2+QE_SEE(I)
         ENDDO
         IF(FRAC.EQ.0) THEN
            MEAN=(E1+E2)/2
            SIG2=(E2-E1)**2/12
         ELSE
            X=X/FRAC
            X2=X2/FRAC
            MEAN=X
            SIG2=X2-X**2
         ENDIF
      ENDIF

      END


      SUBROUTINE QE_SET_MASK

#include "TOP_QEFF.fi"      
      INTEGER I

      DO I=1,MAX_NT
         QE_MASK(I)=1.
      ENDDO
      QE_I1=1
      QE_I2=MAX_NT

      END


      SUBROUTINE QE_RESET_MASK

#include "TOP_QEFF.fi"      
      INTEGER I

      DO I=1,MAX_NT
         QE_MASK(I)=0.
      ENDDO
      QE_I1=MAX_NT
      QE_I2=1

      END


      SUBROUTINE QE_ADD_MASK(MASK,N,NSIZ,E0)

      INTEGER NSIZ
      REAL MASK(0:NSIZ-1)
      INTEGER N
      REAL E0
#include "TOP_QEFF.fi"      
      INTEGER I0, I, K, K1, K2, II
      REAL PP, A, B, C, WY, E
      REAL CONV_SQUARES
 
      IF(N.LE.NSIZ) THEN
         IF(N.EQ.0) RETURN
         I0=NINT((E0-EMI)/E_STEP)+1
         K1=MAX(-(N-1),1-I0)
         K2=MIN(N-1,MAX_NT-I0)
         DO K=K1,K2
            I=K+I0
            II=ABS(K)
            QE_MASK(I)=QE_MASK(I)+MASK(II)
         ENDDO
         QE_I1=MIN(QE_I1,K1+I0)
         QE_I2=MAX(QE_I2,K2+I0)
      ELSE
         A=MASK(0)
         IF(A.EQ.0) RETURN
         B=MASK(1)
         C=MASK(2)
         WY=MASK(3)
         E=EMI+E_STEP/2-E0
         DO I=1,MAX_NT
            PP=CONV_SQUARES(E,A,B,C)
            IF(PP.GT.0) THEN
               QE_MASK(I)=QE_MASK(I)+PP*WY
               QE_I1=MIN(QE_I1,I)
               QE_I2=MAX(QE_I2,I)
            ENDIF
            E=E+E_STEP
         ENDDO         
      ENDIF

      END


      SUBROUTINE QE_SUM_MASK(MEAN,SIG2,FRAC)

      IMPLICIT NONE
      REAL MEAN                 ! mean photon energy [out]
      REAL SIG2                 ! rms**2 [out]
      REAL FRAC                 ! fraction [out]
#include "TOP_QEFF.fi"      
      INTEGER I

      FRAC=0.
      MEAN=0.
      SIG2=0.
      DO I=QE_I1, QE_I2
         FRAC=FRAC+QE_S(I)*QE_MASK(I)
         MEAN=MEAN+QE_SE(I)*QE_MASK(I)
         SIG2=SIG2+QE_SEE(I)*QE_MASK(I)
      ENDDO
      IF(FRAC.EQ.0) RETURN
      MEAN=MEAN/FRAC
      SIG2=SIG2/FRAC-MEAN**2

      END


      SUBROUTINE QE_SET_XQE(SIGE_SIP)

      IMPLICIT NONE
      REAL SIGE_SIP
#include "TOP_QEFF.fi"      
      REAL G(0:MAX_SIZ-1)
      INTEGER I, K, KK, N, NN
      REAL E, S
      REAL XE, XSIGE, XE1, DDE, SEP

      NN=NINT(3.*ABS(SIGE_SIP)/E_STEP)
      N=MIN(NN,(MAX_SIZ-MAX_NT)/2)
      XQE_FLAG=N.EQ.NN

      IF(N.EQ.0) THEN
         NT_XQE=MAX_NT
         DO I=1,NT_XQE
            XQE_E(I)=QE_E(I)
            XQE_S(I)=QE_S(I)
            XQE_SE(I)=QE_SE(I)
            XQE_SEE(I)=QE_SEE(I)
         ENDDO
         EMIN=XQE_E(1)
         EMAX=XQE_E(NT_XQE)
         XQE_MAXSEP=1./E_STEP
         RETURN
      ENDIF

      E=0.
      G(0)=1
      S=G(0)
      DO I=1,MAX_SIZ-1
         E=E+E_STEP
         G(I)=EXP(-0.5*(E/SIGE_SIP)**2)
         S=S+2*G(I)
      ENDDO
      DO I=0,MAX_SIZ-1
         G(I)=G(I)/S
      ENDDO
      NT_XQE=MAX_NT+2*N
      E=EMI-N*E_STEP
      DO I=1,NT_XQE
         XQE_E(I)=E
         XQE_S(I)=0.
         XQE_SE(I)=0.
         XQE_SEE(I)=0.
         DO K=1,MAX_NT
            KK=ABS(K-I+N)
            IF(KK.GT.MAX_SIZ-1) CYCLE
            XQE_S(I)=XQE_S(I)+QE_S(K)*G(KK)
            XQE_SE(I)=XQE_SE(I)+QE_SE(K)*G(KK)
            XQE_SEE(I)=XQE_SEE(I)+QE_SEE(K)*G(KK)
         ENDDO
         E=E+E_STEP
      ENDDO
      EMIN=XQE_E(1)
      EMAX=XQE_E(NT_XQE)
      XQE_MAXSEP=0.
      DO I=1,NT_XQE-1
         IF(XQE_S(I).GT.0.AND.XQE_S(I+1).GT.0) THEN
            XE=XQE_SE(I)/XQE_S(I)
            XSIGE=SQRT(MAX(XQE_SEE(I)/XQE_S(I)-XE**2,0.))
            XE1=XQE_SE(I+1)/XQE_S(I+1)
            DDE=(XE1-XE)/E_STEP
            SEP=DDE/MAX(2.35*XSIGE,E_STEP)
            XQE_MAXSEP=MAX(XQE_MAXSEP,SEP)
         ENDIF
      ENDDO

      END




