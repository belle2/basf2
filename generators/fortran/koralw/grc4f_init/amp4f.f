
      SUBROUTINE AMPINI(XPAR,NPAR)
! ********************************
! Initialization of the external 4fermion matrix el. codes
! XPAR(100),NPAR(100): input parameter matrices of KORALW as explained in
!                      the manual
! For the moment only GRACE is interfaced
! ********************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      DIMENSION  XPAR (100),NPAR (100) 
      DIMENSION AMAFIN(20) 

! parameters for 4-fermion amplitudes
      COMMON /AM4PAR/ INPAR(100),YXPAR(100),YAMAFIN(20),YSIN2W,YGPICOB

! additional information can be obtained with the help of routine
      CALL MASOW(SIN2W,GPICOB,AMAFIN)
!     where sin2w is a sine of weinberg angle 
!     and   AMAFIN contained masses of W decay products
!     all as printed by the KORALW
!

! filling /AM4PAR/
      do ii=1,100
        inpar(ii) = npar(ii)
        yxpar(ii) = xpar(ii)
      enddo
      do ii=1,20
        yamafin(ii) = amafin(ii)
      enddo
      ygpicob =gpicob
      ysin2w = sin2w
! end  filling /AM4PAR/

      WRITE(6,'(10X,A)')
     $         '*******************************************************'
      WRITE(6,'(10X,A)')
     $         '****** AMPINI: external library initialization ********'
      WRITE(6,'(10X,A)')
     $         '*******************************************************'
      WRITE(6,*) ' '

! here users code should come

      WRITE(6,'(10X,A)') 
     $         '*******************************************************'
      WRITE(6,'(10X,A)') 
     $         '************* AMPINI -- GRACE activation **************'
      WRITE(6,'(10X,A/)') 
     $         '*******************************************************'
      WRITE(*,'(20X,A/)') 'GRACE   Ver.   2. 0'
      WRITE(*,'(20X,A/)') 'date of interfacing: 28-05-96'
      WRITE(*,'(10X,A/)')
     .'(c)Copyright 1990-1996 Minami-Tateya Group (KEK, Japan)     '
      WRITE(6,'(10X,A)') 
     $         '*******************************************************'

!      KeyPhy = NPAR(2)
!      KeyZet = MOD(KeyPhy,1000)/100
!      KeyWu  = MOD(KeyPhy,1000000)/100000

!      if( keyzet.ne.1 ) then
!        write(6,*)'AMPINI==> Sorry, not implemented: KeyZet =',keyzet
!        stop
!      endif
!      if( keywu.ne.1 ) then
!        write(6,*)'AMPINI==> Sorry, not implemented: KeyWu =',keywu
!        stop
!      endif
      call gr_init(1,XPAR,NPAR,SIN2W,GPICOB,AMAFIN)
         
      END



! *****************************************************
      SUBROUTINE AMP4F( Q1,IFLBM1, Q2,IFLBM2
     $      ,P1,IFLAV1, P2,IFLAV2, P3,IFLAV3, P4,IFLAV4
     $      , WTMOD4F,WT4F )
! *****************************************************
! external 4-fermion matrix elements calculations
! INPUTS
!   q1,q2              - beam 4momenta (e-, e+)
!   iflbm1,iflbm2      - beam IDs
!   p1..p4             - final state fermions 4momenta
!                        p1:    fermion of W-
!                        p2:antifermion of W-
!                        p3:    fermion of W+
!                        p4:antifermion of W+
!   iflav1..iflav4     - final state fermions IDs
!   WARNING: for the moment iflav-s are dummy, and 4momenta must be
!            ordered as described above
! OUTPUTS
!   wtmod4f        - principal weight for rejection
!   wt4f(9)        - auxiliary weights wector
!      wtmod4f = elmatr_4fermions
! *****************************************************
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  

! parameters for 4fermion amplitudes
      COMMON /AM4PAR/ NPAR(100),XPAR(100),AMAFIN(20),SIN2W,GPICOB
!      common /testownis/ pp
*     PARAMETER ( MXDIM = 50 )
*     DOUBLE PRECISION  X(MXDIM) 
      DIMENSION Q1(4),Q2(4),P1(4),P2(4),P3(4),P4(4)
      DIMENSION RQ1(4),RQ2(4),RP1(4),RP2(4),RP3(4),RP4(4)
      DIMENSION PE(4,6),PP(6,6)
      DIMENSION WT4F(9)
      DIMENSION II(4),JJ(4),KK(2)
      save
      save suma
      data suma / 0d0 / 


! dipswitch ISWITCH if set to 0 activates just CC03 of GRACE.
! it is for consistency tests. 
      ISWITCH=1

! anomalous couplings are suppressed in  fake 4fermion matr. el.
      keyacc_lcl=0

      DO I4F=1,9
        WT4F(I4F) = 0D0
      ENDDO

! -- NOW OFF DIAGONAL NON-GRACE CONTRIBUTIONS
     
      if     (iflav1.eq.1.and.iflav2.ne.-2
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      elseif (iflav1.eq.3.and.iflav2.ne.-4
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      elseif (iflav1.eq.5
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      elseif (iflav4.eq.-1.and.iflav3.ne.2
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      elseif (iflav4.eq.-3.and.iflav3.ne.4
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      elseif (iflav4.eq.-5
     $   .and.iflav1.ne.-iflav2.and.iflav3.ne.-iflav4) then
!--   ======
       wtmod4f =wwborn(p1,p2,p3,p4,keyacc_lcl)
      else
!--   ======
! -- NOW DIAGONAL GRACE MODES
! -- WE START WITH::
! definition of  translation of the final states indices as in KORALW 
! into Grace process numbers (idef). iiii denotes relative order 
! as momenta p1,p2,p3,p4 should enter into Grace calculations.
! KB1 defines if cp transformation hast to be used for particular
! final state channel.
      IF(ISWITCH.eq.0) then 
!this is udmn
c       idef = 27
c       iiii =1234
c       KB1  =1   
! switching to sccs  ms. 3/4/98, tests
! do not forget to adjust normalisation (1/3) at the very end
       idef = 62
       iiii =3412
       KB1  =1
      
      ELSEIF(iflav1.eq.-iflav2.and.iflav3.eq.-iflav4
     $       .and.iflav1*iflav4.lt.0) THEN
! new zz-channels
       IF     (iflav1.eq.11.and.iflav3.eq.11) then
        iiii =1234
        KB1  =1
        idef=7
       ELSEIF (iflav1.eq.11.and.iflav3.eq.13) then
        iiii =1234
        KB1  =1
        idef=8
       ELSEIF (iflav1.eq.13.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=8
       ELSEIF (iflav1.eq.11.and.iflav3.eq.15) then
        iiii =1234
        KB1  =1
        idef=9
       ELSEIF (iflav1.eq.15.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=9
       ELSEIF (iflav1.eq.13.and.iflav3.eq.13) then
        iiii =1234
        KB1  =1
        idef=10
       ELSEIF (iflav1.eq.15.and.iflav3.eq.15) then
        iiii =1234
        KB1  =1
        idef=11
       ELSEIF (iflav1.eq.13.and.iflav3.eq.15) then
        iiii =1234
        KB1  =1
        idef=12
       ELSEIF (iflav1.eq.15.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=12
       ELSEIF (iflav1.eq.11.and.iflav3.eq.14) then
        iiii =1234
        KB1  =1
        idef=13
       ELSEIF (iflav1.eq.14.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=13
       ELSEIF (iflav1.eq.11.and.iflav3.eq.16) then
        iiii =1234
        KB1  =1
        idef=14
       ELSEIF (iflav1.eq.16.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=14
       ELSEIF (iflav1.eq.12.and.iflav3.eq.13) then
        iiii =1234
        KB1  =1
        idef=15
       ELSEIF (iflav1.eq.13.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=15
       ELSEIF (iflav1.eq.12.and.iflav3.eq.15) then
        iiii =1234
        KB1  =1
        idef=16
       ELSEIF (iflav1.eq.15.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=16
       ELSEIF (iflav1.eq.16.and.iflav3.eq.13) then
        iiii =1234
        KB1  =1
        idef=17
       ELSEIF (iflav1.eq.13.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=17
       ELSEIF (iflav1.eq.14.and.iflav3.eq.15) then
        iiii =1234
        KB1  =1
        idef=18
       ELSEIF (iflav1.eq.15.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=18
       ELSEIF (iflav1.eq.12.and.iflav3.eq.12) then
        iiii =1234
        KB1  =1
        idef=19
       ELSEIF (iflav1.eq.12.and.iflav3.eq.14) then
        iiii =1234 
        KB1  =1
        idef=20
       ELSEIF (iflav1.eq.14.and.iflav3.eq.12) then
        iiii =3412  
        KB1  =1
        idef=20
       ELSEIF (iflav1.eq.12.and.iflav3.eq.16) then
        iiii =1234  
        KB1  =1
        idef=21
       ELSEIF (iflav1.eq.16.and.iflav3.eq.12) then
        iiii =3412  
        KB1  =1
        idef=21

       ELSEIF (iflav1.eq.14.and.iflav3.eq.14) then
        iiii =1234
        KB1  =1
        idef=22
       ELSEIF (iflav1.eq.16.and.iflav3.eq.16) then
        iiii =1234
        KB1  =1
        idef=23
       ELSEIF (iflav1.eq.14.and.iflav3.eq.16) then
        iiii =1234  ! is it OK ?????
        KB1  =1
        idef=24
       ELSEIF (iflav1.eq.16.and.iflav3.eq.14) then
        iiii =3412  ! is it OK ?????
        KB1  =1
        idef=24
! ...   end table 1 
! ...   e - quarks
       ELSEIF (iflav1.eq.11.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=31
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=31
       ELSEIF (iflav1.eq.11.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=32
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=32
       ELSEIF (iflav1.eq.11.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=33
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=33
       ELSEIF (iflav1.eq.11.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=34
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=34
       ELSEIF (iflav1.eq.11.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=35
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.11) then
        iiii =3412
        KB1  =1
        idef=35
! ...   mu - quarks messed
       ELSEIF (iflav1.eq.13.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=36
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=36
       ELSEIF (iflav1.eq.13.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=37
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=37
       ELSEIF (iflav1.eq.15.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=38
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.15) then
        iiii =3412
        KB1  =1
        idef=38
       ELSEIF (iflav1.eq.15.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=39
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.15) then
        iiii =3412
        KB1  =1
        idef=39
       ELSEIF (iflav1.eq.13.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=40
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=40
! ...   mu-tau - quarks
       ELSEIF (iflav1.eq.13.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=41
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=41
       ELSEIF (iflav1.eq.13.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=42
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.13) then
        iiii =3412
        KB1  =1
        idef=42
       ELSEIF (iflav1.eq.15.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=43
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.15) then
        iiii =3412
        KB1  =1
        idef=43
       ELSEIF (iflav1.eq.15.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=44
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.15) then
        iiii =3412
        KB1  =1
        idef=44
       ELSEIF (iflav1.eq.15.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=45
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.15) then
        iiii =3412
        KB1  =1
        idef=45
!--
! ...   nu e - quarks
       ELSEIF (iflav1.eq.12.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=31+15
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=31+15
       ELSEIF (iflav1.eq.12.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=32+15
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=32+15
       ELSEIF (iflav1.eq.12.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=33+15
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=33+15
       ELSEIF (iflav1.eq.12.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=34+15
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=34+15
       ELSEIF (iflav1.eq.12.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=35+15
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.12) then
        iiii =3412
        KB1  =1
        idef=35+15
! ...   nu mu - quarks messed
       ELSEIF (iflav1.eq.14.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=36+15
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=36+15
       ELSEIF (iflav1.eq.14.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=37+15
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=37+15
       ELSEIF (iflav1.eq.16.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=38+15
       ELSEIF (iflav1.eq. 2.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=38+15
       ELSEIF (iflav1.eq.16.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=39+15
       ELSEIF (iflav1.eq. 4.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=39+15
       ELSEIF (iflav1.eq.14.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=40+15
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=40+15
! ...   nu mutau - quarks
       ELSEIF (iflav1.eq.14.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=41+15
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=41+15
       ELSEIF (iflav1.eq.14.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=42+15
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.14) then
        iiii =3412
        KB1  =1
        idef=42+15
       ELSEIF (iflav1.eq.16.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=43+15
       ELSEIF (iflav1.eq. 1.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=43+15
       ELSEIF (iflav1.eq.16.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=44+15
       ELSEIF (iflav1.eq. 3.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=44+15
       ELSEIF (iflav1.eq.16.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=45+15
       ELSEIF (iflav1.eq. 5.and.iflav3.eq.16) then
        iiii =3412
        KB1  =1
        idef=45+15
! ... hadronic processes ...
       ELSEIF (iflav1.eq. 2.and.iflav3.eq. 2) then
        iiii =1234
        KB1  =1
        idef=64
       ELSEIF (iflav1.eq. 4.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=65
       ELSEIF (iflav1.eq. 1.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=66
       ELSEIF (iflav1.eq. 3.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=67
       ELSEIF (iflav1.eq. 5.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=68
       ELSEIF (iflav1.eq. 2.and.iflav3.eq. 4) then
        iiii =1234
        KB1  =1
        idef=69
       ELSEIF (iflav1.eq. 4.and.iflav3.eq. 2) then
        iiii =3412
        KB1  =1
        idef=69
       ELSEIF (iflav1.eq. 2.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=70
       ELSEIF (iflav1.eq. 3.and.iflav3.eq. 2) then
        iiii =3412
        KB1  =1
        idef=70
       ELSEIF (iflav1.eq. 2.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=71
       ELSEIF (iflav1.eq. 5.and.iflav3.eq. 2) then
        iiii =3412
        KB1  =1
        idef=71
       ELSEIF (iflav1.eq. 4.and.iflav3.eq. 1) then
        iiii =1234
        KB1  =1
        idef=72
       ELSEIF (iflav1.eq. 1.and.iflav3.eq. 4) then
        iiii =3412
        KB1  =1
        idef=72
       ELSEIF (iflav1.eq. 4.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=73
       ELSEIF (iflav1.eq. 5.and.iflav3.eq. 4) then
        iiii =3412
        KB1  =1
        idef=73
       ELSEIF (iflav1.eq. 1.and.iflav3.eq. 3) then
        iiii =1234
        KB1  =1
        idef=74
       ELSEIF (iflav1.eq. 3.and.iflav3.eq. 1) then
        iiii =3412
        KB1  =1
        idef=74
       ELSEIF (iflav1.eq. 1.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=75
       ELSEIF (iflav1.eq. 5.and.iflav3.eq. 1) then
        iiii =3412
        KB1  =1
        idef=75
       ELSEIF (iflav1.eq. 3.and.iflav3.eq. 5) then
        iiii =1234
        KB1  =1
        idef=76
       ELSEIF (iflav1.eq. 5.and.iflav3.eq. 3) then
        iiii =3412
        KB1  =1
        idef=76

       ELSE
        write(*,*)  'AMP4F: not initialized zz mode'
        write(*,*) iflav1,iflav2,iflav3,iflav4
        stop
       ENDIF

      ELSEIF(IFLAV1.eq. 11.and.IFLAV4.eq.-11) THEN
       idef =1
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 11.and.IFLAV4.eq.-13) THEN
       idef =2
       iiii =1243
       KB1  =1
      ELSEIF(IFLAV1.eq. 13.and.IFLAV4.eq.-11) THEN
       idef =2
       iiii =1243
       KB1  =2
      ELSEIF(IFLAV1.eq. 11.and.IFLAV4.eq.-15) THEN
       idef =3
       iiii =1243
       KB1  =1
      ELSEIF(IFLAV1.eq. 15.and.IFLAV4.eq.-11) THEN
       idef =3
       iiii =1243
       KB1  =2
      ELSEIF(IFLAV1.eq. 13.and.IFLAV4.eq.-13) THEN
       idef =4
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 13.and.IFLAV4.eq.-15) THEN
       idef =6
       iiii =1243
       KB1  =1
      ELSEIF(IFLAV1.eq. 15.and.IFLAV4.eq.-13) THEN
       idef =6
       iiii =1243
       KB1  =2
      ELSEIF(IFLAV1.eq. 15.and.IFLAV4.eq.-15) THEN
       idef =5
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 11.and.IFLAV4.eq.-1) THEN
       idef = 25
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  1.and.IFLAV4.eq.-11) THEN
       idef = 25
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 13.and.IFLAV4.eq.-1) THEN
       idef = 27
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  1.and.IFLAV4.eq.-13) THEN
       idef = 27
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 15.and.IFLAV4.eq.-1) THEN
       idef = 29
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  1.and.IFLAV4.eq.-15) THEN
       idef = 29
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 11.and.IFLAV4.eq.-3) THEN
       idef = 26
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  3.and.IFLAV4.eq.-11) THEN
       idef = 26
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 13.and.IFLAV4.eq.-3) THEN
       idef = 28
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  3.and.IFLAV4.eq.-13) THEN
       idef = 28
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 15.and.IFLAV4.eq.-3) THEN
       idef = 30
       iiii =1234
       KB1  =1
      ELSEIF(IFLAV1.eq.  3.and.IFLAV4.eq.-15) THEN
       idef = 30
       iiii =1234
       KB1  =2
      ELSEIF(IFLAV1.eq. 1.and.IFLAV4.eq.-1) THEN
       idef = 61
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 3.and.IFLAV4.eq.-3) THEN
       idef = 62
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 3.and.IFLAV4.eq.-1) THEN
       idef = 63
       iiii =3412
       KB1  =1
      ELSEIF(IFLAV1.eq. 1.and.IFLAV4.eq.-3) THEN
       idef = 63
       iiii =3412
       KB1  =2
      else
       WRITE(*,*) 'amp4f:     I do not know how to handle:'
       WRITE(*,*) 'IFLAV(1-4): ',IFLAV1,IFLAV2,IFLAV3,IFLAV4
       stop
      endif

!--- CP transformation now ...
        IF(KB1.EQ.1) Then
         do k=1,4
          rq1(k)=q1(k)
          rq2(k)=q2(k)
          rp1(k)=p1(k)
          rp2(k)=p2(k)
          rp3(k)=p3(k)
          rp4(k)=p4(k)
         enddo
        else
         do k=1,3
          rq1(k)=-q2(k)
          rq2(k)=-q1(k)
          rp1(k)=-p4(k)
          rp2(k)=-p3(k)
          rp3(k)=-p2(k)
          rp4(k)=-p1(k)
         enddo
          k=4
          rq1(k)=q2(k)
          rq2(k)=q1(k)
          rp1(k)=p4(k)
          rp2(k)=p3(k)
          rp3(k)=p2(k)
          rp4(k)=p1(k)
        endif
c-- translation of order index iiii into algorithm variables
!
        II(1)=    iiii      /1000
        II(2)=mod(iiii,1000)/100 
        II(3)=mod(iiii, 100)/10  
        II(4)=mod(iiii,  10)     
        DO L=1,4
         IL=II(L)
         JJ(IL)=L+2
        ENDDO
!
!        write(*,*) ii(1),ii(2),ii(3),ii(4)
!        write(*,*) jj(1),jj(2),jj(3),jj(4)
!        stop
!
!---  ======================
       DO K=1,4
*           1:  EL-  INITIAL   LPRTCL  MASS=AMEL
         PE( K,    1 ) = rq1( K)
*           2:  EL+  INITIAL   LANTIP  MASS=AMEL
         PE( K,    2 ) = rq2( K)
*           3:  MU-  FINAL     LPRTCL  MASS=AMMU
         PE( K, JJ(1)) = rp1( K)
*           4:  NM   FINAL     LANTIP  MASS=AMNM
         PE( K, JJ(2)) = rp2( K)
*           5:  UQ   FINAL     LPRTCL  MASS=AMUQ
         PE( K, JJ(3)) = rp3( K)
*           6:  DQ+  FINAL     LANTIP  MASS=AMDQ
         PE( K, JJ(4)) = rp4( K)
       ENDDO
!
*     Set table of inner products of momenta.
*     This table should be calculated from invariants.
*     When it is constructed from PE,it will not so accurate.
*
*     PP(I,J) = inner product between PE(*,I) and PE(*,J)
!      iwal=0
      do i=1,6
        do j=1,6
          pp(i,j) = pe(4,i)*pe(4,j) 
     &         -pe(1,i)*pe(1,j) -pe(2,i)*pe(2,j) -pe(3,i)*pe(3,j)
!          if(pp(i,j).lt.1d-7) iwal=1
        enddo
      enddo
!      if(iwal.eq.1) then
!          write(*,*) '=====amp4f here is problem==============='
!          write(*,*) '=====one of inv is too small============='
!       do i=1,6
!         do j=1,6
!           write(*,*) 'i=',i,' j=',j,' pp=',pp(i,j)
!         enddo
!       enddo
!      endif

!!      WTMOD4F = 1d0
!!      if(iflav1.eq.13.and.iflav3.eq.2) then  !(81)
!!      if(iflav1.eq.1.and.iflav3.eq.2) then  !(11)
!!      if(iflav1.eq.1.and.iflav3.eq.14) then  !(18)
!!      if(iflav1.eq.13.and.iflav3.eq.14) then  !(88)
*=======================================================================
*==== full 4-fermion x-section only for CKM dominant channels !

! MS. 19.06.97
!!       CALL gr_rinit(idef,XPAR,NPAR,SIN2W,GPICOB,AMAFIN)
       CALL gr_rinit(idef)
! MS. 19.06.97
       CALL SM_KORALW(ISWITCH,XPAR,NPAR,SIN2W,GPICOB,AMAFIN)
*         ============= coupling constants
           call amparm
*         =============


*      el_4ferm = func(x)
       el_4ferm = func(pe,pp)
 !The following re-normalisation for CC03 is necessary, because for
 !CC03 all flavour configurations use the same matrix element of
 !doubly leptonic type (in Grace).
         IF(ISWITCH.eq.0) then
           if(abs(iflav1).lt.10) then
             wm = 3d0
           else
             wm = 1d0
           endif
           if(abs(iflav3).lt.10) then
             wp = 3d0
           else
             wp = 1d0
           endif 
c           el_4ferm = el_4ferm*wp*wm/3D0  ! uden
           el_4ferm = el_4ferm*wp*wm/9D0  ! cssc
         ENDIF
 ! ... up to here

       WTMOD4F = el_4ferm
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!! in the doubly ckm-suppresses case we add !!!
!!! in quadratures the CC03 contribution     !!!
!!!                          10/7/98 ms      !!!
! first select ZZ-type final states
      IF(iflav1.eq.-iflav2.and.iflav3.eq.-iflav4
     $       .and.iflav1*iflav4.lt.0) THEN
      icwm=iflav1
      icwp=iflav3
! then select MIX non-diag.
      IF(icwm.EQ.1.AND.icwp.EQ.4 .OR. icwm.EQ.4.AND.icwp.EQ.1 .OR.
!         ccdd
     @   icwm.EQ.2.AND.icwp.EQ.3 .OR. icwm.EQ.3.AND.icwp.EQ.2 .OR. 
!         uuss
     @   icwm.EQ.2.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.2 .OR. 
!         uubb
     @   icwm.EQ.4.AND.icwp.EQ.5 .OR. icwm.EQ.5.AND.icwp.EQ.4) THEN
!         ccbb
c bug 2/20/99        wtcc03 =wwborn(p1,p2,p3,p4,keyacc_lcl) 
! the 4momenta 2-4 must be flipped (u~us~s -> u~ss~u)
        wtcc03 =wwborn(p1,p4,p3,p2,keyacc_lcl)

! send to interfaces also the ratio
        wt4f9=wtmod4f/(wtcc03+wtmod4f)

        CALL waga_ckm(-1,wt4f9)  


        wtmod4f=wtmod4f+wtcc03 
      ENDIF
      ENDIF
!!!                                          !!!
!!! in the doubly ckm-suppresses case we add !!!
!!! in quadratures the CC03 contribution     !!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


      endif
!--   ======
!--   end of the main choice menu. 
*=======================================================================

c      WT4F(1) = el_WWpair
c      WT4F(2) = el_4ferm

*=======================================================================

!!!!!!!
c      suma=suma+wtmod4f
c      WRITE(6,*)'suma==',suma
c      IF(el_4ferm.le.0d0) write(6,*)'elmatr zero,wtmod4f',wtmod4f
!!!!!!!
!      write(8,'(a,f18.11)') '88 wt4f=',wtmod4f
       IF(ISWITCH.eq.0) then
         el_WW=wwborn(p1,p2,p3,p4,0)
         WRITE(8,'(a,f18.11,a,f22.16,a,I3,I3,I3,I3)') 
     $   '88 wt4f=',wtmod4f,' ',wtmod4f/el_WW, 
     $   ' IFLAV(1-4): ',IFLAV1,IFLAV2,IFLAV3,IFLAV4
         WRITE(6,'(a,f18.11,a,f22.16,a,I3,I3,I3,I3)') 
     $   '88 wt4f=',wtmod4f,' ',wtmod4f/el_WW, 
     $   ' IFLAV(1-4): ',IFLAV1,IFLAV2,IFLAV3,IFLAV4
       ENDIF

*      write(6,*) '4ferm=',el_WWpair,el_4ferm
*      write(6,*) 'wtmod4ferm=',wtmod4f

      return
      END

* File spdetc.f generated by GRACE Ver. 2.00(35)        1996/03/24/15:33
* 
*          Fortran source code generator
*     (c)copyright 1990-1996 Minami-Tateya Group, Japan
*-----------------------------------------------------------------------
      subroutine spdetx(ireco)
*-----------------------------------------------------------------------
*     Selection of Color State for llll,llqq,qqqq
*        output:icolst (0->ncbase-1)
*-----------------------------------------------------------------------
      implicit DOUBLE PRECISION (a-h,o-z)
      include 'incl1.f'
      include 'inclk.f'
      dimension asum(0:ncbase-1),cratio(0:ncbase-1)
      DIMENSION drvec(1)

      if( kmcbas .eq. 1 ) then
          icolst = 0
      else
*-----------------------------------------------------------------------
*        Calcul amplitutes squared.
*-----------------------------------------------------------------------
      do 120 ic = 0 , ncbase-1
      asum(ic) = 0.0d0
      do 110 ih = 0, lag-1
         asum(ic) = asum(ic) + dble(agc(ih,ic))**2 + imag(agc(ih,ic))**2
  110 continue
      asum(ic) = asum(ic)*cfmtx(ic,ic)
  120 continue
*-----------------------------------------------------------------------
*        Search maximum.
*-----------------------------------------------------------------------
      allsum = 0.0d0
      do 130 ic = 0, ncbase-1
         allsum = allsum + asum(ic)
  130 continue
      tmpsum = 0.0d0
      do 140 ic = 0, ncbase-1
         tmpsum = tmpsum + asum(ic)
         cratio(ic) = tmpsum/allsum
  140 continue

!      cran = drn(idummy)
        CALL varran(drvec,1)
        cran=drvec(1)
      icolst = 0
      do 150 ic = 1, ncbase-1
! here was a bug ? zw 20.06.96 if( cratio(ic) .gt. cran ) then
         if( cratio(ic-1) .gt. cran ) then
             goto 160
         else
             icolst = ic
         endif
  150 continue
  160 continue
      endif
      ireco=icolst
      return
      end






