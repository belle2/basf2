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
      
      double precision ctau
      common /KK_cntl/ ctau, xpar
      double precision dum(4)

      integer m_Read_PythiaFile
      common /Read_PythiaFile/ m_Read_PythiaFile
      
C   Input environment
      open(16,FILE=DatX_o,STATUS='new',ACTION='write')

      Print *,'KK DATAX_0 : '//DatX_d
      Print *,'KK DATAX_1 : '//DatX_u
      Print *,'KK DATAX_P : '//DatX_p

      open(1,FILE=DatX_u,STATUS='old',ACTION='read')

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

C   Random seed init for init
      Print *,'KK init random seed for init by ',IRAND1
      CALL PseuMar_Initialize(IRAND1,0,0)
      call kkmc_rand_init(IRAND1+1)

C   KK initialize
      CALL KK2f_ReaDataX(DatX_d, 1,imax,xpar)
      CALL KK2f_ReaDataX(DatX_u, 0,imax,xpar)

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


CCCCCC
      subroutine KK_begin_run(Ecms, Espread)
      implicit         none
      double precision Ecms, Espread
      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      double precision ctau
      common /KK_cntl/ ctau, xpar

      xpar(1)= Ecms
      xpar(2)= Espread
      Print *,'KK CMS energy changes to ',xpar(1),' (GeV)'
      Print *,'KK CMS energy spread changes to ',xpar(2),' (GeV)'
      CALL KK2f_Initialize(xpar)                  ! initialize generator


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

      end



CCCCCC
      subroutine KK_ShiftTauDecayVtx
      implicit none

      integer imax
      parameter (imax = 10000)
      double precision xpar(imax)

      integer itau,i,j
      double precision p, betagamma, flen, rvec(1), vtx(4), vtx0(4)

      double precision ctau
      common /KK_cntl/ ctau, xpar
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

