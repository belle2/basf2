      FUNCTION xmatel(p1,q1,p2,q2,pk)
*     *******************************
!----------------------------------------------------------------------!
! This function provides a value of the matrix element squared for     !
! Bhabha scattering including single hard photon radiation.            !
!                                                                      !
!         e+(p1) + e-(q1) ---> e+(p2) + e-(q2) + gamma(pk)             !
!                                                                      !
! where p1, q1, p2, q2, pk  are 4-momenta of corresponding particles.  !
!                                                                      !
! INPUT:  p1(4),q1(4),p2(4),q2(4),pk(4) - 4-momenta of the particles,  !
!                                       all FERMIONS must be MASSIVE!  !
!                                                                      !
! Note: Vacuum polarization corrections are included in the above      !
!       matrix element as Dyson-type factors at the level of the       !
!       helicity amplitudes calculation.                               !
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                 Knoxville, May 1995      !
! Last update: 07.02.1996      by: W. P.                               !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      REAL*8 p1(4) ,q1(4) ,p2(4) ,q2(4) ,pk(4)
      REAL*8 p1r(4),q1r(4),p2r(4),q2r(4),pkr(4)
      COMPLEX*16 amp(12) 
!
! Rescaling 4-momenta of all particles for the massless limit
      pmod = SQRT(p2(1)**2 +p2(2)**2 +p2(3)**2)
      qmod = SQRT(q2(1)**2 +q2(2)**2 +q2(3)**2) 
      refa = ( pmod + qmod + pk(4) )/( p1(4) + q1(4) )
      DO i=1,4
         p1r(i) = p1(i)
         q1r(i) = q1(i)
         p2r(i) = p2(i)/refa
         q2r(i) = q2(i)/refa
         pkr(i) = pk(i)/refa
      ENDDO
      p1r(3) = p1r(4)
      q1r(3) =-q1r(4)
      p2r(4) = pmod/refa
      q2r(4) = qmod/refa
! Matrix element squared for massless spinors
      CALL amphel(p1r,q1r,p2r,q2r,pkr,amp)
      suma = 0
      DO i=1,12
         suma = suma + amp(i)*DCONJG(amp(i))
      ENDDO
      xmem0 = suma/4
! Mass correction terms (here fermions must be massive!)
      xmemc = xmelmc(p1,q1,p2,q2,pk)
! Total matrix element squared
      xmatel = xmem0 + xmemc
      END

      SUBROUTINE amphel(p1,q1,p2,q2,pk,amp)
*     *************************************
!----------------------------------------------------------------------!
! This routine calculates helicity amplitude for the Bhabha scattering !
! process with sigle photon radiation:                                 !
!                                                                      !
!    e+(p1,l1) + e-(q1,l2) ---> e+(p2,l3) + e-(q2,l4) + gamma(pk,l5)   !
!                                                                      !
! where p1, q1, p2, q2, pk  are 4-momenta of corresponding particles,  !  
! while l1, l2, l3, l4, l5 are their helicities.                       !
! Note: Both gamma and Z exchange diagrams are included.               !
!                                                                      !
! INPUT:  p1(4),q1(4),p2(4),q2(4),pk(4) - 4-momenta of the particles,  !
!                                      all PARTICLES must be MASSLESS! !
! OUTPUT: amp(12) - complex-type array of 12 helicity amplitudes,      !
!                   amp(i) = M(l1,l2,l3,l4,l5);                        !
!                                                                      !
!          amp( 1) = M(+++++),     amp( 2) = M(++++-)                  !
!          amp( 3) = M(----+),     amp( 4) = M(-----)                  !
!          amp( 5) = M(-++-+),     amp( 6) = M(-++--)                  !
!          amp( 7) = M(+--++),     amp( 8) = M(+--+-)                  !
!          amp( 9) = M(+-+-+),     amp(10) = M(+-+--)                  !
!          amp(11) = M(-+-++),     amp(12) = M(-+-+-)                  !
!                                                                      !
! Note: Vacuum polarization corrections are included in the above      !
!       helicity aplitudes as Dyson-type factors.                      ! 
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                 Knoxville, May 1995      !
! Last update: 19.01.1999      by: W. P.                               !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      PARAMETER( ALFPI=  1D0/PI/ALFINV ,ALFA=1D0/ALFINV)
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      SAVE   / BHPAR3 /, / BHPARZ /
      REAL*8 p1(4),q1(4),p2(4),q2(4),pk(4)
      COMPLEX*16 amp(12) 
      COMPLEX*16 Fspinp,Gspinp
      COMPLEX*16 F1 ,F2 ,F3 ,F4 ,F5 ,F6 ,G1 ,G2 ,G3 ,G4 
      COMPLEX*16 F1s,F2s,F3s,F4s,F5s,F6s,G1s,G2s,G3s,G4s
      COMPLEX*16 c,Rs,gs,zs,gs1,zs1
      SAVE e,c,ale,ari,alar,ale2,ari2,ifset
      DATA ifset /0/
! Statement functions for propagator factors
      Rs(a,gs,zs) = gs + a*zs
      Rt(a,gt,zt) = gt + a*zt
!
! Set up some constants
      IF (ifset.EQ.0) THEN
        ifset = 1
! Electric charge
        e = SQRT(4*PI*ALFA)
        c = DCMPLX(0d0,2*DSQRT(2d0)*e**3)
! Left and right coupling constant for Z exchange
        ale  = GV + GA
        ari  = GV - GA
        alar = ale*ari
        ale2 = ale**2
        ari2 = ari**2
      ENDIF
! Mandelstam variables
      s  = 2*( p1(4)*q1(4)-p1(3)*q1(3)-p1(2)*q1(2)-p1(1)*q1(1) )
      s1 = 2*( p2(4)*q2(4)-p2(3)*q2(3)-p2(2)*q2(2)-p2(1)*q2(1) )
      tp =-2*( p1(4)*p2(4)-p1(3)*p2(3)-p1(2)*p2(2)-p1(1)*p2(1) )
      tq =-2*( q1(4)*q2(4)-q1(3)*q2(3)-q1(2)*q2(2)-q1(1)*q2(1) )
      u  =-2*( p1(4)*q2(4)-p1(3)*q2(3)-p1(2)*q2(2)-p1(1)*q2(1) )
      u1 =-2*( q1(4)*p2(4)-q1(3)*p2(3)-q1(2)*p2(2)-q1(1)*p2(1) )
! gamma and Z propagator factors
      CALL profas(s ,gs ,zs )
      CALL profas(s1,gs1,zs1)
      CALL profat(tp,gtp,ztp)
      CALL profat(tq,gtq,ztq)
! Functions of spinor products
      F1 = Fspinp(p1,q1)
      F2 = Fspinp(p2,q2)
      F3 = Fspinp(p1,p2)
      F4 = Fspinp(q1,q2)
      F5 = Fspinp(p1,q2)
      F6 = Fspinp(q1,p2)
      G1 = Gspinp(p1,p2,q1,q2,pk)/tp
      G2 = Gspinp(q2,q1,p1,p2,pk)/tq
      G3 = Gspinp(q2,p2,p1,q1,pk)/s1
      G4 = Gspinp(p1,q1,p2,q2,pk)/s
      F1s= DCONJG(F1)
      F2s= DCONJG(F2)
      F3s= DCONJG(F3)
      F4s= DCONJG(F4)
      F5s= DCONJG(F5)
      F6s= DCONJG(F6)
      G1s= DCONJG(G1)
      G2s= DCONJG(G2)
      G3s= DCONJG(G3)
      G4s= DCONJG(G4)
! Helicity amplitudes
      amp( 1) = -c*s *F1 *( Rt(alar,gtp,ztp)*G1  +Rt(alar,gtq,ztq)*G2  )
      amp( 2) = -c*s1*F2s*( Rt(alar,gtp,ztp)*G1s +Rt(alar,gtq,ztq)*G2s )  
      amp( 3) = -c*s1*F2 *( Rt(alar,gtp,ztp)*G1  +Rt(alar,gtq,ztq)*G2  )  
      amp( 4) = -c*s *F1s*( Rt(alar,gtp,ztp)*G1s +Rt(alar,gtq,ztq)*G2s )
      amp( 5) = -c*tq*F4 *( Rs(alar,gs1,zs1)*G3  +Rs(alar,gs ,zs )*G4  )
      amp( 6) = -c*tp*F3s*( Rs(alar,gs1,zs1)*G3s +Rs(alar,gs ,zs )*G4s )  
      amp( 7) = -c*tp*F3 *( Rs(alar,gs1,zs1)*G3  +Rs(alar,gs ,zs )*G4  )
      amp( 8) = -c*tq*F4s*( Rs(alar,gs1,zs1)*G3s +Rs(alar,gs ,zs )*G4s ) 
      amp( 9) =  c*u *F5 *( Rt(ale2,gtp,ztp)*G1  +Rt(ale2,gtq,ztq)*G2   
     $                     +Rs(ale2,gs1,zs1)*G3  +Rs(ale2,gs ,zs )*G4  )
      amp(10) =  c*u1*F6s*( Rt(ale2,gtp,ztp)*G1s +Rt(ale2,gtq,ztq)*G2s 
     $                     +Rs(ale2,gs1,zs1)*G3s +Rs(ale2,gs ,zs )*G4s )
      amp(11) =  c*u1*F6 *( Rt(ari2,gtp,ztp)*G1  +Rt(ari2,gtq,ztq)*G2  
     $                     +Rs(ari2,gs1,zs1)*G3  +Rs(ari2,gs ,zs )*G4  )
      amp(12) =  c*u *F5s*( Rt(ari2,gtp,ztp)*G1s +Rt(ari2,gtq,ztq)*G2s 
     $                     +Rs(ari2,gs1,zs1)*G3s +Rs(ari2,gs ,zs )*G4s )
      END

      SUBROUTINE profas(s,gs,zs)
*     **************************
!----------------------------------------------------------------------!
! This subroutine privides values of gamma and Z propagator factors    ! 
! (i.e. s*propagator) for the s-channel exchange.                      !
! Various forms depending on the input parameters KeyEWC and KeyLib.   !
!----------------------------------------------------------------------!
! Written by: Wielaw Placzek                    Knoxville, Sep. 1995   !
! Last update: 25.09.1996         by: W. P.                            !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      COMMON / INOUT  / NINP,NOUT
      SAVE   / BHPAR3 /, / BHPARZ /, / INOUT  /
      COMPLEX*16 gs,zs,ags,azs
      REAL*8 IMSIGG
!
      KeyEWC = MOD(KeyRad,10000)/1000
      KeyLib = MOD(KeyRad,1000)/100
      KeyPia = MOD(KeyRad,10)
! Vacuum polarization

cc      s = 1d4

      CALL vacpol(KeyPia,s,SINW2,RePis,dRePis)
      IF (KeyEWC.EQ.0) THEN
        gs = 1/(1+RePis)

cc        print*,' s, gs = ',s, gs

! Constant Z width
        zs = s/DCMPLX(s-AMAZ**2,GAMMZ*AMAZ) /(1+RePis)
! Running Z width
cc          zs = s/DCMPLX(s-AMAZ**2,s*GAMMZ/AMAZ) /(1+RePis)
      ELSEIF (KeyEWC.EQ.1) THEN
        IF (KeyLib.eq.2) THEN
! Propagators from ALIBABA
          CALL sprogz(s,ags,azs)
!WP          gs = s*ags
! Photon propagator: real part from vacpol, imaginary part from ALIBABA 
          gsRe = 1 + RePis
          gsIm = IMSIGG(s)/s
          gs = 1/DCMPLX(gsRe,gsIm)

c          print*,' gsRe,gsIm=', gsRe,gsIm
c          print*,' azs =', azs
! Z propagator (from ALIBABA)
          zs = s*azs
        ELSEIF (KeyLib.EQ.1) THEN
          gs = 1/(1+RePis)
! Running Z width
          zs = s/DCMPLX(s-AMAZ**2,s*GAMMZ/AMAZ) /(1+RePis)
        ELSE
          WRITE(6   ,*)'>> profas: Wrong KeyLib !!!, KeyLib=',KeyLib
          WRITE(NOUT,*)'>> profas: Wrong KeyLib !!!, KeyLib=',KeyLib
          STOP
        ENDIF
      ELSE
        WRITE(6   ,*)'>> profas: Wrong KeyEWC !!!, KeyEWC=',KeyEWC
        WRITE(NOUT,*)'>> profas: Wrong KeyEWC !!!, KeyEWC=',KeyEWC
        STOP
      ENDIF
      END

      SUBROUTINE profat(t,gt,zt)
*     **************************
!----------------------------------------------------------------------!
! This subroutine privides values of gamma and Z propagator factors    ! 
! (i.e. t*propagator) for the t-channel exchange.                      !
! Various forms depending on the input parameters KeyEWC and KeyLib.   !
!----------------------------------------------------------------------!
! Written by: Wielaw Placzek                    Knoxville, Sep. 1995   !
! Last update: 25.09.1996         by: W. P.                            !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      COMMON / INOUT  / NINP,NOUT
      SAVE   / BHPAR3 /, / BHPARZ /, / INOUT  /
!
      KeyEWC = MOD(KeyRad,10000)/1000
      KeyLib = MOD(KeyRad,1000)/100
      KeyPia = MOD(KeyRad,10)
! Vacuum polarization
cc      t = -1d2
      CALL vacpol(KeyPia,t,SINW2,RePit,dRePit)
      IF (KeyEWC.EQ.0) THEN
        gt = 1/(1+RePit)
        zt = t/(t-AMAZ**2) /(1+RePit)

cc        print*,' t, gt = ',t, gt

      ELSEIF (KeyEWC.EQ.1) THEN 
        IF (KeyLib.EQ.2) THEN
! Propagators from ALIBABA
          CALL tprogz(t,agt,azt)
!WP          gt  = t*agt
          gt  = 1/(1+RePit)
          zt  = t*azt 
        ELSEIF (KeyLib.EQ.1) THEN
          gt = 1/(1+RePit)
          zt = t/(t-AMAZ**2) /(1+RePit)
        ELSE
          WRITE(6   ,*)'>> profat: Wrong KeyLib !!!, KeyLib=',KeyLib
          WRITE(NOUT,*)'>> profat: Wrong KeyLib !!!, KeyLib=',KeyLib
          STOP
        ENDIF
      ELSE
        WRITE(6   ,*)'>> profat: Wrong KeyEWC !!!, KeyEWC=',KeyEWC
        WRITE(NOUT,*)'>> profat: Wrong KeyEWC !!!, KeyEWC=',KeyEWC
        STOP
      ENDIF
      END

      COMPLEX*16 FUNCTION Fspinp(p,q)
*     *******************************
!----------------------------------------------------------------------!
! This functions gives the result of the following expression          ! 
! of spinor products:                                                  !
!                                <pq>                                  !
!                       F(p,q) = -----                                 !
!                                <pq>*                                 !
! where the spinor product <pq> is defined in Ref. Zhan Xu et al.,     !
! Nucl. Phys. B291 (1987) 392; <pq>* is a complex conjugate of <pq>.   !
!                                                                      !
! INPUT: p(4), q(4) - two 4-vectors                                    !  
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                     Knoxville, May 1995  !
! Last update: 03.05.1995       by: W. P.                              !
!----------------------------------------------------------------------!
      REAL*8 p(4),q(4)
      COMPLEX*16 spipro,pxq
! Spinor product
      pxq = spipro(p,q)
      Fspinp = pxq/DCONJG(pxq)
      END

      COMPLEX*16 FUNCTION Gspinp(p,q,r,s,k)
*     *************************************
!----------------------------------------------------------------------!
! This functions gives the result of the following expression          ! 
! of spinor products:                                                  !
!                                  <pq>*                               !
!                       F(p,q) = --------                              !
!                                <rk><ks>                              !
! where the spinor products <..> are defined in Ref. Zhan Xu et al.,   !
! Nucl. Phys. B291 (1987) 392; <pq>* is a complex conjugate of <pq>.   !
!                                                                      !
! INPUT: p(4), q(4), r(4), s(4), k(4) - five 4-vectors                 !  
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                     Knoxville, May 1995  !
! Last update: 03.05.1995       by: W. P.                              !
!----------------------------------------------------------------------!
      REAL*8 p(4),q(4),r(4),s(4),k(4)
      COMPLEX*16 spipro,pxq,rxk,kxs
! Spinor products
      pxq = spipro(p,q)
      rxk = spipro(r,k)
      kxs = spipro(k,s)
      Gspinp = DCONJG(pxq)/(rxk*kxs)
      END

      COMPLEX*16 FUNCTION spipro(p,q)
*     *******************************
!----------------------------------------------------------------------!
! This functions gives the value of the spinor product <pq> = <p-|q+>  ! 
! as defined in Ref. Zhan Xu et al., Nucl. Phys. B291 (1987) 392.      !
! INPUT: p(4), q(4) - two 4-vectors                                    !  
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                     Knoxville, May 1995  !
! Last update: 08.05.1995       by: W. P.                              !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      REAL*8 p(4),q(4)
      COMPLEX*16 pt,qt,eip,eiq
!
      pp = p(4)+p(3)
      pm = p(4)-p(3)
      pta= SQRT(pp*pm)
      IF (pta.gt.0) THEN
         pt = DCMPLX(p(1),p(2))
         eip = pt/pta
      ELSE
         eip = 1
      ENDIF
      qp = q(4)+q(3)
      qm = q(4)-q(3)
      qta= SQRT(qp*qm)
      IF (qta.gt.0) THEN
         qt = DCMPLX(q(1),q(2)) 
         eiq = qt/qta
      ELSE
         eiq = 1
      ENDIF
! Spinor product
      spipro = SQRT(pm*qp)*eip - SQRT(pp*qm)*eiq 
      END

      FUNCTION xmelmc(p1,q1,p2,q2,pk)
*     *******************************
!----------------------------------------------------------------------!
! This function provides a value of the finite fermion mass correction !
! to the matrix element for Bhabha scattering including hard photon    !
! radiation:                                                           !
!                                                                      !
!         e+(p1) + e-(q1) ---> e+(p2) + e-(q2) + gamma(pk)             !
!                                                                      !
! where p1, q1, p2, q2, pk  are 4-momenta of corresponding particles.  !
!                                                                      !
! INPUT:  p1(4),q1(4),p2(4),q2(4),pk(4) - 4-momenta of the particles,  !
!                                       all FERMIONS must be MASSIVE!  !
!                                                                      !
! Note: Vacuum polarization corrections are included in the above      !
!       matrix element as Dyson-type factors.                          !
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                 Knoxville, May 1995      !
! Last update: 08.05.1995      by: W. P.                               !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      PARAMETER( ALFA=1D0/ALFINV)
      COMMON / BHPAR2 / CMSENE,AMEL
      SAVE   / BHPAR2 /
      REAL*8 p1(4),q1(4),p2(4),q2(4),pk(4)
!
      ame = AMEL
! Electric charge squared
      e2 = 4*PI*ALFA
! Mandelstam variables
      s  = (p1(4)+q1(4))**2 - (p1(3)+q1(3))**2 
     $    -(p1(2)+q1(2))**2 - (p1(1)+q1(1))**2
      s1 = (p2(4)+q2(4))**2 - (p2(3)+q2(3))**2 
     $    -(p2(2)+q2(2))**2 - (p2(1)+q2(1))**2
      tp = (p1(4)-p2(4))**2 - (p1(3)-p2(3))**2 
     $    -(p1(2)-p2(2))**2 - (p1(1)-p2(1))**2
      tq = (q1(4)-q2(4))**2 - (q1(3)-q2(3))**2 
     $    -(q1(2)-q2(2))**2 - (q1(1)-q2(1))**2
      u  = (p1(4)-q2(4))**2 - (p1(3)-q2(3))**2 
     $    -(p1(2)-q2(2))**2 - (p1(1)-q2(1))**2
      u1 = (q1(4)-p2(4))**2 - (q1(3)-p2(3))**2 
     $    -(q1(2)-p2(2))**2 - (q1(1)-p2(1))**2
! Scalar products of photon 4-momentum with other 4-momenta
      p1k = p1(4)*pk(4)-p1(3)*pk(3)-p1(2)*pk(2)-p1(1)*pk(1)
      q1k = q1(4)*pk(4)-q1(3)*pk(3)-q1(2)*pk(2)-q1(1)*pk(1)
      p2k = p2(4)*pk(4)-p2(3)*pk(3)-p2(2)*pk(2)-p2(1)*pk(1)
      q2k = q2(4)*pk(4)-q2(3)*pk(3)-q2(2)*pk(2)-q2(1)*pk(1)
! Mass correction terms
      xmelmc = -e2*( (ame/p1k)**2 *xmate0(s1,tq,u1)
     $              +(ame/q1k)**2 *xmate0(s1,tp,u )
     $              +(ame/p2k)**2 *xmate0(s ,tq,u )
     $              +(ame/q2k)**2 *xmate0(s ,tp,u1) )
      END

      FUNCTION xmate0(s,t,u)
*     **********************
!----------------------------------------------------------------------!
! This function provides a value of the lowest order matrix element    !
! squared for Bhabha scattering.                                       !
! INPUT:  s, t, u - Mandelstam varriables                              !
! Note: Vacuum polarization corrections are included in the above      !
!       matrix element as Dyson-type factors.                          ! 
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                 Knoxville, May 1995      !
! Last update: 08.02.1996      by: W. P.                               !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      PARAMETER( ALFA=1D0/ALFINV)
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      SAVE   / BHPAR3 /, / BHPARZ /
      COMPLEX*16 Rs,Rsl2,Rsl2s,Rsr2,Rsr2s,Rslr,Rslrs,gs,zs
      SAVE e4,ale,ari,alar,ale2,ari2,ifset,KeyCha
      DATA ifset /0/
! Statement functions for propagator factors
      Rs(a,gs,zs) = gs + a*zs
      Rt(a,gt,zt) = gt + a*zt
!
! Set up some constants
      IF (ifset.EQ.0) THEN
        ifset = 1
! Electric charge: e**4
        e4= (4*PI*ALFA)**2
! left and right coupling constant for Z exchange
        ale  = GV + GA
        ari  = GV - GA
        alar = ale*ari
        ale2 = ale**2
        ari2 = ari**2
! Channel switch (s+t, s-only or t-only)
        KeyCha = MOD(KeyOpt,1000)/100   
      ENDIF
! gamma and Z propagator factors
      CALL profas(s,gs,zs)
      CALL profat(t,gt,zt)
! Combinations of the above factors
      Rsl2 = Rs(ale2,gs,zs)
      Rsl2s= DCONJG(Rsl2)
      Rsr2 = Rs(ari2,gs,zs)
      Rsr2s= DCONJG(Rsr2)
      Rslr = Rs(alar,gs,zs)
      Rslrs= DCONJG(Rslr)
      Rtl2 = Rt(ale2,gt,zt)
      Rtr2 = Rt(ari2,gt,zt)
      Rtlr = Rt(alar,gt,zt)
! Various (real) combinations of the above factors
      Asl2 = Rsl2*Rsl2s
      Asr2 = Rsr2*Rsr2s 
      Aslr = Rslr*Rslrs 
      Atl2 = Rtl2*Rtl2  
      Atr2 = Rtr2*Rtr2     
      Atlr = Rtlr*Rtlr  
      Astl = DBLE(Rsl2s)*Rtl2
      Astr = DBLE(Rsr2s)*Rtr2
!... s-channel off
      IF (KeyCha.EQ.2) THEN
        Asl2 = 0
        Asr2 = 0
        Aslr = 0
        Astl = 0
        Astr = 0
      ENDIF
!... t-channel off
      IF (KeyCha.EQ.1) THEN
        Atl2 = 0
        Atr2 = 0
        Atlr = 0
        Astl = 0
        Astr = 0
      ENDIF
! Matrix element
      xmate0 = e4*( ( (Asl2+Asr2)*u**2 + 2*Aslr*t**2 )/s**2
     $            + ( (Atl2+Atr2)*u**2 + 2*Atlr*s**2 )/t**2
     $            + 2*(Astl+Astr)*u**2/s/t )
      END

      FUNCTION xmecal(p1,q1,p2,q2,pk)
*     *******************************
!----------------------------------------------------------------------!
! This function provides a value of the matrix element squared for     !
! Bhabha scattering including single hard photon radiation as given    !
! in Ref. (CALKUL) F.A. Berends et al., Nucl. Phys. B206 (1982) 61.    !
!                                                                      !
!         e+(p1) + e-(q1) ---> e+(p2) + e-(q2) + gamma(pk)             !
!                                                                      !
! where p1, q1, p2, q2, pk  are 4-momenta of corresponding particles.  !
!                                                                      !
! INPUT:  p1(4),q1(4),p2(4),q2(4),pk(4) - 4-momenta of the particles,  !
!                                       all FERMIONS must be MASSIVE!  !
!                                                                      !
! Note: Vacuum polarization corrections are included in the above      !
!       matrix element as Dyson-type factors.                          !
!----------------------------------------------------------------------!
! Written by: Wieslaw Placzek                 Knoxville, May 1995      !
! Last update: 08.02.1996      by: W. P.                               !
!----------------------------------------------------------------------!
      IMPLICIT REAL*8 (a-h,o-z)
      PARAMETER( PI = 3.1415926535897932D0, ALFINV = 137.0359895D0)
      PARAMETER(ALFA=1D0/ALFINV)
      COMMON / BHPAR3 / KEYRAD,KEYOPT
      COMMON / BHPARZ / AMAZ,GAMMZ,SINW2,GV,GA
      SAVE   / BHPAR3 /, / BHPARZ /
      REAL*8 p1(4) ,q1(4) ,p2(4) ,q2(4) ,pk(4)
      REAL*8 p1r(4),q1r(4),p2r(4),q2r(4),pkr(4)
      COMPLEX*16 Asa2,Asa2c,Asb2,Asb2c,Asab,Asabc
      COMPLEX*16 As1a2,As1a2c,As1b2,As1b2c,As1ab,As1abc
      COMPLEX*16 As,gs,zs,gs1,zs1
      SAVE e6,a,b,a2,b2,ab,ifset,KeyCha
      DATA ifset /0/
! Statement function for propagator factors
      As(a,gs,zs) = gs + 4*a*zs
      At(a,gt,zt) = gt + 4*a*zt
!
! Set up some constants
      IF (ifset.eq.0) THEN
        ifset = 1
! Electric charge: e**6
        e6 = (4*PI*ALFA)**3
! Z coupling constans
        a = (GV+GA)/2
        b = (GV-GA)/2
        a2= a*a
        b2= b*b
        ab= a*b
! Channel switch (s+t, s-only or t-only)
        KeyCha = MOD(KeyOpt,1000)/100   
      ENDIF
! Rescaling 4-momenta of all particles for the massless limit (tests)
cc      pmod = DSQRT(p2(1)**2+p2(2)**2+p2(3)**2)
cc      qmod = DSQRT(q2(1)**2+q2(2)**2+q2(3)**2) 
cc      refa = ( pmod + qmod + pk(4) )/( p1(4) + q1(4) )
      refa = 1
      DO i=1,4
         p1r(i) = p1(i)
         q1r(i) = q1(i)
         p2r(i) = p2(i)/refa
         q2r(i) = q2(i)/refa
         pkr(i) = pk(i)/refa
      ENDDO
cc      p1r(3) = p1r(4)
cc      q1r(3) =-q1r(4)
cc      p2r(4) = pmod/refa
cc      q2r(4) = qmod/refa
! Mandelstam variables
      s  = 2*( p1r(4)*q1r(4)-p1r(3)*q1r(3)-p1r(2)*q1r(2)-p1r(1)*q1r(1) )
      s1 = 2*( p2r(4)*q2r(4)-p2r(3)*q2r(3)-p2r(2)*q2r(2)-p2r(1)*q2r(1) )
      tp =-2*( p1r(4)*p2r(4)-p1r(3)*p2r(3)-p1r(2)*p2r(2)-p1r(1)*p2r(1) )
      tq =-2*( q1r(4)*q2r(4)-q1r(3)*q2r(3)-q1r(2)*q2r(2)-q1r(1)*q2r(1) )
      u  =-2*( p1r(4)*q2r(4)-p1r(3)*q2r(3)-p1r(2)*q2r(2)-p1r(1)*q2r(1) )
      u1 =-2*( q1r(4)*p2r(4)-q1r(3)*p2r(3)-q1r(2)*p2r(2)-q1r(1)*p2r(1) )
! Scalar products of photon 4-momentum with other 4-momenta
      p1k = p1r(4)*pkr(4)-p1r(3)*pkr(3)-p1r(2)*pkr(2)-p1r(1)*pkr(1)
      q1k = q1r(4)*pkr(4)-q1r(3)*pkr(3)-q1r(2)*pkr(2)-q1r(1)*pkr(1)
      p2k = p2r(4)*pkr(4)-p2r(3)*pkr(3)-p2r(2)*pkr(2)-p2r(1)*pkr(1)
      q2k = q2r(4)*pkr(4)-q2r(3)*pkr(3)-q2r(2)*pkr(2)-q2r(1)*pkr(1)
! Propagator factors
! ... if s-channel ON
      IF (KeyCha.NE.2) THEN
        CALL profas(s ,gs ,zs )
        CALL profas(s1,gs1,zs1)
        Asa2  = As(a2,gs,zs) 
        Asa2c = DCONJG(Asa2)
        Asb2  = As(b2,gs,zs) 
        Asb2c = DCONJG(Asb2)
        Asab  = As(ab,gs,zs) 
        Asabc = DCONJG(Asab)
        As1a2 = As(a2,gs1,zs1) 
        As1a2c= DCONJG(As1a2)
        As1b2 = As(b2,gs1,zs1) 
        As1b2c= DCONJG(As1b2)
        As1ab = As(ab,gs1,zs1) 
        As1abc= DCONJG(As1ab)
      ELSE
! ... s-channel OFF
        Asa2  = 0
        Asa2c = 0
        Asb2  = 0
        Asb2c = 0
        Asab  = 0
        Asabc = 0
        As1a2 = 0
        As1a2c= 0
        As1b2 = 0
        As1b2c= 0
        As1ab = 0
        As1abc= 0
      ENDIF
! ... if t-channel ON
      IF (KeyCha.NE.1) THEN
        CALL profat(tp,gtp,ztp)
        CALL profat(tq,gtq,ztq)
        Atpa2 = At(a2,gtp,ztp) 
        Atpb2 = At(b2,gtp,ztp) 
        Atpab = At(ab,gtp,ztp) 
        Atqa2 = At(a2,gtq,ztq) 
        Atqb2 = At(b2,gtq,ztq)
        Atqab = At(ab,gtq,ztq) 
      ELSE
! ... t-channel OFF 
        Atpa2 = 0
        Atpb2 = 0
        Atpab = 0
        Atqa2 = 0
        Atqb2 = 0
        Atqab = 0
      ENDIF
! Radiation factors
      rf1 = s /(p1k*q1k)
      rf2 = s1/(p2k*q2k)
      rf3 =-tp/(p1k*p2k)
      rf4 =-tq/(q1k*q2k)
      rf5 = u /(p1k*q2k)
      rf6 = u1/(q1k*p2k)
      W1 = rf1
      W2 = rf2
      W3 = rf3 + rf4 + rf5 + rf6
      W4 = rf3
      W5 = rf4
      W6 = rf1 + rf2 + rf5 + rf6
! Some traces
      Tr1 = 2*(s *p2k +u1*p1k -tp*q1k)
      Tr2 = 2*(s *q2k +u *q1k -tq*p1k)
      Tr3 = 2*(s1*p1k +u *p2k -tp*q2k)
      Tr4 = 2*(s1*q1k +u1*q2k -tq*p2k)
      epspq = s/2 *( p2(2)*q2(1) - p2(1)*q2(2) )
! Matrix element squared for massless spinors
      xm1 = ( W1*As1ab*As1abc +W2*Asab *Asabc +W3*DBLE(Asab *As1abc) )
     $     *(tp**2 + tq**2)/(s *s1)
      xm2 = ( W4*Atqab*Atqab  +W5*Atpab*Atpab +W6*     Atpab*Atqab   )
     $     *(s**2  + s1**2)/(tp*tq) 
      xm3 = ( W1*(As1a2*As1a2c + As1b2*As1b2c) 
     $       +W2*(Asa2 *Asa2c  + Asb2 *Asb2c )
     $       +W3*DBLE(Asa2 *As1a2c + Asb2 *As1b2c) )
     $     *(u**2 + u1**2)/(2*s *s1)
      xm4 = ( W4*(Atqa2*Atqa2  + Atqb2*Atqb2 ) 
     $       +W5*(Atpa2*Atpa2  + Atpb2*Atpb2 )
     $       +W6*    (Atpa2*Atqa2  + Atpb2*Atqb2 ) )
     $     *(u**2 + u1**2)/(2*tp*tq)
      xm5 = ( p1k/(s *tp)*Tr1*DBLE(Asa2 *Atpa2 + Asb2 *Atpb2) 
     $       +q1k/(s *tq)*Tr2*DBLE(Asa2 *Atqa2 + Asb2 *Atqb2) 
     $       +p2k/(s1*tp)*Tr3*DBLE(As1a2*Atpa2 + As1b2*Atpb2) 
     $       +q2k/(s1*tq)*Tr4*DBLE(As1a2*Atqa2 + As1b2*Atqb2) )
     $     *(u**2 + u1**2)/(4*p1k*q1k*p2k*q2k)
      xm6 =-( (s-s1)/(s*s1)*DIMAG(Asa2*As1a2c - Asb2*As1b2c) 
     $       +2*p1k/(s *tp)*DIMAG(Asa2 *Atpa2 - Asb2 *Atpb2) 
     $       +2*q1k/(s *tq)*DIMAG(Asa2 *Atqa2 - Asb2 *Atqb2) 
     $       -2*p2k/(s1*tp)*DIMAG(As1a2*Atpa2 - As1b2*Atpb2) 
     $       -2*q2k/(s1*tq)*DIMAG(As1a2*Atqa2 - As1b2*Atqb2) )
     $     *(u**2 - u1**2)*epspq/(2*p1k*q1k*p2k*q2k)
      xmem0 = e6 *(xm1+xm2+xm3+xm4+xm5+xm6)
! Mass correction terms (here fermions must be massive!)
      xmemc = xmelmc(p1,q1,p2,q2,pk)
! Total matrix element squared
      xmecal = xmem0 + xmemc
      END
