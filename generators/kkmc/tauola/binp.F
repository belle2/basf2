      SUBROUTINE CURR_BINP(MNUM,IMOD,P1,P2,P3,P4,HADCUR)
C     Subroutine counting hadronic current (rho~ -> 4pi)
C     Described in CPC. 146 (2002) 139-153 (hep-ph/0201149)
C
C MNUM = 1 tau+ -> pi,A1(rho+sigma) -> pi+pi-pi+pi0
C MNUM = 2 tau+ -> pi,A1(rho+sigma) -> pi+pi0pi0pi0
C 

      real p1(4),p2(4),p3(4),p4(4),pa(4)
      real*8 scalar ,ss2       
      real ssqrt,ss

      Complex*16 z_summ(4),z_vec(4),factor
      Complex HADCUR(4)

      Logical atStart
      real*8 invMrho4
      real*8 AMass_Rho,Gamma_Rho
      common /zrho_pool/ AMass_Rho,Gamma_Rho
      data Gamma_Rho /0.1445D0/ ! 9 Jan 2002 Chnged Gamma_Rho /0.1477D0/
      data AMass_Rho /0.7761D0/ ! 9 Jan 2002 Chnged AMass_Rho /0.7753D0/
      data atStart/.True./

      if (atStart) then
         atStart = .False.
         invMrho4 = 1.d0/AMass_Rho**4
      endif
         
      call LATA(1,p1,p2,p3,p4)

      z_summ(1) = DCMPLX(0.D0,0.D0)
      z_summ(2) = DCMPLX(0.D0,0.D0)
      z_summ(3) = DCMPLX(0.D0,0.D0)
      z_summ(4) = DCMPLX(0.D0,0.D0)

      do i=1,4
         pa(i)=p1(i)+p2(i)+p3(i)+p4(i)
      enddo   
      ss2=scalar(pa,pa)

      if(MNUM.eq.2) then
         
C        IMOD=-1
C     Calculation Matrix element for tau -> pi,A1 -> pi+pi0pi0pi0
C                                           via rho,pi
         call t1(p2,p3,p1,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p2,p4,p1,p3,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p3,p2,p1,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p3,p4,p1,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p4,p2,p1,p3,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p4,p3,p1,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

C     Calculation Matrix element for tau -> pi,A1  -> pi+pi0pi0pi0
C                                           via sigma,pi

C  !!!! Relative amplitude inserted into 't2'  !!!!!

         call t2(p2,p1,p3,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)
        
         call t2(p3,p1,p2,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t2(p4,p1,p3,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t2(p1,p2,p3,p4,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t2(p1,p3,p2,p4,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t2(p1,p4,p3,p2,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

      elseif(MNUM.eq.1) then
C     Calculation Matrix element for tau -> pi,A1  -> pi+pi-pi+pi0
C                                           via rho,pi
         IF (IMOD.EQ.1) THEN
         call t1(p1,p2,p3,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p3,p2,p1,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p1,p3,p2,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p3,p1,p2,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p4,p3,p1,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t1(p4,p1,p3,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

C     Calculation Matrix element for tau -> pi,A1  -> pi+pi-pi+pi0
C                                           via sigma,pi

C  !!!! Relative amplitude inserted into 't2'  !!!!!

         call t2(p4,p3,p1,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t2(p4,p1,p3,p2,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t2(p3,p4,p1,p2,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t2(p1,p4,p3,p2,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

C     Calculation Matrix element for tau -> pi,omega  -> pi+pi-pi+pi0
C                                           via rho,pi
         ELSEIF (IMOD.EQ.7) THEN

         call t3(p1,p2,p3,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t3(p3,p2,p1,p4,z_vec)
         z_summ(1) = z_summ(1) + z_vec(1)
         z_summ(2) = z_summ(2) + z_vec(2)
         z_summ(3) = z_summ(3) + z_vec(3)
         z_summ(4) = z_summ(4) + z_vec(4)

         call t3(p1,p3,p2,p4,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t3(p3,p1,p2,p4,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t3(p1,p4,p3,p2,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         call t3(p3,p4,p1,p2,z_vec)
         z_summ(1) = z_summ(1) - z_vec(1)
         z_summ(2) = z_summ(2) - z_vec(2)
         z_summ(3) = z_summ(3) - z_vec(3)
         z_summ(4) = z_summ(4) - z_vec(4)

         ENDIF

      else
         go to 910
      endif
      ssqrt=sqrt(ss2)
      ss=ss2
C     
      if (MNUM.eq.1) then
C
         if(IMOD.eq.1) then
C     factor == "function G_pi+_pi-_pi+_pi0(Q^2)" see Eq.(22,23)hep-ph/0201149 
C               and Eq.(1) in internal note.
            factor= fit_a1_1(ssqrt) * 76.565033643843D0*
     $           sqrt(0.71709*ssqrt-0.27505)*invMrho4/ssqrt 
         elseif(IMOD.eq.7) then
C     factor == "function G^{omega}_pi+_pi-_pi+_pi0(Q^2)" see Eq.(24) 
C               hep-ph/0201149 and Eq.(2) in internal note.
            factor= fit_om_1(ssqrt) * 886.837943974463D0 *
     $           sqrt(0.70983*ssqrt-0.26689)*invMrho4/ssqrt
         else
            write(*,*)' Wrong IMOD=',IMOD,' !'
            stop
         endif
C
      elseif(MNUM.eq.2) then
C     factor == "function G_pi+_pi0_pi0_pi0(Q^2)" see Eq.(14,15)hep-ph/0201149 
C               and Eq.(3) in internal note.
         factor=  fit_2(ssqrt) * 96.867161854922D0* ZFA1TAB(ss)*
     $        sqrt(0.70907*ssqrt-0.26413)*invMrho4/ssqrt
C
      else
C
         write(*,*)' WRONG MNUM ! MNUM=',MNUM
         stop
C
      endif
      do i=1,4
         z_summ(i) = z_summ(i)*factor
      enddo   
      DO I=1,4
         HADCUR(I)=Z_SUMM(I)
      ENDDO
      call LATA(-1,p1,p2,p3,p4)
      RETURN
 910  PRINT 9100
 9100 FORMAT(' -----  WRONG VALUE OF MNUM ')
      STOP
      END

      subroutine t1(p1,p2,p3,p4,z_vec)
C     Function t1 - Eq.(16) hep-ph/0201149 (see also internal note 
C     point 5)
C
C     Based on "z_a1rho" subroutine - Novosibirsk code
C     Calculation of matrix element for ee -> pi,a1
C     ee    == p1,A1
C     A1    == p2,Rho
C     Rho   == p3,p4

      IMPLICIT none
      INTEGER i,jnpi

      real p1(4),p2(4),p3(4),p4(4),pa(4)

      complex*16 z_vec(4),z_ee
      Complex*16 Z_a1,Z_rho
      Complex*16 Z_da1,Z_drho,fcom

      real*8 AMass_Rho,Gamma_Rho
      common /zrho_pool/ AMass_Rho,Gamma_Rho
      real*8 AMass_A,Gamma_A,Scale_A
      Complex*16 DSigma_A
      common /za1p_pool/ AMass_A,Gamma_A,Scale_A,DSigma_A
      
      real*8 sa,fs,scalar
      real*8 PPp1,p4Pp1,p3Pp1,p3P,p4P,p1p3,p1p4


      Z_rho = z_drho(p3,p4)
      Pa(1)=p1(1)+p2(1)+p3(1)+p4(1) ! E
      Pa(2)=p1(2)+p2(2)+p3(2)+p4(2) !px
      Pa(3)=p1(3)+p2(3)+p3(3)+p4(3) !py
      Pa(4)=p1(4)+p2(4)+p3(4)+p4(4) !pz

      sa = (pa(1)-p1(1))**2 - (pa(2)-p1(2))**2 - 
     $     (pa(3)-p1(3))**2 - (pa(4)-p1(4))**2

      Z_a1  = z_da1(sa)

      fs = ((1.D0+AMass_A**2/Scale_A)/(1.D0+sa/Scale_A))**2

      fcom = fs/(Z_a1*Z_rho)
      
      PPp1  = scalar(pa,pa) - scalar(pa,p1)
      p4Pp1 = scalar(p4,pa) - scalar(p4,p1)
      p3Pp1 = scalar(p3,pa) - scalar(p3,p1)
      p3P   = scalar(p3,pa)
      p4P   = scalar(p4,pa)
      p1p3  = scalar(p1,p3)
      p1p4  = scalar(p1,p4)
         
      do i=1,4
         z_vec(i) = fcom*(PPp1*(p3(i)*p4Pp1 - p4(i)*p3Pp1) +
     $        (pa(i) - p1(i))*(p3P*p1p4-p4P*p1p3))
      enddo
C     Changing 4 vector convention to (x,y,z,e) like in TAUOLA
      z_ee=z_vec(1)
      do i=1,3
         z_vec(i) = z_vec(i+1)
      enddo   
      z_vec(4)=  z_ee 
      end

      subroutine t2(p1,p2,p3,p4,z_vec)
C     Function t2 - Eq.(17) hep-ph/0201149 
C
C     Based on "z_a1sigma" subroutine - Novosibirsk code
C     Calculation of matrix element for ee -> pi,a1 -> pi,pi,Sigma
C     ee    == p1,A1
C     A1    == p2,Sigma
C     Sigma == p3,p4

      IMPLICIT none
      INTEGER i

      real p1(4),p2(4),p3(4),p4(4),pa(4)

      complex*16 z_vec(4),z_ee
      Complex*16 Z_a1,Z_sgm,fcom
      Complex*16 Z_da1,Z_dsigma

      real*8 AMass_Rho,Gamma_Rho
      common /zrho_pool/ AMass_Rho,Gamma_Rho

      real*8 AMass_A,Gamma_A,Scale_A
      Complex*16 DSigma_A
      common /za1p_pool/ AMass_A,Gamma_A,Scale_A,DSigma_A

      real*8 sa,fs,scalar
      real*8 PPp1,p2P

      Z_sgm = z_dsigma(p3,p4)

      Pa(1)=p1(1)+p2(1)+p3(1)+p4(1) ! E
      Pa(2)=p1(2)+p2(2)+p3(2)+p4(2) !px
      Pa(3)=p1(3)+p2(3)+p3(3)+p4(3) !py
      Pa(4)=p1(4)+p2(4)+p3(4)+p4(4) !pz

      sa = (pa(1)-p1(1))**2 - (pa(2)-p1(2))**2 - 
     $     (pa(3)-p1(3))**2 - (pa(4)-p1(4))**2
      Z_a1  = z_da1(sa)

C     Q^2 for A1

      fs = ((1.D0+AMass_A**2/Scale_A)/(1.D0+sa/Scale_A))**2 

      fcom = fs*DSigma_A/(Z_a1*Z_sgm)*sa
      
      PPp1   = scalar(pa,pa) - scalar(pa,p1)
      p2P    = scalar(p2,pa)   
      do i = 1,4
         z_vec(i) = fcom*(p2(i)*PPp1 + (p1(i)-pa(i))*p2P)
      enddo 
C     Changing 4 vector convention to (x,y,z,e) like in TAUOLA
      z_ee=z_vec(1)
      do i=1,3
         z_vec(i) = z_vec(i+1)
      enddo   
      z_vec(4)=  z_ee
      end

      real*8 function scalar(a,b)
c     real scalar product
      real a(4),b(4)
      scalar = a(1)*b(1)-a(2)*b(2)-a(3)*b(3)-a(4)*b(4)
      end

C---------------------------------------------------------------
C    Breit-Wigner for a1, rho, sigma
C----------------------------------------------------------------
      complex*16 function z_da1(sa_q)
C  Calculation of A1 propagator (see Eq.(18) from hep-ph/0201149 
C  and point 2 from internal note)   
C
C     p_q    - pion  with A1
C     p1,p2  - pions from Rho
C     p3     - pion  from A1
C
      implicit none

      real*8 AMass_A,Gamma_A,Scale_A
      Complex*16 DWave_A
      common /za1p_pool/ AMass_A,Gamma_A,Scale_A,DWave_A

      integer ia1f
      real*8 ama1,gma1,gmv0

      real*8 sa_q,pm,pm0
      real*8 gma1v

      data ia1f/0/
      data Gamma_A /0.45D0/
      data AMass_A /1.23D0/
      data Scale_A /1.2D0/
      data DWave_A / (1.269D0,0.591D0) /
      
C New way of correction
C
      if(ia1f.eq.0) then
         ia1f = 1

         ama1 = AMass_A**2
         gma1 = Gamma_A/AMass_A
         gmv0 = gma1v(ama1)
      endif
      pm0 = gmv0               ! normalization width
      pm  = gma1v(sa_q)        ! resonanse width

C     Normalization
C
      z_da1 = DCMPLX(sa_q/ama1-1.D0,gma1*pm/pm0)
      end

      double precision function gma1v(X2)
C     a1 width, a1 -> pi+ pi- pi0 or 3pi0
C     Eq. (18) hep-ph/0201149 and points 6-9 from internal note
      implicit none

      real*8 AMass_A,Gamma_A,Scale_A
      Complex*16 Z
      common /za1p_pool/ AMass_A,Gamma_A,Scale_A,Z

      real*8 fs
      REAL*8 X2,DELTA,GA1MY
      INTEGER I
      REAL*8 S(100),INTE(100),INT_ABS(100),INT_RE(100),INT_IM(100)
      DATA S/0.17531806E+00,0.20062977E+00,0.22594148E+00,0.25125318E+00,
     #0.27656489E+00,0.30187660E+00,0.32718830E+00,0.35250001E+00,0.37781172E+00,0.40312342E+00,
     #0.42843513E+00,0.45374684E+00,0.47905854E+00,0.50437025E+00,0.52968195E+00,0.55499366E+00,
     #0.58030537E+00,0.60561707E+00,0.63092878E+00,0.65624049E+00,0.68155219E+00,0.70686390E+00,
     #0.73217561E+00,0.75748731E+00,0.78279902E+00,0.80811073E+00,0.83342243E+00,0.85873414E+00,
     #0.88404585E+00,0.90935755E+00,0.93466926E+00,0.95998096E+00,0.98529267E+00,0.10106044E+01,
     #0.10359161E+01,0.10612278E+01,0.10865395E+01,0.11118512E+01,0.11371629E+01,0.11624746E+01,
     #0.11877863E+01,0.12130980E+01,0.12384097E+01,0.12637214E+01,0.12890331E+01,0.13143449E+01,
     #0.13396566E+01,0.13649683E+01,0.13902800E+01,0.14155917E+01,0.14409034E+01,0.14662151E+01,
     #0.14915268E+01,0.15168385E+01,0.15421502E+01,0.15674619E+01,0.15927736E+01,0.16180853E+01,
     #0.16433970E+01,0.16687087E+01,0.16940205E+01,0.17193322E+01,0.17446439E+01,0.17699556E+01,
     #0.17952673E+01,0.18205790E+01,0.18458907E+01,0.18712024E+01,0.18965141E+01,0.19218258E+01,
     #0.19471375E+01,0.19724492E+01,0.19977609E+01,0.20230726E+01,0.20483843E+01,0.20736960E+01,
     #0.20990078E+01,0.21243195E+01,0.21496312E+01,0.21749429E+01,0.22002546E+01,0.22255663E+01,
     #0.22508780E+01,0.22761897E+01,0.23015014E+01,0.23268131E+01,0.23521248E+01,0.23774365E+01,
     #0.24027482E+01,0.24280599E+01,0.24533716E+01,0.24786834E+01,0.25039951E+01,0.25293068E+01,
     #0.25546185E+01,0.25799302E+01,0.26052419E+01,0.26305536E+01,0.26558653E+01,0.26811770E+01/
      DATA INTE/0.00000000E+00,0.19835316E-09,0.17277872E-08,0.63260250E-08,
     #0.16228446E-07,0.34254199E-07,0.63924158E-07,0.10961715E-06,0.17677484E-06,0.27217027E-06,
     #0.40426145E-06,0.58366008E-06,0.82375764E-06,0.11415688E-05,0.15588754E-05,0.21037881E-05,
     #0.28128794E-05,0.37340827E-05,0.49305520E-05,0.64855462E-05,0.85079467E-05,0.11136565E-04,
     #0.14538578E-04,0.18892731E-04,0.24346357E-04,0.30949160E-04,0.38601580E-04,0.47069702E-04,
     #0.56065745E-04,0.65330521E-04,0.74672459E-04,0.83967396E-04,0.93142038E-04,0.10215726E-03,
     #0.11099490E-03,0.11964932E-03,0.12812206E-03,0.13641877E-03,0.14454677E-03,0.15251471E-03,
     #0.16033133E-03,0.16800535E-03,0.17554509E-03,0.18295843E-03,0.19025283E-03,0.19743618E-03,
     #0.20451223E-03,0.21148974E-03,0.21837340E-03,0.22516843E-03,0.23187967E-03,0.23851165E-03,
     #0.24506849E-03,0.25155433E-03,0.25797236E-03,0.26432627E-03,0.27061916E-03,0.27685378E-03,
     #0.28303336E-03,0.28916027E-03,0.29523695E-03,0.30126590E-03,0.30724868E-03,0.31318791E-03,
     #0.31908532E-03,0.32494289E-03,0.33076174E-03,0.33654407E-03,0.34229119E-03,0.34800456E-03,
     #0.35368553E-03,0.35933539E-03,0.36495540E-03,0.37054666E-03,0.37611031E-03,0.38164760E-03,
     #0.38715892E-03,0.39264583E-03,0.39810880E-03,0.40354939E-03,0.40896773E-03,0.41436490E-03,
     #0.41974145E-03,0.42509830E-03,0.43043607E-03,0.43575541E-03,0.44105696E-03,0.44634150E-03,
     #0.45160906E-03,0.45686072E-03,0.46209683E-03,0.46731790E-03,0.47252442E-03,0.47771686E-03,
     #0.48289566E-03,0.48806125E-03,0.49321407E-03,0.49835450E-03,0.50348292E-03,0.50859971E-03/
      DATA INT_ABS/0.00000000E+00,0.21066009E-09,0.16564992E-08,0.55359496E-08,
     #0.13052891E-07,0.25435346E-07,0.43943074E-07,0.69871482E-07,0.10455410E-06,0.14936437E-06,
     #0.20571712E-06,0.27506977E-06,0.35892330E-06,0.45882244E-06,0.57635873E-06,0.71316741E-06,
     #0.87092972E-06,0.10513722E-05,0.12562662E-05,0.14874272E-05,0.17467133E-05,0.20360241E-05,
     #0.23572986E-05,0.27125129E-05,0.31036774E-05,0.35328342E-05,0.40020531E-05,0.45134287E-05,
     #0.50690759E-05,0.56711263E-05,0.63217233E-05,0.70230186E-05,0.77771640E-05,0.85863202E-05,
     #0.94526337E-05,0.10378245E-04,0.11365281E-04,0.12415854E-04,0.13532054E-04,0.14715952E-04,
     #0.15969588E-04,0.17294974E-04,0.18694106E-04,0.20168926E-04,0.21721372E-04,0.23353312E-04,
     #0.25066665E-04,0.26863051E-04,0.28744432E-04,0.30712501E-04,0.32768956E-04,0.34915462E-04,
     #0.37153651E-04,0.39485116E-04,0.41911400E-04,0.44434054E-04,0.47054555E-04,0.49774355E-04,
     #0.52594877E-04,0.55517507E-04,0.58543603E-04,0.61674489E-04,0.64911460E-04,0.68255782E-04,
     #0.71708689E-04,0.75271391E-04,0.78945069E-04,0.82730875E-04,0.86629940E-04,0.90643366E-04,
     #0.94772202E-04,0.99017560E-04,0.10338045E-03,0.10786187E-03,0.11246282E-03,0.11718426E-03,
     #0.12204385E-03,0.12699238E-03,0.13208090E-03,0.13729358E-03,0.14263128E-03,0.14809487E-03,
     #0.15368518E-03,0.15940303E-03,0.16524921E-03,0.17122452E-03,0.17732972E-03,0.18356558E-03,
     #0.18993282E-03,0.19643219E-03,0.20306438E-03,0.20983011E-03,0.21673005E-03,0.22376489E-03,
     #0.23093528E-03,0.23824187E-03,0.24568530E-03,0.25326619E-03,0.26098517E-03,0.26884283E-03/
      DATA INT_RE/0.00000000E+00,-.38711044E-09,-.30791558E-08,-.10445232E-07,
     #-.25062489E-07,-.49803729E-07,-.87911341E-07,-.14307792E-06,-.21954305E-06,-.32221399E-06,
     #-.45681885E-06,-.63010309E-06,-.85008368E-06,-.11263793E-05,-.14706404E-05,-.18971082E-05,
     #-.24233327E-05,-.30710710E-05,-.38673463E-05,-.48455316E-05,-.60460375E-05,-.75155863E-05,
     #-.93030466E-05,-.11448885E-04,-.13966772E-04,-.16823546E-04,-.19934441E-04,-.23185919E-04,
     #-.26472191E-04,-.29719254E-04,-.32887419E-04,-.35962204E-04,-.38944230E-04,-.41841471E-04,
     #-.44665024E-04,-.47426566E-04,-.50137251E-04,-.52807218E-04,-.55445447E-04,-.58059837E-04,
     #-.60657047E-04,-.63242968E-04,-.65822615E-04,-.68400294E-04,-.70979704E-04,-.73564030E-04,
     #-.76155632E-04,-.78758044E-04,-.81372163E-04,-.84000166E-04,-.86643611E-04,-.89303861E-04,
     #-.91982105E-04,-.94679390E-04,-.97396632E-04,-.10013464E-03,-.10289412E-03,-.10567554E-03,
     #-.10847991E-03,-.11130726E-03,-.11415800E-03,-.11703298E-03,-.11993205E-03,-.12285565E-03,
     #-.12580402E-03,-.12877720E-03,-.13177591E-03,-.13479977E-03,-.13784908E-03,-.14092395E-03,
     #-.14402448E-03,-.14715072E-03,-.15030274E-03,-.15348058E-03,-.15668426E-03,-.15991379E-03,
     #-.16316917E-03,-.16645040E-03,-.16975745E-03,-.17309031E-03,-.17644893E-03,-.17983303E-03,
     #-.18324331E-03,-.18667896E-03,-.19014018E-03,-.19362689E-03,-.19713905E-03,-.20067656E-03,
     #-.20423936E-03,-.20782708E-03,-.21144049E-03,-.21507866E-03,-.21874178E-03,-.22242976E-03,
     #-.22614252E-03,-.22987995E-03,-.23364196E-03,-.23742846E-03,-.24123935E-03,-.24507452E-03/
      DATA INT_IM/0.00000000E+00,0.12002091E-09,0.12850209E-08,0.51141443E-08,
     #0.13632230E-07,0.29227933E-07,0.54655557E-07,0.93051069E-07,0.14795445E-06,0.22333412E-06,
     #0.32360920E-06,0.45366352E-06,0.61884173E-06,0.82491108E-06,0.10779615E-05,0.13841975E-05,
     #0.17495437E-05,0.21789224E-05,0.26750078E-05,0.32360835E-05,0.38525487E-05,0.45015863E-05,
     #0.51400771E-05,0.56980360E-05,0.60787180E-05,0.61747611E-05,0.59005563E-05,0.52216022E-05,
     #0.41575455E-05,0.27611998E-05,0.10942719E-05,-.78733816E-06,-.28386020E-05,-.50252585E-05,
     #-.73219124E-05,-.97098783E-05,-.12175421E-04,-.14708367E-04,-.17301124E-04,-.19947904E-04,
     #-.22644299E-04,-.25386852E-04,-.28172810E-04,-.30999930E-04,-.33866347E-04,-.36770471E-04,
     #-.39710918E-04,-.42686461E-04,-.45695990E-04,-.48738489E-04,-.51813066E-04,-.54918732E-04,
     #-.58054706E-04,-.61220193E-04,-.64414436E-04,-.67636711E-04,-.70886320E-04,-.74162594E-04,
     #-.77464884E-04,-.80792567E-04,-.84145039E-04,-.87521715E-04,-.90922031E-04,-.94345438E-04,
     #-.97791406E-04,-.10125942E-03,-.10474898E-03,-.10825960E-03,-.11179082E-03,-.11534217E-03,
     #-.11891322E-03,-.12250352E-03,-.12611268E-03,-.12974120E-03,-.13338592E-03,-.13704922E-03,
     #-.14072981E-03,-.14442733E-03,-.14814143E-03,-.15187176E-03,-.15561799E-03,-.15937980E-03,
     #-.16315687E-03,-.16694889E-03,-.17075556E-03,-.17457660E-03,-.17841171E-03,-.18226185E-03,
     #-.18612432E-03,-.19000007E-03,-.19388755E-03,-.19778906E-03,-.20170309E-03,-.20562942E-03,
     #-.20956779E-03,-.21351800E-03,-.21747982E-03,-.22145304E-03,-.22543745E-03,-.22943284E-03/

      DELTA = S(2)-S(1)
      I = INT((X2-S(1))/DELTA) + 1
      
      GA1MY = INTE(I) + (INTE(I+1)-INTE(I))/DELTA*(X2-S(I))
      GA1MY = GA1MY + ABS(Z)**2 * (INT_ABS(I) + (INT_ABS(I+1)-INT_ABS(I))/DELTA*(X2-S(I)))
      GA1MY = GA1MY + DBLE(Z)* (INT_RE(I) + (INT_RE(I+1)-INT_RE(I))/DELTA*(X2-S(I)))
      GA1MY = GA1MY + DIMAG(Z)*(INT_IM(I) + (INT_IM(I+1)-INT_IM(I))/DELTA*(X2-S(I))) 

      fs = ((1.D0+AMass_A**2/Scale_A)/(1.D0+X2/Scale_A))**2
      gma1v=fs*GA1MY
      end

      complex*16 function z_dsigma(p1,p2)
C  Calculation of Sigma propagator (see Eq.(18) from hep-ph/0201149
C  and point 2 from internal note)
C
C     p1,p2  - pions from Sigma
C
      implicit none

      real*8 AMass_S,Gamma_S

      real p1(4),p2(4)
      real*8 ps1,ps2,am2_1,am2_2,as,d1,d2,dsq,pm,pm0,dd,am12
      real*8 am1,am2

      data Gamma_S /0.8D0/
      data AMass_S /0.8D0/

      ps1 = p1(2)**2+p1(3)**2+p1(4)**2
      ps2 = p2(2)**2+p2(3)**2+p2(4)**2

      am2_1 = p1(1)**2-ps1
      am2_2 = p2(1)**2-ps2

      am12 = p1(1)*p2(1)-p1(2)*p2(2)-p1(3)*p2(3)-p1(4)*p2(4)
      as = am2_1+am2_2+2.D0*am12   ! Sigma invariant mass

      am1 = sqrt(am2_1)
      am2 = sqrt(am2_2)
      d1 = 1.D0-(am1+am2)**2/as
      d2 = 1.D0-(am1-am2)**2/as
      dd = max(d1*d2,1.D-16)

      dsq = sqrt(dd)
      pm = dsq

      d1 = 1.D0-(am1+am2)**2/AMass_S**2
      d2 = 1.D0-(am1-am2)**2/AMass_S**2
      dd = d1*d2
      dsq = sqrt(dd)
      pm0 = dsq

      z_dsigma = DCMPLX(as/AMass_S**2-1.D0,Gamma_S/AMass_S*pm/pm0)
      end

      complex*16 function z_drho(p1,p2)
C  Calculation of rho propagator (see Eq.(18) from hep-ph/0201149 
C  and point 2,3(Eq.(4)) from internal note)   

C     Denominator of Rho propagator.
      implicit none
      real p1(4),p2(4)

      real*8 AMass_Rho,Gamma_Rho,ampi,s,g,GamMas,dm
      common /zrho_pool/ AMass_Rho,Gamma_Rho

      Parameter (ampi=.13957D0)

      g(s) = sqrt(MAX((s-4.*ampi**2)**3/s,0.d0))
      
      s = (p1(1)+p2(1))**2 -(p1(2)+p2(2))**2-(p1(3)+p2(3))**2-
     #     (p1(4)+p2(4))**2

      GamMas = Gamma_Rho*AMass_Rho
      z_drho = CMPLX(s-AMass_Rho**2-dm(s)*GamMas,GamMas*g(s)/g(AMass_Rho**2))
      z_drho = z_drho / AMass_Rho**2 
      z_drho = z_drho /(1.+ Gamma_Rho/AMass_Rho*dm(0.d0)) ! Normalization
      end

      real*8 function dm(s)
C     Mass correction to the rho propagator (see Eq.(4,5) from internal note)
      implicit none
      real*8 s,m2,gm2,hrho,dhrho
      real*8 ampi,pi
      real*8 AMass_Rho,Gamma_Rho
      common /zrho_pool/ AMass_Rho,Gamma_Rho
      Parameter (ampi=.13957D0)
      Parameter (pi=3.1415926D0)
      
      m2 = AMass_Rho**2
      gm2=m2*(sqrt(1.D0-(2.D0*ampi)**2/m2))**3
      dm = (hrho(s)-hrho(m2)-(s-m2)*dhrho(m2))/gm2
      end
      real*8 function hrho(s)
C     See Eq.(5) from internal note
      implicit none
      real*8 s,pi,ampi,y,w
      Parameter (ampi=.13957D0)
      Parameter (pi=3.1415926D0)
      
      if (s.eq.0.d0) then
         hrho = -2.d0*4.d0*ampi**2/pi
      else
         y=1.d0-4.d0*ampi**2/s
         y = sqrt(max(0.d0,y))
         w = y*log((1.d0+y)/(1.d0-y))
         hrho = w*(s-4.d0*ampi**2)/pi
      endif
      end
      real*8 function dhrho(s)
C     Derivative of function hrho(s), for more information see Eq.(4,5)
C     from internal note
      implicit none
      real*8 s,pi,ampi,y,a,w,dy
      Parameter (ampi=.13957D0)
      Parameter (pi=3.1415926D0)

      y=1.d0-4.d0*ampi**2/s
      y = sqrt(max(0.d0,y))
      a = log((1.d0+y)/(1.d0-y))
      w = y*a
      dy= 4.d0*ampi**2/2.d0/s/y
      dhrho = (w + (dy*a+1.d0)*y**2)/pi
      end

      subroutine LATA(key,p1,p2,p3,p4)
C     Subroutine changing 4 vector convention (x,y,z,e) <=> (e,x,y,z)
      real ee1,ee2,ee3,ee4,p1(4),p2(4),p3(4),p4(4)
      integer key
      if (key.eq.1) then ! (x,y,z,e) -> (e,x,y,z)
C     from TAUOLA to BINP
         ee1=p1(4)
         ee2=p2(4)
         ee3=p3(4)
         ee4=p4(4)
         do i=3,1,-1
            p1(i+1)=p1(i)
            p2(i+1)=p2(i)
            p3(i+1)=p3(i)
            p4(i+1)=p4(i)
         enddo
         p1(1)=ee1
         p2(1)=ee2
         p3(1)=ee3
         p4(1)=ee4
      else               ! (e,x,y,z) -> (x,y,z,e)
         ee1=p1(1)
         ee2=p2(1)
         ee3=p3(1)
         ee4=p4(1)
         do i=1,3
            p1(i)=p1(i+1)
            p2(i)=p2(i+1)
            p3(i)=p3(i+1)
            p4(i)=p4(i+1)
         enddo
         p1(4)=ee1
         p2(4)=ee2
         p3(4)=ee3
         p4(4)=ee4
      endif   
      end
      subroutine t3(p1,p2,p3,p4,z_vec)
C     Function t3 - Eq.(25) hep-ph/0201149 (see also internal note point 10)
C
C     Based on "z_omegav" subroutine - Novosibirsk code
C     Calculation of matrix element for tau- -> pi-,omega
C     Rho   == p3,p4
C     Omega == p2,Rho

      IMPLICIT none

      INTEGER i
      real p1(4),p2(4),p3(4),p4(4),PA(4)

      real*8 scalar,Pp2,Pp3,Pp4,p1p2,p1p3,p1p4
      complex*16 z_vec(4),z_ee
      Complex*16 Z_a1,Z_rho
      Complex*16 Z_domega,Z_drho,fcom,zmix

      real*8 AMass_Rho,Gamma_Rho
      common /zrho_pool/ AMass_Rho,Gamma_Rho
      real*8 som,fs

      zmix=DCMPLX(1.D0,0.D0)

      Z_rho = z_drho(p3,p4)
      Pa(1)=p1(1)+p2(1)+p3(1)+p4(1) ! E
      Pa(2)=p1(2)+p2(2)+p3(2)+p4(2) !px
      Pa(3)=p1(3)+p2(3)+p3(3)+p4(3) !py
      Pa(4)=p1(4)+p2(4)+p3(4)+p4(4) !pz

      som = (pa(1)-p1(1))**2 - (pa(2)-p1(2))**2 - 
     $     (pa(3)-p1(3))**2 - (pa(4)-p1(4))**2

      fs = 1.0D0
      Z_a1  = z_domega(som)
      
      fcom = fs/(Z_a1*Z_rho)
      Pp2 = scalar(pa,p2)
      Pp3 = scalar(pa,p3)
      Pp4 = scalar(pa,p4)
      p1p2= scalar(p1,p2)
      p1p3= scalar(p1,p3)
      p1p4= scalar(p1,p4)
      do i=1,4
         z_vec(i)=fcom*(p2(i)*(Pp3*p1p4-Pp4*p1p3)-Pp2*(p3(i)*p1p4-
     $        p4(i)*p1p3) + p1p2*(p3(i)*Pp4-p4(i)*Pp3)) * zmix
      enddo
C     Changing 4 vector convention to (x,y,z,e) like in TAUOLA
      z_ee=z_vec(1)
      do i=1,3
         z_vec(i) = z_vec(i+1)
      enddo   
      z_vec(4)= z_ee
      end

      complex*16 function z_domega(som)
C  Calculation of Omega propagator : ee-> pi0(q) + Omega(pi(p3),Rho)
C  Eq.(18) from hep-ph/0201149 and points 2,4 from internal note
C     p_q    - pion  with Omega
C     p1,p2  - pions from Rho
C     p3     - pion  from Omega
C
      implicit none
      real*8 AMass_O,Gamma_O
      real*8 som,gom,gomega
      common /omega2/AMass_O

      data Gamma_O /0.00841D0/
      data AMass_O /0.782D0/

      gom = gomega(sqrt(som))
      z_domega = DCMPLX(som/(AMass_O**2)-1.D0,Gamma_O/AMass_O*gom)
      end
      real*8 function gomega(x)
C     Evolution of width of omega (see Eq.(7) from internal note)
      implicit none
      real*8 x
      real*8 AMass_O,PAR(10)
      common /omega2/AMass_O
      data par/17.5598888d0,141.110153d0,894.884460d0,4977.35107d0,
     #7610.65625d0,-42524.4062d0,-1333.26282d0,4860.18799d0,
     #-6000.80908d0,2504.97461d0/
      if (x.le.1.) then
      gomega = 1. + par(1)*(x-0.782)+par(2)*(x-0.782)**2
     # +par(3)*(x-0.782)**3+par(4)*(x-0.782)**4+par(5)*(x-0.782)**5
     # +par(6)*(x-0.782)**6
      else
        gomega =  par(7)+par(8)*x+par(9)*x**2+par(10)*x**3
      endif
      gomega = max(0.d0,gomega)
      end

      REAL FUNCTION ZFA1TAB(Q2)
C     Tabulated  value of complex function, describing the energy
C     dependence of the cross section e+e- -> a1 pi -> 2pi+ 2pi- (see comments
C     in internal note - just below Eq.(3) )
      IMPLICIT NONE
      INTEGER I
      REAL*8 DELTA
      REAL Q2
      REAL S(100),VAL(100)
      SAVE S,VAL
      DATA S/ 0.2916000,0.3206586,0.3497172,0.3787757,
     # 0.4078344,0.4368929,0.4659515,0.4950101,0.5240687,0.5531273,
     # 0.5821859,0.6112444,0.6403030,0.6693616,0.6984202,0.7274788,
     # 0.7565374,0.7855960,0.8146545,0.8437131,0.8727717,0.9018303,
     # 0.9308889,0.9599475,0.9890060,1.0180646,1.0471232,1.0761818,
     # 1.1052403,1.1342990,1.1633576,1.1924162,1.2214748,1.2505333,
     # 1.2795919,1.3086505,1.3377091,1.3667676,1.3958262,1.4248848,
     # 1.4539435,1.4830021,1.5120606,1.5411192,1.5701778,1.5992364,
     # 1.6282949,1.6573535,1.6864121,1.7154707,1.7445292,1.7735878,
     # 1.8026465,1.8317051,1.8607637,1.8898222,1.9188808,1.9479394,
     # 1.9769980,2.0060565,2.0351152,2.0641737,2.0932324,2.1222908,
     # 2.1513495,2.1804080,2.2094667,2.2385252,2.2675838,2.2966425,
     # 2.3257010,2.3547597,2.3838181,2.4128768,2.4419353,2.4709940,
     # 2.5000525,2.5291111,2.5581696,2.5872283,2.6162868,2.6453454,
     # 2.6744041,2.7034626,2.7325213,2.7615798,2.7906384,2.8196969,
     # 2.8487556,2.8778141,2.9068727,2.9359312,2.9649899,2.9940486,
     # 3.0231071,3.0521657,3.0812242,3.1102829,3.1393414,3.1684000/
      DATA VAL/ 2.0261996,2.2349865,2.4839740,2.7840748,
     # 3.1488798,3.5936222,4.1301847,4.7517977,5.3984838,5.9147439,
     # 6.0864558,5.8283591,5.2841811,4.6615186,4.0839195,3.5914702,
     # 3.1841860,2.8494759,2.5732665,2.3434010,2.1502059,1.9862038,
     # 1.8456544,1.7241427,1.6182493,1.5253036,1.4432002,1.3702650,
     # 1.3051554,1.2467849,1.1942677,1.1468738,1.1039963,1.0651271,
     # 1.0298390,0.9977714,0.9686196,0.9421255,0.9180685,0.8962603,
     # 0.8765374,0.8587573,0.8427927,0.8285285,0.8158574,0.8046767,
     # 0.7948853,0.7863811,0.7790571,0.7728010,0.7674922,0.7630011,
     # 0.7591889,0.7559078,0.7530031,0.7503147,0.7476809,0.7449428,
     # 0.7419487,0.7385587,0.7346500,0.7301207,0.7248930,0.7189151,
     # 0.7121620,0.7046344,0.6963565,0.6873729,0.6777444,0.6675445,
     # 0.6568548,0.6457604,0.6343476,0.6227004,0.6108983,0.5990148,
     # 0.5871165,0.5752623,0.5635037,0.5518846,0.5404415,0.5292045,
     # 0.5181981,0.5074410,0.4969472,0.4867267,0.4767860,0.4671288,
     # 0.4577557,0.4486661,0.4398569,0.4313242,0.4230627,0.4150662,
     # 0.4073282,0.3998415,0.3925985,0.3855914,0.3788125,0.3722538/
             

      DELTA = (S(100)-S(1))/99.D0
      I = INT((Q2-S(1))/DELTA) + 1
      
      ZFA1TAB = VAL(I) + (VAL(I+1)-VAL(I))/(S(I+1)-S(I))*(Q2-S(I))
      END
       
      REAL FUNCTION fit_a1_1(E)
C     Tabulated fit function for chanel 2pi+ pi- pi0 
C     (contribution from a1 -> 3pi),
C     it is a part of G_pi+_pi-_pi+_pi0(Q^2) function.
      IMPLICIT NONE
      INTEGER I
      REAL E,FIT
      REAL ARG(98),VAL(98)
      SAVE ARG,VAL
      DATA ARG/ 0.6000000,0.6131313,0.6262626,0.6393939,
     # 0.6525252,0.6656566,0.6787879,0.6919192,0.7050505,0.7181818,
     # 0.7313131,0.7444444,0.7575758,0.7707071,0.7838384,0.7969697,
     # 0.8101010,0.8232324,0.8363636,0.8494949,0.8626263,0.8757576,
     # 0.8888889,0.9020202,0.9151515,0.9282829,0.9414141,0.9545454,
     # 0.9676768,0.9808081,0.9939394,1.0070707,1.0202020,1.0333333,
     # 1.0464647,1.0595959,1.0727273,1.0858586,1.0989898,1.1121212,
     # 1.1252525,1.1383839,1.1515151,1.1646465,1.1777778,1.1909091,
     # 1.2040404,1.2171717,1.2303030,1.2434343,1.2565657,1.2696970,
     # 1.2828283,1.2959596,1.3090909,1.3222222,1.3353535,1.3484849,
     # 1.3616161,1.3747475,1.3878788,1.4010102,1.4141414,1.4272727,
     # 1.4404041,1.4535353,1.4666667,1.4797980,1.4929293,1.5060606,
     # 1.5191919,1.5323232,1.5454545,1.5585859,1.5717171,1.5848485,
     # 1.5979798,1.6111112,1.6242424,1.6373737,1.6505051,1.6636363,
     # 1.6767677,1.6898990,1.7030303,1.7161616,1.7292930,1.7424242,
     # 1.7555555,1.7686869,1.7818182,1.7949495,1.8080808,1.8212122,
     # 1.8343434,1.8474747,1.8606061,1.8737373/
      DATA VAL/  0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,13.1664906,
     #10.7234087, 8.8219614,10.7989664, 9.1883001, 7.8526378, 7.7481031,
     # 8.2633696, 5.5042820, 4.9029269, 4.4794345, 3.9654009, 4.5254011,
     # 3.6509495, 3.5005512, 3.2274280, 3.1808922, 2.9925177, 2.6886659,
     # 2.5195024, 2.4678771, 2.3540580, 2.2123868, 2.1103525, 2.0106986,
     # 1.8792295, 1.8250662, 1.7068460, 1.6442842, 1.5503920, 1.4814349,
     # 1.4225838, 1.3627135, 1.3205355, 1.2784383, 1.2387408, 1.1975995,
     # 1.1633024, 1.1318133, 1.1114354, 1.0951439, 1.0691465, 1.0602311,
     # 1.0392803, 1.0220672, 1.0154786, 1.0010130, 0.9908018, 0.9710845,
     # 0.9602382, 0.9488459, 0.9316537, 0.9118049, 0.8920435, 0.8719332,
     # 0.8520256, 0.8280582, 0.8064085, 0.7767881, 0.7570597, 0.7382626,
     # 0.7100251, 0.6846500, 0.6666913, 0.6372250, 0.6162248, 0.6007728,
     # 0.5799103, 0.5674670, 0.5446148, 0.5352115, 0.5128809, 0.4932536,
     # 0.5310397, 0.8566489, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000/

      if (E.lt.0.6) then
         fit=0.
      elseif(E.lt.1.777) then
         do i=98,1,-1
            if (arg(i).le.E) goto 100
         enddo
 100     fit=val(i)+(val(i+1)-val(i))/(arg(i+1)-arg(i))*(E-arg(i))
      else
         fit=0.
      endif
      fit_a1_1=fit
      END
      REAL FUNCTION fit_om_1(E)
C     Tabulated fit function for chanel 2pi+ pi- pi0 
C     (contribution from omega -> 3pi),
C     it is a part of G^{omega}_pi+_pi-_pi+_pi0(Q^2) function.
      IMPLICIT NONE
      INTEGER I
      REAL E,FIT
      REAL ARG(98),VAL(98)
      SAVE ARG,VAL
      DATA ARG/ 0.6000000,0.6131313,0.6262626,0.6393939,
     # 0.6525252,0.6656566,0.6787879,0.6919192,0.7050505,0.7181818,
     # 0.7313131,0.7444444,0.7575758,0.7707071,0.7838384,0.7969697,
     # 0.8101010,0.8232324,0.8363636,0.8494949,0.8626263,0.8757576,
     # 0.8888889,0.9020202,0.9151515,0.9282829,0.9414141,0.9545454,
     # 0.9676768,0.9808081,0.9939394,1.0070707,1.0202020,1.0333333,
     # 1.0464647,1.0595959,1.0727273,1.0858586,1.0989898,1.1121212,
     # 1.1252525,1.1383839,1.1515151,1.1646465,1.1777778,1.1909091,
     # 1.2040404,1.2171717,1.2303030,1.2434343,1.2565657,1.2696970,
     # 1.2828283,1.2959596,1.3090909,1.3222222,1.3353535,1.3484849,
     # 1.3616161,1.3747475,1.3878788,1.4010102,1.4141414,1.4272727,
     # 1.4404041,1.4535353,1.4666667,1.4797980,1.4929293,1.5060606,
     # 1.5191919,1.5323232,1.5454545,1.5585859,1.5717171,1.5848485,
     # 1.5979798,1.6111112,1.6242424,1.6373737,1.6505051,1.6636363,
     # 1.6767677,1.6898990,1.7030303,1.7161616,1.7292930,1.7424242,
     # 1.7555555,1.7686869,1.7818182,1.7949495,1.8080808,1.8212122,
     # 1.8343434,1.8474747,1.8606061,1.8737373/
      DATA VAL/  0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 2.2867811, 2.9710648, 2.9344304, 2.6913538,
     # 2.5471206, 2.3557470, 2.2448280, 2.1074708, 2.0504866, 1.9270257,
     # 1.8669430, 1.7907301, 1.7184515, 1.6535717, 1.6039416, 1.5535343,
     # 1.5065620, 1.4608675, 1.4215596, 1.3849826, 1.3480113, 1.3147917,
     # 1.2793381, 1.2487282, 1.2184237, 1.1952927, 1.1683835, 1.1458827,
     # 1.1145806, 1.0935820, 1.0608720, 1.0390474, 1.0164336, 0.9908721,
     # 0.9585276, 0.9307971, 0.9017274, 0.8731154, 0.8452763, 0.8145532,
     # 0.7817339, 0.7493086, 0.7199919, 0.6887290, 0.6568120, 0.6255773,
     # 0.5944664, 0.5661956, 0.5391204, 0.5102391, 0.4786543, 0.4546428,
     # 0.4316779, 0.4063754, 0.3769831, 0.3561141, 0.3333555, 0.3139160,
     # 0.2949214, 0.2814728, 0.2602444, 0.2349602, 0.2269845, 0.2192318,
     # 0.2286938, 0.2839763, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000/

      if (E.lt.0.6) then
         fit=0.
      elseif(E.lt.1.777) then
         do i=98,1,-1
            if (arg(i).le.E) goto 100
         enddo
 100     fit=val(i)+(val(i+1)-val(i))/(arg(i+1)-arg(i))*(E-arg(i))
      else
         fit=0.
      endif
      fit_om_1=fit
      END
      REAL FUNCTION fit_2(E)
C     Tabulated fit function for chanel tau- -> pi- 3pi0,
C     it is a part of G_pi-_pi0_pi0_pi0(Q^2) function.
      IMPLICIT NONE
      INTEGER I
      REAL E,FIT
      REAL ARG(98),VAL(98)
      SAVE ARG,VAL
      DATA ARG/ 0.6000000,0.6131313,0.6262626,0.6393939,
     # 0.6525252,0.6656566,0.6787879,0.6919192,0.7050505,0.7181818,
     # 0.7313131,0.7444444,0.7575758,0.7707071,0.7838384,0.7969697,
     # 0.8101010,0.8232324,0.8363636,0.8494949,0.8626263,0.8757576,
     # 0.8888889,0.9020202,0.9151515,0.9282829,0.9414141,0.9545454,
     # 0.9676768,0.9808081,0.9939394,1.0070707,1.0202020,1.0333333,
     # 1.0464647,1.0595959,1.0727273,1.0858586,1.0989898,1.1121212,
     # 1.1252525,1.1383839,1.1515151,1.1646465,1.1777778,1.1909091,
     # 1.2040404,1.2171717,1.2303030,1.2434343,1.2565657,1.2696970,
     # 1.2828283,1.2959596,1.3090909,1.3222222,1.3353535,1.3484849,
     # 1.3616161,1.3747475,1.3878788,1.4010102,1.4141414,1.4272727,
     # 1.4404041,1.4535353,1.4666667,1.4797980,1.4929293,1.5060606,
     # 1.5191919,1.5323232,1.5454545,1.5585859,1.5717171,1.5848485,
     # 1.5979798,1.6111112,1.6242424,1.6373737,1.6505051,1.6636363,
     # 1.6767677,1.6898990,1.7030303,1.7161616,1.7292930,1.7424242,
     # 1.7555555,1.7686869,1.7818182,1.7949495,1.8080808,1.8212122,
     # 1.8343434,1.8474747,1.8606061,1.8737373/
      DATA VAL/  0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 1.4819183,
     # 1.7086354, 1.6958492, 1.6172935, 1.6301320, 1.5719706, 1.5459771,
     # 1.5377471, 1.5008864, 1.4924121, 1.4720882, 1.4371741, 1.3990080,
     # 1.3879193, 1.4030601, 1.3768673, 1.3493533, 1.3547127, 1.3275831,
     # 1.3167892, 1.3035913, 1.2968298, 1.2801558, 1.2650299, 1.2557997,
     # 1.2325822, 1.2210644, 1.1935984, 1.1746194, 1.1510350, 1.1358515,
     # 1.1205584, 1.1010553, 1.0903869, 1.0731295, 1.0578678, 1.0438409,
     # 1.0377911, 1.0253277, 1.0103551, 1.0042409, 0.9937978, 0.9858117,
     # 0.9770346, 0.9724492, 0.9656686, 0.9606671, 0.9525813, 0.9488522,
     # 0.9417335, 0.9399430, 0.9323438, 0.9281269, 0.9244171, 0.9237418,
     # 0.9174354, 0.9177181, 0.9120840, 0.9047825, 0.9065579, 0.9034142,
     # 0.8992961, 0.9011586, 0.9036470, 0.8954964, 0.8898208, 0.8911991,
     # 0.8854824, 0.8888282, 0.8868449, 0.9004632, 0.8981572, 0.9096183,
     # 0.9046990, 1.7454215, 0.0000000, 0.0000000, 0.0000000, 0.0000000,
     # 0.0000000, 0.0000000, 0.0000000, 0.0000000/

      if (E.lt.0.6) then
         fit=0.
      elseif(E.lt.1.777) then
         do i=98,1,-1
            if (arg(i).le.E) goto 100
         enddo
 100     fit=val(i)+(val(i+1)-val(i))/(arg(i+1)-arg(i))*(E-arg(i))
      else
         fit=0.
      endif
      fit_2=fit
      END
