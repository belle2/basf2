C
C     KK interface
C
CCCCCC
      subroutine KK_init (DatX_d,DatX_u,DatX_p,IRAND1,DatX_o)
      implicit         none
      character*132 DatX_d, DatX_u, DatX_p, DatX_o

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      integer iflong, IRAND1
      integer ijetset
      common /KK_pythia/ ijetset
      
      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar
      double precision dum(4)

      integer m_Read_PythiaFile
      common /Read_PythiaFile/ m_Read_PythiaFile
      
C   Input environment
      open(16,FILE=DatX_o,STATUS='new',ACTION='write')

      Print *,'KK DATAX_0 : '//DatX_d
      Print *,'KK DATAX_1 : '//DatX_u
      Print *,'KK DATAX_P : '//DatX_p

      open(1,FILE=DatX_u,STATUS='old',ACTION='read')

C   Input beam momentum for boost
C      read(1,*) Pb1(1), Pb1(2), Pb1(3), Pb1(4)
C      read(1,*) Pb2(1), Pb2(2), Pb2(3), Pb2(4)
C      Print *,'KK P(e+)= ',Pb1
C      Print *,'KK P(e-)= ',Pb2
C###### Dummy (for backward compatibility)
      read(1,*,err=99) ctau, dum(2), dum(3), dum(4)
      read(1,*) dum(1), dum(2), dum(3), dum(4)
      Print *,'KK ignored the input of beam momentum from DATAX_1'

C   Input c*tau
      read(1,*) ctau
 99   Print *,'KK ctau= ',ctau

C   Input flag for long lived particle (1:no decay, 0:decay)
      read(1,*) iflong
      Print *,'KK decay switch for long lived particle =',iflong

C   Input flag for long lived particle (1:no decay, 0:decay)
      read(1,*) ijetset
      Print *,'KK switch for using KKMC-JETSET/PYTHIA6 =',ijetset

      close(1)  ! end of input-file open

      Print *,'KK P(e+)= ',Pb1
      Print *,'KK P(e-)= ',Pb2

C   Random seed init for init
      Print *,'KK init random seed for init by ',IRAND1
      CALL PseuMar_Initialize(IRAND1,0,0)
      call kkmc_rand_init(IRAND1+1)

C   KK initialize
      CALL KK2f_ReaDataX(DatX_d, 1,imax,xpar)
      CALL KK2f_ReaDataX(DatX_u, 0,imax,xpar)
C      xpar(1)=sqrt((Pb1(4)+Pb2(4))**2-(Pb1(1)+Pb2(1))**2
C     &            -(Pb1(2)+Pb2(2))**2-(Pb1(3)+Pb2(3))**2)
C      Print *,'KK CMS energy changes to ',xpar(1),' (GeV)'
C      CALL KK2f_Initialize(xpar)                  ! initialize generator

C   Pythia initialize
c      call KK_Pythia_output_table  ! output default table (for tempolary use)
c     switch for long lived particle
      if(iflong.eq.1) then
         call PYGIVE('MSTJ(22)=2!No decay for long lived particle')
      endif
c     input particle data file for pythia
      open(1,FILE=DatX_p,STATUS='old',ACTION='read',ERR=101)
      call PYUPDA(3,1)
      close(1)
      m_Read_PythiaFile=1
      goto 100
 101  Print *,'KK  No input for Pythia'
      m_Read_PythiaFile=0 
 100  continue

C   TAUOLA fifo initialize
      call YesFifo
C     call TAUFIF(-1)  ! open outcome.dat

C      call NoFifo

C      call KK_Boost_flush

      end

      subroutine KK_begin_run(Espread)
      implicit         none
      double precision Espread
      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      integer iflong, IRAND1

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar
      double precision dum(4)

      xpar(1)=sqrt((Pb1(4)+Pb2(4))**2-(Pb1(1)+Pb2(1))**2
     &            -(Pb1(2)+Pb2(2))**2-(Pb1(3)+Pb2(3))**2)
      xpar(2)=Espread
      Print *,'KK CMS energy changes to ',xpar(1),' (GeV)'
      Print *,'KK CMS energy spread changes to ',xpar(2),' (GeV)'
      CALL KK2f_Initialize(xpar)                  ! initialize generator

      call KK_Boost_flush

      end

CCCCCC
      subroutine KK_init_seed
      implicit         none

      integer IRAND1, IRAND2
      real dum(1)
      
C   Get random number seed
      call RANECQ(IRAND1,IRAND2, 1, 'R')
      Print *,'KK random seed: ', IRAND1, IRAND2
C   Initialize random number
      CALL PseuMar_Initialize(IRAND1,0,0)
C      CALL RMARIN(IRAND2,0,0)
      call kkmc_rand_init(IRAND1+1)

      call RANECU(dum,1,1)    ! dummy to increment the seed

      end

CCCCCC
      subroutine KK_term(xSecPb,xErrPb)
      implicit         none

      double precision xSecPb, xErrPb

C      call TAUFIF(2)                              ! TAUOLA fifo
      CALL KK2f_Finalize
      CALL KK2f_GetXsecMC(xSecPb, xErrPb)         ! get MC x-section

      close(16)

      end

CCCCCC
      subroutine KK_event ( status )
      implicit         none
      integer*4 status
c
c  HepEvt
      INCLUDE 'KK2f/HepEvt.fi'
C      integer nmxhep   ! maximum number of particles
C      parameter (nmxhep=4000)
C      double precision phep, vhep
C      integer nevhep, nhep, isthep, idhep, jmohep, jdahep
C      common /HepEvt/
C     &     nevhep,           ! serial number
C     &     nhep,             ! number of particles
C     &     isthep(nmxhep),   ! status code
C     &     idhep(nmxhep),    ! particle ident KF
C     &     jmohep(2,nmxhep), ! parent particles
C     &     jdahep(2,nmxhep), ! children particles
C     &     phep(5,nmxhep),   ! four-momentum, mass [GeV]
C     &     vhep(4,nmxhep)    ! vertex [mm]
C      save /hepevt/
c
     
c
      double precision p(4)
      integer i

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar
      integer ijetset
      common /KK_pythia/ ijetset

      integer m_Read_PythiaFile
      common /Read_PythiaFile/ m_Read_PythiaFile

C   TAUOLA fifo clear
      call TAUFIF(0)

C   Select decay channel
      IF (m_Read_PythiaFile.EQ.1) THEN 
         call KK_select_decay_channel(-15) !tau+
         call KK_select_decay_channel(15) !tau-
      ENDIF
C   Make single event by KK2f
      CALL KK2f_Make
C   Switch on internal JETSET? (this is the default of Belle)
      IF(ijetset.eq.1) THEN
	call PYEXEC
      ENDIF
      
      call PYHEPC(1)       ! Pythia -> Hepevt
      
c  test print
C      CALL PYgive('MSTU(11)=16')
C      CALL PYlist(1)
C      CALL PYgive('MSTU(11)=6')
C      CALL PYlist(1)
c
c      Print *,'KK  nhep= ',nhep

C   boost from CM of beam to lab frame of Belle
      do i=1,nhep
         p(1)=phep(1,i)
         p(2)=phep(2,i)
         p(3)=phep(3,i)
         p(4)=phep(4,i)
         call KK_Boost_CM2lab(p)
         phep(1,i)=p(1)
         phep(2,i)=p(2)
         phep(3,i)=p(3)
         phep(4,i)=p(4)
      enddo

C   Shift vertex point due to tau life time
      call KK_ShiftTauDecayVtx

C   HepEvt to panther
C      CALL KK_HEPEVT_BBS    
C   TAUOLA fifo, KKMC tables to panther
C      call KK_KKMC_BBS

      end

CCCCCC
      subroutine KK_Boost_flush
      implicit none

C  input: Pb1(4), Pb2(4)
C  output: BoostV(3), cosA, sinA

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar
      double precision Pb4, PbKK(4), dtemp

C   Make boost vector and rotation angle
      Pb4=Pb1(4)+Pb2(4)
      BoostV(1)=(Pb1(1)+Pb2(1))/Pb4
      BoostV(2)=(Pb1(2)+Pb2(2))/Pb4
      BoostV(3)=(Pb1(3)+Pb2(3))/Pb4

C      Alpha=0
      cosA=1.
      sinA=0.
      PbKK(1)=Pb2(1)
      PbKK(2)=Pb2(2)
      PbKK(3)=Pb2(3)
      PbKK(4)=Pb2(4)
C    lab to CM
      BoostV(1)=-BoostV(1)
      BoostV(2)=-BoostV(2)
      BoostV(3)=-BoostV(3)
      call KK_Boost_CM2lab(PbKK)
      BoostV(1)=-BoostV(1)
      BoostV(2)=-BoostV(2)
      BoostV(3)=-BoostV(3)
C      Print *,'KK P(e-,CM)= ',PbKK
C      Alpha=datan2(PbKK(1),PbKK(3))
      dtemp=sqrt(PbKK(1)*PbKK(1)+PbKK(3)*PbKK(3))
      cosA=PbKK(3)/dtemp
      sinA=PbKK(1)/dtemp
C      Print *,'Alpha= ',Alpha
Cc    check
C      PbKK(1)=0
C      PbKK(2)=0
C      PbKK(3)=-5.29
C      PbKK(4)=5.29
C      Print *,'KK P(e+,CM)= ',PbKK
C      call KK_Boost_CM2lab(PbKK)
C      Print *,'KK P(e+)= ',PbKK
C      PbKK(1)=0
C      PbKK(2)=0
C      PbKK(3)=5.29
C      PbKK(4)=5.29
C      Print *,'KK P(e-,CM)= ',PbKK
C      call KK_Boost_CM2lab(PbKK)
C      Print *,'KK P(e-)= ',PbKK

      end

CCCCCC
      subroutine KK_Boost_CM2lab ( p )
      implicit none
      double precision p(4)

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar

      double precision b2, gamma, gamma2, bp, tpz, dca, dsa


C  rotate
      tpz=p(3)
C      dca = dcos(Alpha)
C      dsa = dsin(Alpha)
C      p(3) = tpz*dca - p(1)*dsa
C      p(1) = tpz*dsa + p(1)*dca
      p(3) = tpz*cosA - p(1)*sinA
      p(1) = tpz*sinA + p(1)*cosA

C  boost
      b2 = BoostV(1)*BoostV(1)+BoostV(2)*BoostV(2)+BoostV(3)*BoostV(3)
      gamma = 1.0 / dsqrt(1.0 - b2)
      bp = BoostV(1)*p(1)+BoostV(2)*p(2)+BoostV(3)*p(3)
      gamma2 = (gamma - 1.0)/b2

      p(1)=p(1)+ gamma2*bp*BoostV(1) + gamma*BoostV(1)*p(4)
      p(2)=p(2)+ gamma2*bp*BoostV(2) + gamma*BoostV(2)*p(4)
      p(3)=p(3)+ gamma2*bp*BoostV(3) + gamma*BoostV(3)*p(4)
      p(4)=gamma*(p(4)+bp)

      end

CCCCCC
c      subroutine KK_HEPEVT_BBS
c      implicit none
cc
cc  HepEvt
c      INCLUDE 'KK2f/HepEvt.fi'
c#include "panther/panther.inc"
c#include HEPEVT_INC
cc
cc BBS table
cc
cc
cc Var
cc
c      Integer * 4      I, J
c      BBSOffset        I4, I8
cc
cc Main
cc
cc --- clear the table
cc      Call BsDelEnt( Gen_HEPEVT, BBS_ID_ALL )
c
cc --- loop over the particle
cc      Do I = 1, NHEP
cc    --- a new table
cc        Call BsNewEnt(Gen_HEPEVT, I4, I8)
cc
ccc    --- status, particle code, mother
cc        Gen_HEPEVT_isthep(I4)  = ISTHEP(I)
cc        Gen_HEPEVT_idhep(I4)   = IDHEP(I)
cc        Gen_HEPEVT_mother(I4)  = JMOHEP(1,I)
cc
cc    --- mother and daughter
cc        Do J = 1, 2
cc          Gen_HEPEVT_mo(I4, J) = JMOHEP(J, I)
cc          Gen_HEPEVT_da(I4, J) = JDAHEP(J, I)
cc        Enddo
c
cc c       if(JDAHEP(1,I).eq.0) then
cc           Gen_HEPEVT_da(I4, 2) = -1
cc        endif
c
cc    --- four momentum and mass
cc        Do J = 1, 5
cc          Gen_HEPEVT_P(I4, J) = PHEP(J, I)
cc        Enddo
c
cc    --- decay vertex and flight length
cc        Do J = 1, 4
cc          Gen_HEPEVT_V(I4, J) = VHEP(J, I)
cc        Enddo
cc      Enddo
c      Return
c      End

CCCCCC
      subroutine KK_GetBeam ( PXH, PYH, PZH, EH, PXL, PYL, PZL, EL )
      implicit none

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar

      double precision PXH, PYH, PZH, EH, PXL, PYL, PZL, EL

      PXH = Pb2(1)
      PYH = Pb2(2)
      PZH = Pb2(3)
      EH  = Pb2(4)
      PXL = Pb1(1)
      PYL = Pb1(2)
      PZL = Pb1(3)
      EL  = Pb1(4)

      Return
      End

CCCCCC
      subroutine KK_PutBeam ( PXH, PYH, PZH, EH, PXL, PYL, PZL, EL )
      implicit none

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar

      double precision PXH, PYH, PZH, EH, PXL, PYL, PZL, EL

      Pb2(1) = PXH
      Pb2(2) = PYH
      Pb2(3) = PZH
      Pb2(4) = EH
      Pb1(1) = PXL
      Pb1(2) = PYL
      Pb1(3) = PZL
      Pb1(4) = EL

      Return
      End

CCCCCC
      subroutine KK_ShiftTauDecayVtx
      implicit none

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      integer itau,i,j
      double precision p, betagamma, flen, rvec(1), vtx(4), vtx0(4)

      double precision Pb1, Pb2, BoostV, cosA, sinA, ctau
      common /KK_cntl/ Pb1(4), Pb2(4), BoostV(3), cosA, sinA, ctau, xpar
c  HepEvt
      INCLUDE 'KK2f/HepEvt.fi'
      
      do itau=1,2
         do i=1,nhep
            if(idhep(i).eq.(itau*30-45)) then
               p = dsqrt(phep(4,i)*phep(4,i)-phep(5,i)*phep(5,i))
               betagamma = p/phep(5,i)
               call KK_RandExp(rvec,1)
               flen = betagamma*ctau*rvec(1)
               do j=1,4
                  vtx0(j) = VHEP(j,jdahep(1,i))-VHEP(j,i)
                  vtx(j) = flen*phep(j,i)/p - vtx0(j)
               enddo
c               write(17,'(9(1x,f13.6))') 
c     +              flen, (phep(j,i),j=1,5), (vtx0(j),j=1,4)
               call KK_Shift_Daughters_Vtx(i,vtx)
            endif
         enddo
      enddo

      return
      end

CCCCCC
      subroutine KK_Shift_Daughters_Vtx (id, vtx)
      implicit none
      integer id
      double precision vtx(4)
c  HepEvt
      INCLUDE 'KK2f/HepEvt.fi'

      integer i(nmxhep)
      
      if((jdahep(1,id).eq.0).or.(jdahep(2,id).eq.0)) return
      
      i(id)=jdahep(1,id)                   ! do i=jdahep(1,id),jdahep(2,id)
 100  continue                             ! for recursive treatment
      call KK_Shift_Vtx(i(id),vtx)
      call KK_Shift_Daughters_Vtx2(i(id),vtx)
      i(id)=i(id)+1
      if(i(id).le.jdahep(2,id)) goto 100

      return
      end
CCCCCC
      subroutine KK_Shift_Daughters_Vtx2(id, vtx)
      implicit none
      integer id
      double precision vtx(4)

      call KK_Shift_Daughters_Vtx(id,vtx)

      return
      end

CCCCCC
      subroutine KK_Shift_Vtx (id, vtx)
      implicit none
      integer id,i
      double precision vtx(4)
c  HepEvt
      INCLUDE 'KK2f/HepEvt.fi'

      do i=1,4
         VHEP(i,id) = VHEP(i,id)+vtx(i)
      enddo

      return
      end

CCCCCC
      subroutine KK_RandExp (rvec, len)
      implicit none
      double precision rvec
      integer len
      dimension rvec(*)
      
      real dum(1)
      integer i

      do i=1,len
 100     call RANECU(dum,1,1)
         if(dum(1).eq.0) goto 100
         rvec(i)=-dlog(dble(dum(1)))
      enddo

      return
      end

CCCCCC
      subroutine KK_Pythia_output_table
      implicit none

c     output default pythia parameter table
      open(1,FILE="pythia_table_default.dat")
      call PYGIVE('MSTU(11)=1')
      call PYLOGO
      call PYGIVE('MSTU(11)=6')
      write(1,*)
     &'                              Particle/parton data table'
      write(1,*)
     &'       KC  particle          antiparticle    chg col'//
     &' anti   mass       width       w-cut     lifetime decay'
      write(1,*)
     &'        on/off  ME     Br.rat.      decay products'
      write(1,*)'------------ Above should be deleted. --------------'
      call PYUPDA(1,1)
      close(1)

      return
      end

CCCCCC
      subroutine KK_select_decay_channel (id)
      implicit none
c     id = 15 for tau- , -15 for tau+
      integer id

c     tauola
      integer JAK1,JAK2,JAKP,JAKM,KTOM
      COMMON / JAKI / JAK1,JAK2,JAKP,JAKM,KTOM

c     pythia
      integer MDCY,MDME,KFDP
      double precision BRAT
      COMMON/PYDAT3/MDCY(500,3),MDME(8000,2),BRAT(8000),KFDP(8000,5)
      integer PYCOMP               ! function

      integer KC,KFA,KFS,KFSP,KFSN
      integer NOPE,IDL,IDC
      double precision BRSU,RBR

      real dum(1)
      integer JAK

      KFA=IABS(id)
      if(KFA.ne.15) then
         return
      endif
      KFS=-ISIGN(1,id)  !!! pythia ID is different from PDG
      KC=PYCOMP(KFA)

c     select decay channel using pythia decay table and method

C...Check existence of decay channels. Particle/antiparticle rules.
      IF(MDCY(KC,2).LE.0.OR.MDCY(KC,3).LE.0) THEN
        CALL PYERRM(9,'(PYDECY:) no decay channel defined')
        RETURN
      ENDIF
      IF(KFS.GT.0) THEN
        KFSP=1
        KFSN=0
      ELSE
        KFSP=0
        KFSN=1
      ENDIF
 
C...Sum branching ratios of allowed decay channels.
  220 NOPE=0
      BRSU=0D0
      DO 230 IDL=MDCY(KC,2),MDCY(KC,2)+MDCY(KC,3)-1
        IF(MDME(IDL,1).NE.1.AND.KFSP*MDME(IDL,1).NE.2.AND.
     &  KFSN*MDME(IDL,1).NE.3) GOTO 230
        IF(MDME(IDL,2).GT.100.and.MDME(IDL,2).LT.2100) GOTO 230
        NOPE=NOPE+1
        BRSU=BRSU+BRAT(IDL)
  230 CONTINUE
      IF(NOPE.EQ.0) THEN
        CALL PYERRM(2,'(PYDECY:) all decay channels closed by user')
        RETURN
      ENDIF
 
C...Select decay channel among allowed ones.
  240 call RANECU(dum,1,1)


      RBR=BRSU*dum(1)
      IDL=MDCY(KC,2)-1
  250 IDL=IDL+1
      IF(MDME(IDL,1).NE.1.AND.KFSP*MDME(IDL,1).NE.2.AND.
     &KFSN*MDME(IDL,1).NE.3) THEN
        IF(IDL.LT.MDCY(KC,2)+MDCY(KC,3)-1) GOTO 250
      ELSEIF(MDME(IDL,2).GT.100.and.MDME(IDL,2).LT.2100) THEN
        IF(IDL.LT.MDCY(KC,2)+MDCY(KC,3)-1) GOTO 250
      ELSE
        IDC=IDL
        RBR=RBR-BRAT(IDL)
        IF(IDL.LT.MDCY(KC,2)+MDCY(KC,3)-1.AND.RBR.GT.0D0) GOTO 250
      ENDIF

c     IDC is selected decay channel.
      
      JAK = -1
      if(MDME(IDC,2).GT.2100.and.MDME(IDC,2).LE.2199) then
c     use TAUOLA
         JAK = MDME(IDC,2)-2100
c      else
c     use PYTHIA  JAK=-1
      endif

c     store for TAUOLA
      if(id.eq.15) then
         JAK2 = JAK
C         PRINT *,'KK_select_decay_channel (tau-) jak2 = ',jak2,' IDC = ',IDC, MDME(IDC,2)
      else
         JAK1 = JAK
C         PRINT *,'KK_select_decay_channel (tau+) jak1 = ',jak1,' IDC = ',IDC, MDME(IDC,2)
      endif
      return
      end
CCCCCC
c      subroutine KK_KKMC_BBS
c      implicit none
c
cc TAUOLA fifo common block
c      Integer NMWT2
c      PARAMETER (NMWT2=44)
c      COMMON /CHERE/  WTCHE,KK,iffifo  ! need separate space for tau+tau-
c      REAL*8          WTCHE(NMWT2)
c      Integer  KK
c      LOGICAL  iffifo
c      COMMON /OLAoutA/  MNUMA,IDA,JAK1A,JAK2A,JAKPA,JAKMA,KTOMA,AMPLITA,
c     $                  HVA,PTA,PNA,PIM1A,PIM2A,PIPLA,PIM3A,PIM4A,DGAMTA
c      REAL  PTA(4),PNA(4),PIM1A(4),PIM2A(4),PIPLA(4),HVA(4),PIM3A(4),PIM4A(4)
c      Integer MNUMA,IDA,JAK1A,JAK2A,JAKPA,JAKMA,KTOMA
c      Real  AMPLITA,DGAMTA
c      COMMON /OLAoutB/  MNUMB,IDB,JAK1B,JAK2B,JAKPB,JAKMB,KTOMB,AMPLITB,
c     $                  HVB,PTB,PNB,PIM1B,PIM2B,PIPLB,PIM3B,PIM4B,DGAMTB
c      REAL  PTB(4),PNB(4),PIM1B(4),PIM2B(4),PIPLB(4),HVB(4),PIM3B(4),PIM4B(4)
c      Integer MNUMB,IDB,JAK1B,JAK2B,JAKPB,JAKMB,KTOMB
c      Real  AMPLITB,DGAMTB
c
cc BBS table
c#include "panther/panther.inc"
c#include KKMC_INC
c
cc Var
c      Integer * 4      I, J
c      BBSOffset        I4, I8
c
cc      DOUBLE PRECISION WtMain,WtCrud
c      Integer NISR,NFSR
c
c      INCLUDE 'KK2f/KK2f.fi'
cc
cc Main
cc
c      IF(.not.iffifo) return
c
cc  count number of ISR/FSR photons
cc      call KK2f_GetNphot(Nphot)
cc      call KK2f_GetIsr(isr)
c      NISR = 0
c      NFSR = 0
c      Do I = 1, m_nphot
c         if(m_isr(I) .eq. 1) then
c            NISR = NISR + 1
c         else
c            NFSR = NFSR + 1
c         endif
c      Enddo
c
cc --- clear the table
cc      Call BsDelEnt(Gen_KKMC_Info, BBS_ID_ALL)
cc --- new table
cc      Call BsNewEnt(Gen_KKMC_Info, I4, I8)
c
cc --- KK2f
cc      call KK2f_GetWt(WtMain,WtCrud)
cc      Gen_KKMC_Info_WtMain(I4) = SNGL(m_WtMain)
cc      Gen_KKMC_Info_WtCrud(I4) = SNGL(m_WtCrud)
cc      Gen_KKMC_Info_NISR(I4) = NISR
cc      Gen_KKMC_Info_NFSR(I4) = NFSR
c
cc --- OLAoutA
cc      Gen_KKMC_Info_MNUMA(I4) = MNUMA
cc      Gen_KKMC_Info_IDA(I4) = IDA
cc      Gen_KKMC_Info_JAK1A(I4) = JAK1A
cc      Gen_KKMC_Info_JAK2A(I4) = JAK2A
cc      Gen_KKMC_Info_JAKPA(I4) = JAKPA
cc      Gen_KKMC_Info_JAKMA(I4) = JAKMA
cc      Gen_KKMC_Info_KTOMA(I4) = KTOMA
cc      Gen_KKMC_Info_AMPLITA(I4) = AMPLITA
cc      Gen_KKMC_Info_DGAMTA(I4) = DGAMTA
cc      Do J = 1, 4
cc         Gen_KKMC_Info_HVA(I4, J) = HVA(J)
cc         Gen_KKMC_Info_PTA(I4, J) = PTA(J)
cc         Gen_KKMC_Info_PNA(I4, J) = PNA(J)
cc         Gen_KKMC_Info_PIPLA(I4, J) = PIPLA(J)
cc         Gen_KKMC_Info_PIM1A(I4, J) = PIM1A(J)
cc         Gen_KKMC_Info_PIM2A(I4, J) = PIM2A(J)
cc         Gen_KKMC_Info_PIM3A(I4, J) = PIM3A(J)
cc         Gen_KKMC_Info_PIM4A(I4, J) = PIM4A(J)
cc      Enddo
c
cc --- OLAoutB
cc      Gen_KKMC_Info_MNUMB(I4) = MNUMB
cc      Gen_KKMC_Info_IDB(I4) = IDB
cc      Gen_KKMC_Info_JAK1B(I4) = JAK1B
cc      Gen_KKMC_Info_JAK2B(I4) = JAK2B
cc      Gen_KKMC_Info_JAKPB(I4) = JAKPB
cc      Gen_KKMC_Info_JAKMB(I4) = JAKMB
cc      Gen_KKMC_Info_KTOMB(I4) = KTOMB
cc      Gen_KKMC_Info_AMPLITB(I4) = AMPLITB
cc      Gen_KKMC_Info_DGAMTB(I4) = DGAMTB
cc      Do J = 1, 4
cc         Gen_KKMC_Info_HVB(I4, J) = HVB(J)
cc         Gen_KKMC_Info_PTB(I4, J) = PTB(J)
cc         Gen_KKMC_Info_PNB(I4, J) = PNB(J)
cc         Gen_KKMC_Info_PIPLB(I4, J) = PIPLB(J)
cc         Gen_KKMC_Info_PIM1B(I4, J) = PIM1B(J)
cc         Gen_KKMC_Info_PIM2B(I4, J) = PIM2B(J)
cc         Gen_KKMC_Info_PIM3B(I4, J) = PIM3B(J)
cc         Gen_KKMC_Info_PIM4B(I4, J) = PIM4B(J)
cc      Enddo
c
c      Return
c      End
