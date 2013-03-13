*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//                                                                                         //
*//                               Pseudo-CLASS  BVR                                         //
*//                                                                                         //
*//             B-functions Real and Virtual and YFS form-factors                           //
*//                                                                                         //
*//                                                                                         //
*//    WARNIG:  !!!!!!!!!!!!!!!!!!!!!!![[[[[[[[]]]]]]]!!!!!!!!!!!!!!!                       //
*//    Function BVR_SBvirt modified temporarily to avoid problem with pi**2/beta            //
*//    WARNIG:  !!!!!!!!!!!!!!!!!!!!!!![[[[[[[[]]]]]]]!!!!!!!!!!!!!!!                       //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////

      SUBROUTINE BVR_MakeF2Pair(svar,amex,Alfpi,Q,F2Pair)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION Alfpi,Q,svar,amex,F2Pair
      INTEGER          iKF,NCf
      DOUBLE PRECISION Masf, Qf,T3f, Ene,Sum
      DOUBLE PRECISION BVR_F2PairS, F2PairS
      DOUBLE PRECISION Mquark(6)
*                       d       u       s       c       b       t
      DATA Mquark / 0.20d0, 0.20d0, 0.20d0, 1.30d0, 5.00d0, 175.0d0 /
cc      DATA Mquark / 0.40d0, 0.40d0, 0.40d0, 1.30d0, 5.00d0, 175.0d0 /
      SAVE Mquark
*-------------------------------
      INTEGER          icont
      DATA icont /0/
      SAVE icont
      icont=icont+1
*-------------------------------
      Ene =SQRT(svar)
      Sum = 0d0
      DO iKF=1,16
         IF(iKF.LE.6 .OR. iKF.GE.11) THEN
            CALL BornV_GetParticle(iKF, Masf, Qf,T3f,NCf)
            IF(iKF.LE.6) Masf=Mquark(iKF)                       ! replace with dispersive mass
            IF( (Ene.GT.2*Masf) .AND. (ABS(Qf).GT.1d-6) ) THEN  ! only charged above threshols
               F2PairS = BVR_F2PairS(svar,Masf,amex)
               F2PairS = F2PairS *NCf*Qf**2 *Alfpi
               Sum = Sum + F2PairS
****           IF(icont.LT.100) write(*,*) '===, iKF, F2PairS =',iKF, F2PairS!
            ENDIF
         ENDIF
      ENDDO
      F2Pair = Sum *Q**2 *Alfpi
****  IF(icont.LT.100) write(*,*) '--- BVR_MakeF2Pair: F2Pair=',F2Pair,' Ene=', Ene 
      END

      DOUBLE PRECISION FUNCTION BVR_F2PairS(svar,ampair,amel)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*// s-channel one-pair virtual correction to ee vertex                                      //
*//\bibitem{BURGERS}G.J.H. Burgers, {\it Phys. Lett.}, {\bf B164}, (1985), {167}.           //
*//\bibitem{KNIEHL}B. A. Kniehl,    {\it Phys. Lett.}, {\bf B237}, (1990), {127}.           //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION svar,ampair,amel
      DOUBLE PRECISION slog,const
*
      slog=DLOG(svar/ampair**2)
      IF ( ABS(ampair/amel-1) .LT. 0.01d0 ) THEN
        const=383d0/108d0 -11d0/36d0*m_pi**2 
      ELSE
        const=-1/3d0*m_zeta3 -19d0/18d0*m_pi**2/6d0 +3355d0/1296d0
      ENDIF
      BVR_F2PairS=
     $   -1d0/36d0                     *slog**3 
     $   +19d0/72d0                    *slog**2 
     $   +(1d0/18d0*m_pi**2 -265d0/216d0) *slog 
     $   +const
      END

      DOUBLE PRECISION FUNCTION BVR_F2PairT(tvar,ampair,amel)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//   NOT USED, kept for futute.                                                            //
*// t-channel one-pair virtual correction to ee vertex                                      //
*//\bibitem{BURGERS}G.J.H. Burgers, {\it Phys. Lett.}, {\bf B164}, (1985), {167}.           //
*//\bibitem{KNIEHL}B. A. Kniehl,    {\it Phys. Lett.}, {\bf B237}, (1990), {127}.           //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION tvar,ampair,amel
      DOUBLE PRECISION tlog,const
*
      tlog=DLOG(tvar/ampair**2)
      IF ( ABS(ampair/amel-1) .LT. 0.01d0 ) THEN
ccc      IF (ampair.EQ.amel) THEN
        const=383d0/108d0 -11d0/36d0*m_pi**2 
      ELSE
        const=-1d0/3d0*m_zeta3 -19d0/18d0*m_pi**2/6d0 +3355d0/1296d0
      ENDIF
      BVR_F2PairT=
     $   -1d0/36d0                     *tlog**3 
     $   +19d0/72d0                    *tlog**2 
     $   +(-1d0/36d0*m_pi**2 -265d0/216d0) *tlog 
     $   +const
     $   +19d0/72d0*m_pi**2
      END

      SUBROUTINE  BVR_MakeVini(Alfpi,Q,p1,m1,p2,m2,ph, V1,V2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Initial state virtual correction to single bremsstrahlung                             //
*//   IR subtracted, in small mass approximation.                                           //
*//   Second order LL  (+NLL???) is included                                                //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION   Alfpi,Q,m1,m2,p1(4),p2(4),ph(4)
      DOUBLE COMPLEX     V1,V2, cL, V2a
      DOUBLE COMPLEX     BVR_CnuA
      DOUBLE PRECISION   p1p2,Svar,r1,r2,zz
      DOUBLE PRECISION   BVR_Dilog
*
      p1p2  =  p1(4)*p2(4) -p1(3)*p2(3) -p1(2)*p2(2) -p1(1)*p2(1)
      Svar  = 2*p1p2 +m1**2+m2**2
      r1 = (p1(4)*ph(4)-p1(3)*ph(3)-p1(2)*ph(2)-p1(1)*ph(1))/p1p2
      r2 = (p2(4)*ph(4)-p2(3)*ph(3)-p2(2)*ph(2)-p2(1)*ph(1))/p1p2

      cL = BVR_CnuA(Svar,m1,m2)-1d0                 !! <-- this is just ln(s/m**2)-i*pi -1
***   cL = DCMPLX( DLOG(Svar/m1/m2)-1d0, -1d0 )

      V1 = (Alfpi*Q**2) *0.5d0*cL                   !! constant LL part

      V2a= (Alfpi*Q**2) *( -0.25d0*cL*DLOG((1d0-r1)*(1d0-r2)) ) !! LL formula averaged over r1,r2

      V2 = (Alfpi*Q**2) *0.5d0*(
     $     +DLOG(r1)*DLOG(1-r2)  +DLOG(r2)*DLOG(1-r1)           !! LL part
     $     +BVR_Dilog(r1)        +BVR_Dilog(r2)                 !! NLL this and all the rest
     $     -1d0/2*DLOG(1-r1)**2  -1d0/2*DLOG(1-r2)**2
     $     +3d0/2*DLOG(1-r1)     +3d0/2*DLOG(1-r2)
     $     +1d0/2*r1*(1-r1)/(1+(1-r1)**2)
     $     +1d0/2*r2*(1-r2)/(1+(1-r2)**2)
     $     )
      END

      SUBROUTINE  BVR_MakeVfin(Alfpi,Q,p3,m3,p4,m4,ph, V1,V2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Final state virtual correction to single bremsstrahlung                               //
*//   IR subtracted, in small mass approximation.                                           //
*//   Second order LL  (+NLL???) is included                                                //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION   Alfpi,Q,m3,m4,p3(4),p4(4),ph(4)
      DOUBLE COMPLEX     V1,V2, cL, V2a
      DOUBLE COMPLEX     BVR_CnuA
      DOUBLE PRECISION   p3p4,Svar,r1,r2,zz,s1,s2
      DOUBLE PRECISION   BVR_Dilog
*
      p3p4  =  p3(4)*p4(4) -p3(3)*p4(3) -p3(2)*p4(2) -p3(1)*p4(1)
      Svar  = 2*p3p4 +m3**2+m4**2
      s1 = (p3(4)*ph(4)-p3(3)*ph(3)-p3(2)*ph(2)-p3(1)*ph(1))/p3p4
      s2 = (p4(4)*ph(4)-p4(3)*ph(3)-p4(2)*ph(2)-p4(1)*ph(1))/p3p4
* normal definition as in O(alf1) single-photon case
      r1 = s1/( 1d0 +s1 +s2 )
      r2 = s2/( 1d0 +s1 +s2 )

      cL = BVR_CnuA(Svar,m3,m4)-1d0                 !! <-- this is just ln(s/m**2)-i*pi -1
***   cL = DCMPLX( DLOG(Svar/m3/m4)-1d0, -1d0 )

      V1 = (Alfpi*Q**2) *0.5d0*cL                   !! constant LL part

      V2= (Alfpi*Q**2) *( -0.25d0*cL*DLOG((1d0-r1)*(1d0-r2)) )  !! LL formula averaged over r1,r2
      V2= V2+ (Alfpi*Q**2) *( +0.50d0*cL*DLOG((1d0-r1)*(1d0-r2)) ) !! corr. due YFS formfactor

* this ISR formula is to be replaced
cc      V2 = (Alfpi*Q**2) *0.5d0*(
cc     $     +DLOG(r1)*DLOG(1-r2)  +DLOG(r2)*DLOG(1-r1)         !! LL part
cc     $     +BVR_Dilog(r1)        +BVR_Dilog(r2)               !! NLL this and all the rest
cc     $     -1d0/2*DLOG(1-r1)**2  -1d0/2*DLOG(1-r2)**2
cc     $     +3d0/2*DLOG(1-r1)     +3d0/2*DLOG(1-r2)
cc     $     +1d0/2*r1*(1-r1)/(1+(1-r1)**2)
cc     $     +1d0/2*r2*(1-r2)/(1+(1-r2)**2)
cc     $     )
cc      V2= V2-(Alfpi*Q**2) *( -0.50d0*cL*DLOG((1d0-r1)*(1d0-r2)) )

      END

      SUBROUTINE  BVR_MakeF1ini(Svar,Mas1,Mas2,Alfpi,Q,F1_1,F1_2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Initial state vertex correction, IR subtracted, in small mass approximation.          //
*//   Second order LL+NLL is included.                                                      //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION   Svar, Mas1,Mas2, Alfpi, Q
      DOUBLE COMPLEX     F1_1, F1_2, cL
      DOUBLE COMPLEX     BVR_CnuA
*
      cL = BVR_CnuA(Svar,Mas1,Mas2) - DCMPLX(1d0)         !!! <-- this is just ln(s/m**2)-i*pi -1
***   cL = DCMPLX( DLOG(Svar/Mas1/Mas2)-1d0, -1d0 )

      F1_1 = (Alfpi*Q**2)   *0.5d0*cL
      F1_2 = F1_1
     $     +(Alfpi*Q**2)**2 *(
     $              +cL**2/8d0 
     $              +cL*( 3d0/32 -3d0/4*m_zeta2 +3d0/2*m_zeta3 ) 
     $     )
*------------------------------------------------------------
* Subleading terms from Berends, Burgers, Van Neerveen
* Cooked up by S.J. long time ago (1989?)
*      dels2 =  charg2**2*alfpi**2  *0.5d0*bilg**2
*     $     +charg2**2*alfpi**2*(
*     $     -(13d0/16d0 +1.5d0*zet2 -3d0*zet3)*bilg
*     $     -16d0/5d0*zet2*zet2 +51d0/8d0*zet2 +13d0/4d0
*     $     -4.5d0*zet3 -6d0*zet2*log(2d0) )
*------------------------------------------------------------
      END

      SUBROUTINE  BVR_MakeF1fin(Svar,Mas1,Mas2,Alfpi,Q,F1_1,F1_2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Final state vertex correction, second order                                           //
*//   finite mass  only for first order part                                                //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION   Svar,Mas1,Mas2,Alfpi,Q
      DOUBLE COMPLEX     F1_1,F1_2,cL
      DOUBLE COMPLEX     BVR_CnuA
*
      cL = BVR_CnuA(Svar,Mas1,Mas2) - DCMPLX(1d0)       !!! <-- this is just ln(s/m**2)-i*pi -1
***   cL = DCMPLX( DLOG(Svar/Mas1/Mas2)-1d0, -1d0 )

      F1_1 = (Alfpi*Q**2)   *0.5d0*cL
      F1_2 = F1_1
     $     +(Alfpi*Q**2)**2 *(
     $              +cL**2/8d0 
     $              +cL*( 3d0/32 -3d0/4*m_zeta2 +3d0/2*m_zeta3 ) 
     $     )
      END

      DOUBLE COMPLEX  FUNCTION BVR_CnuA(Svar,Mas1,Mas2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Function nu*A(nu) Complex version appropriate for s and t-chanels.                    //
*//   No small mass approximation.                                                          //
*//                                                                                         //
*//       s-chanel:  Nu = (-s+m1**2+m2**2)/2 = -p1p2 < 0   p1 and p2 incoming or outgoing   //
*//       t-chanel:  Nu = (-t+m1**2+m2**2)/2 =  p1p2 > 0   p1 incoming p2 outgoing          //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      Svar,Mas1,Mas2
      DOUBLE COMPLEX        Eps,Nu,Mas12,xlam,z
      DOUBLE COMPLEX        BVR_CDLN
*----------
      Eps = DCMPLX( +1d0,0.d0)   ! plus was adjusted empiricaly
      IF( DABS(Mas1*Mas2) .LT. 1d-10 ) GOTO 900
      Mas12 = DCMPLX(Mas1*Mas2)
      Nu    = DCMPLX( (-Svar +Mas1**2+Mas2**2)/2d0 )
      xlam  = CDSQRT( ( Nu - Mas12)*( Nu + Mas12) )
* take care of numerical stability
      IF( svar .GT. 0d0) THEN
         z = Mas12/(Nu-xlam)
      ELSE
         z = (Nu + xlam)/Mas12
      ENDIF
      BVR_CnuA  = Nu/xlam *BVR_CDLN( z ,Eps)
      RETURN
 900  CONTINUE
      WRITE(*,*) '++++++ BVR_CnuA: STOP due to zero mass: ',Mas1,Mas2
      STOP
      END


      SUBROUTINE BVR_RBoxGG(MasPhot,s,t,u,ReBox,ImBox,ReIR,DelBox)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//   Box Gamma-Gamma, TEST version, no use of COMPLEX                                      //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      MasPhot,s,t,u,ReBox,ImBox,ReIR,DelBox
      DOUBLE PRECISION      ct,cp,cm,lcm,lcp
*------
      cm = ABS(t/s)
      cp = ABS(u/s)
      ct = cp-cm
      lcm = DLOG(cm)
      lcp = DLOG(cp)
      ReBox = DLOG(cm/cp)*DLOG(MasPhot**2/s) -ct/(2*cp*cp)*0.5d0*lcm**2  +lcm /(2*cp)
      ImBox = DLOG(cm/cp)*m_pi               -ct/(2*cp*cp)*m_pi *lcm     +m_pi/(2*cp)
      ReIR  = DLOG(cm/cp)*DLOG(MasPhot**2/DSQRT(u*t)) +DLOG(cm/cp)/2d0
      DelBox = 
     $     2*DLOG(cm/cp)*DLOG(MasPhot**2/s)
     $    +1/(1+ct*ct)*(2*cp*lcm - 2*cm*lcp -ct*lcm**2 -ct*lcp**2)
      DelBox = DelBox - 2*ReIR
      END

      DOUBLE COMPLEX  FUNCTION BVR_CBoxGG(MasPhot,s,t,u)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Box Gamma-Gamma, taken from   KORAZ/KORALB programs                                   //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      MasPhot,s,t,u
      DOUBLE COMPLEX  MG,Eps,SS,TT,UU,CiPi,Clnt
      DOUBLE COMPLEX  BVR_Spence,BVR_CDLN
*---------
      Eps = DCMPLX(-1.D0,0.D0)
      MG  = DCMPLX(MasPhot, 0d0)
      SS  = DCMPLX(s, 0d0)
      TT  = DCMPLX(t, 0d0)
      UU  = DCMPLX(u, 0d0)
      CiPi = DCMPLX(0d0, m_pi)
      Clnt = BVR_CDLN( (-TT/SS) ,Eps)
      BVR_CBoxGG= 
     $     BVR_CDLN( (TT/UU) ,Eps) *(BVR_CDLN( MG**2/SS ,Eps) +CiPi) !!!   <-- Infrared part
     $     +DCMPLX(0.5d0)*SS*(UU-TT)/UU**2 *( DCMPLX(0.5d0)*Clnt**2 +CiPi*Clnt)
     $     -DCMPLX(0.5d0)*SS/UU*( Clnt +Cipi)
      END

      DOUBLE COMPLEX  FUNCTION BVR_CBoxGZ(MasPhot,MassZ,GammZ,s,t,u)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*// Box Gamma-Z, From W. Brown, R. Decker, E. Pashos, Phys. Rev. Lett., 52 (1984), 1192     //
*// Programmed similarly as in KORALZ                                                       //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      MasPhot,MassZ,GammZ,s,t,u
      DOUBLE COMPLEX  MZ2,MG2,Eps,SS,TT,UU
      DOUBLE COMPLEX  BVR_Spence,BVR_CDLN
*
      Eps = DCMPLX(-1.D0,0.D0)
      MZ2  = DCMPLX(MassZ**2, -MassZ*GammZ)
      MG2  = DCMPLX(MasPhot**2, 0d0)
      SS  = DCMPLX(s, 0d0)
      TT  = DCMPLX(t, 0d0)
      UU  = DCMPLX(u, 0d0)
      BVR_CBoxGZ = 
     $        BVR_CDLN( (TT/UU) ,Eps) *BVR_CDLN( MG2/CDSQRT(TT*UU) ,Eps) !!!<-- Infrared part
     $     -2*BVR_CDLN( (TT/UU) ,Eps) *BVR_CDLN( ((MZ2-SS)/MZ2 )   ,Eps)
     $     +BVR_Spence( ((MZ2+UU)/MZ2) ,Eps)
     $     -BVR_Spence( ((MZ2+TT)/MZ2) ,Eps)
     $     +(MZ2-SS)*(UU-TT-MZ2)/UU/UU *(
     $              BVR_CDLN( (-TT/SS) ,Eps) *BVR_CDLN( ((MZ2-SS)/MZ2) ,Eps)
     $             +BVR_Spence( ((MZ2+TT)/MZ2) ,Eps)  
     $             -BVR_Spence( ((MZ2-SS)/MZ2) ,Eps)  )
     $     +(MZ2-SS)*(MZ2-SS)/UU/SS *BVR_CDLN( ((MZ2-SS)/MZ2) ,Eps) 
     $     +(MZ2-SS)/UU             *BVR_CDLN( (-TT/MZ2)      ,Eps)
      END


      DOUBLE COMPLEX  FUNCTION BVR_IntReson(MasPhot,MassZ,GammZ,s,t,u)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Resonant part of virtual formfactor                                                   //
*//   Needed for Greco-Pancheri-Srivastava exponentiation                                   //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      MasPhot,MassZ,GammZ,s,t,u
      DOUBLE COMPLEX  MZ2,MG2,Eps,SS,TT,UU
      DOUBLE COMPLEX  BVR_Spence,BVR_CDLN
*
      Eps = DCMPLX(-1.D0,0.D0)
      MZ2  = DCMPLX(MassZ**2, -MassZ*GammZ)
      MG2  = DCMPLX(MasPhot**2, 0d0)
      SS  = DCMPLX(s, 0d0)
      TT  = DCMPLX(t, 0d0)
      UU  = DCMPLX(u, 0d0)
      BVR_IntReson = 
     $     -2*BVR_CDLN( (TT/UU) ,Eps) *BVR_CDLN( ((MZ2-SS)/MZ2 )   ,Eps)
ccc     $    +BVR_CDLN( (TT/UU) ,Eps) *BVR_CDLN( MG2/CDSQRT(TT*UU) ,Eps)
ccc     $     +BVR_Spence( ((MZ2+UU)/MZ2) ,Eps)
ccc     $     -BVR_Spence( ((MZ2+TT)/MZ2) ,Eps)
      END


      DOUBLE COMPLEX  FUNCTION BVR_IntIR(MasPhot,s,t,u)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Virtual 2*(B(t)-B(u)) Intereference IR part to be subtracted from boxes               //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION      MasPhot,s,t,u
      DOUBLE COMPLEX  BVR_CDLN,Eps,MG2,TT,UU
*-------
      Eps = DCMPLX(-1.D0,0.D0)
      MG2 = DCMPLX(MasPhot**2, 0d0)
      TT  = DCMPLX(t, 0d0)
      UU  = DCMPLX(u, 0d0)
      BVR_IntIR =
     $      BVR_CDLN( (TT/UU) ,Eps) *BVR_CDLN( (MG2/CDSQRT(TT*UU)) ,Eps)
     $     +DCMPLX(0.5d0)*BVR_CDLN( (TT/UU) ,Eps)
      END

      DOUBLE PRECISION  FUNCTION  BVR_SForFac(alfpic, p1,m1, p2,m2, Emin, MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   s-channel YFS formfactor for  acollinear fermion pair.                                //
*//   Mass effects are eaxct.                                                               //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
*
      DOUBLE PRECISION    alfpic, p1(4),m1, p2(4),m2, Emin, MasPhot
      DOUBLE PRECISION    Breal, Bvirt, p1p2
      DOUBLE PRECISION    BVR_Btilda, BVR_SBvirt
*----------
      p1p2  =  p1(4)*p2(4) -p1(3)*p2(3) -p1(2)*p2(2) -p1(1)*p2(1)
      Breal = BVR_Btilda( alfpic, p1p2, p1(4),p2(4), m1, m2,  Emin, MasPhot) !! Exact
      Bvirt = BVR_SBvirt( alfpic, p1p2, m1, m2, MasPhot)                     !! Exact
      BVR_SForFac = EXP( Breal + Bvirt)
      END                       !!! BVR_SForFac !!!


      DOUBLE PRECISION  FUNCTION BVR_TForFac(alfpic, p1,m1, p2,m2, Emin, MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   T-channel YFS formfactor for acollinear fermion pair.                                 //
*//   m1 is assumed to be small, m2 can be finite.                                          //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
*
      DOUBLE PRECISION    alfpic, p1(4),m1, p2(4),m2, Emin, MasPhot, FormFac
      DOUBLE PRECISION    Breal, Bvirt, t, p1p2
      DOUBLE PRECISION    BVR_Btilda, BVR_TBvirt
*----------
      p1p2  =  p1(4)*p2(4) -p1(3)*p2(3) -p1(2)*p2(2) -p1(1)*p2(1)
      t     = -2*p1p2 +m1**2 +m2**2
      Breal   = BVR_Btilda( alfpic, p1p2, p1(4),p2(4), m1, m2,  Emin, MasPhot) !! Exact!!!
      Bvirt   = BVR_TBvirt( alfpic, p1p2, m1, m2, MasPhot) !!!! m1<<m2 approximation here!!!
      BVR_TForFac = EXP( Breal + Bvirt)
      END                       !!! BVR_TForFac !!!


      DOUBLE PRECISION   FUNCTION BVR_SBvirt(alfpic,p1p2,m1,m2,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Real part of B-VIRTUAL S-CHANNEL, Exact.                                              //
*//   Present version according to eq.(12) in uthep-95-0801                                 //
*//   Notation: mu->Nu, mu*(1+rho)->nu+xlam, rho=xlam/nu                                    //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION    alfpic,p1p2,m1,m2,MasPhot
*
      DOUBLE PRECISION     BVR_Dilog,xlam,Nu,s
*-----
      IF( p1p2 .LE. m1*m2 ) GOTO 900
      Nu = p1p2
      s = 2d0*p1p2 +m1*m1 +m2*m2
      xlam = SQRT( Nu**2 -(m1*m2)**2 )
      BVR_SBvirt = alfpic*(
     $      (Nu/xlam *LOG((Nu+xlam)/m1/m2) -1d0) *LOG(MasPhot**2/m1/m2)
     $     +xlam/s *LOG((Nu+xlam)/m1/m2)
     $     +(m1**2-m2**2)/(2d0*s) *LOG(m1/m2)
*###################################################[[[[[[[[[[[[[[[[[[[[[[[[[
ccc     $     +Nu/xlam* m_pi**2                    !!!<---  Important pi**2/beta of Schwinger
     $     +m_pi**2                    !!!<--- temporary solution for the coulomb problem
*###################################################]]]]]]]]]]]]]]]]]]]]]]]]]
     $     +Nu/xlam*( 
     $         -0.5d0*LOG((Nu+xlam)/m1**2) *LOG((Nu+xlam)/m2**2)
     $         -0.5d0 *LOG( (m1*m1 +(Nu+xlam))/(m2*m2 + Nu+xlam) )**2
     $         -BVR_Dilog( 2d0*xlam/(m1*m1 +Nu+xlam) )
     $         -BVR_Dilog( 2d0*xlam/(m2*m2 +Nu+xlam) )
     $     -1d0)
     $     )
      RETURN
 900  CONTINUE
c{{{{{
ccc      CALL  KK2f_Print1(6)
c}}}}}}
      WRITE(*,'(a,5g20.12)') '##### STOP in BVR_SBvirt: p1p2,m1,m2 = ', p1p2,m1,m2
      STOP
      END                       !!!BVR_SBvirt!!!


      DOUBLE PRECISION   FUNCTION BVR_TBvirt(alfpic,p1p2,m1,m2,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Real part of B-VIRTUAL T-CHANNEL  m1 is assumed to be very small.                     //
*//   Present version according to eq.(14) in uthep-95-0801 with COSMETIC changes.          //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION    alfpic,p1p2,m1,m2,MasPhot
*
      DOUBLE PRECISION    t,ta,zeta,BVR_Dilog

      t  = m1*m1 +m2*m2 -2d0*p1p2
      ta = ABS(t)
      zeta = 1+ m2**2/ta
      BVR_TBvirt = alfpic*(
     $     (DLOG(2d0*p1p2/(m1*m2)) -1d0)*DLOG(MasPhot**2/(m1*m2))
     $     +0.5d0*zeta*DLOG(ta*zeta/(m1*m2))
     $     -0.5d0*DLOG(ta/m1**2)*DLOG(ta/m2**2)
     $     +BVR_Dilog(1/zeta) -1d0
     $     +0.5d0*(zeta -1d0)*DLOG(m1/m2)
     $     -DLOG(zeta)*(DLOG(ta/(m1*m2)) +0.5d0*DLOG(zeta))
     $     )
      END                       !!! BVR_TBvirt !!!


      DOUBLE PRECISION   FUNCTION BVR_TBvirt2(alfpic,p1p2,m1,m2,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   B-virtual t-channel  m1 is assumed to be very small.                                  //
*//   Present version according to eq.(14) in uthep-95-0801                                 //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION    alfpic,p1p2,m1,m2,MasPhot
*
      DOUBLE PRECISION    t,ta,zeta,BVR_Dilog

      t  = m1*m1 +m2*m2 -2d0*p1p2
      ta = ABS(t)
      zeta = 1+ m2**2/ta
      BVR_TBvirt2 = alfpic*(
     $     (DLOG(ta/(m1*m2)) + DLOG(zeta) -1d0)*DLOG(MasPhot**2/m1**2)
     $     +0.5d0*zeta*(DLOG(ta/(m1*m2)) + DLOG(zeta))
     $     -0.5d0*DLOG(ta/m1**2)*DLOG(ta/m2**2)
     $     -DLOG(m2/m1)*(DLOG(ta/(m1*m2)) +DLOG(zeta) +.5d0*zeta -1.5d0)
     $     -DLOG(zeta)*(DLOG(ta/(m1*m2)) +0.5d0*DLOG(zeta))
     $     +BVR_Dilog(1/zeta) -1d0
     $     )
      END                       !!! BVR_TBvirt2 !!!


      DOUBLE PRECISION   FUNCTION BVR_TBvirtExact(alfpic,p1p2,m1,m2,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   B-virtual t-channel, masses treated exactly.                                          //
*//   Based on 1986 internal note by SJ.                                                    //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'BVR.fi'
      DOUBLE PRECISION    alfpic,p1p2,m1,m2,MasPhot
*
      DOUBLE PRECISION    BVR_Dilog,nu,xlam,w1,w2,A,A1,A3,ta
*------------
      nu  = p1p2
      ta  = 2d0*nu -m1*m1 -m2*m2
      xlam = DSQRT((nu-m1*m2)*(nu+m1*m2))
      w1  =  m1**2/(xlam+nu-m1**2)
      w2  =  m2**2/(xlam+nu-m2**2)
      A   =  1d0/xlam *DLOG((nu+xlam)/(m1*m2))
      A1  = -2d0  +(m1*m1 -m2*m2)/(-ta)*DLOG(m1/m2)
     $            -2d0*xlam**2/(-ta)*A
      A3  = DLOG(ta/(m1*m2))*A
     $     +0.5d0/xlam*( -0.5d0*DLOG(w1)**2 +0.5d0*DLOG(1d0+w1)**2
     $                   -DLOG(1d0+w1+w2)*DLOG(w1/(1d0+w1))
     $                   -BVR_Dilog((1d0+w1)/(1d0+w1+w2)) +BVR_Dilog(w1/(1d0+w1+w2)) )
     $     +0.5d0/xlam*( -0.5d0*DLOG(w2)**2 +0.5d0*DLOG(1d0+w2)**2
     $                   -DLOG(1d0+w1+w2)*DLOG(w2/(1d0+w2))
     $                   -BVR_Dilog((1d0+w2)/(1d0+w1+w2)) +BVR_Dilog(w2/(1d0+w1+w2)) )
*
      BVR_TBvirtExact = alfpic*( (nu*A-1d0)*DLOG(MasPhot**2/(m1*m2)) -nu*A3 +0.5d0*A1 )
      END                       !!! BVR_TBvirtExact !!!


      DOUBLE PRECISION   FUNCTION BVR_Btilda(alfpi,p1p2,E1,E2,Mas1,Mas2,Kmax,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//  Full/Complete/exact 2-particle Btilde function.                                        //
*//  Exact treatment of masses, numericaly stable in high energy limit.                     //
*//  Equivalent of routine Btilde programmed by W. Placzek (S.J.)                           //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    alfpi,p1p2,E1,E2,Mas1,Mas2,Kmax,MasPhot
      DOUBLE PRECISION    BVR_A, BVR_A4, BVR_A4sng
*-------------
      BVR_Btilda = alfpi*(
     $     (p1p2*BVR_A( p1p2,Mas1,Mas2) -1 )*2*LOG(2*Kmax/MasPhot)
     $     +p1p2*BVR_A4(p1p2,E1,E2,Mas1,Mas2)
     $     -0.5d0*Mas1**2*BVR_A4sng(E1,Mas1)
     $     -0.5d0*Mas2**2*BVR_A4sng(E2,Mas2)
     $     )
      END


      DOUBLE PRECISION    FUNCTION BVR_Btildc(alfpi,p1p2,E1,E2,Mas1,Mas2,Kmax,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*// Crude/Truncated (crude MC) 2-particle Btilde equivalent S.J.                            //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    alfpi,p1p2,E1,E2,Mas1,Mas2,Kmax,MasPhot
      DOUBLE PRECISION    BVR_A, BVR_A4
*-------------
      BVR_Btildc = alfpi*(
     $      p1p2*BVR_A( p1p2      ,Mas1,Mas2)*2*LOG(2*Kmax/MasPhot)
     $     +p1p2*BVR_A4(p1p2,E1,E2,Mas1,Mas2)
     $     )
      END

      DOUBLE PRECISION  FUNCTION BVR_A(p1p2,Mas1,Mas2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//   Function A(p1,p2) real version appropriate for B-tilde calculation                    //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION     p1p2,Mas1,Mas2
      DOUBLE PRECISION     Mas12,xlam
*----------
      Mas12 = Mas1*Mas2
      IF ( (p1p2-Mas12) .LT. 1d-10) THEN
         BVR_A=0d0
         WRITE(*,*) '+++++++ BVR_A:: WARNING, p1p2 = ',p1p2
         RETURN
      ENDIF
      xlam = SQRT( (p1p2 - Mas12)*(p1p2 + Mas12) )
      BVR_A  = 1/xlam *LOG( (p1p2 + xlam)/Mas12 )
      END

      DOUBLE PRECISION   FUNCTION BVR_A4sng(E1,Mas1)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*// Function (p1*p1)*A4(p1,p1) equal argument momenta.                                      //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION   E1,Mas1
      DOUBLE PRECISION   bet1,b1ln
*-------
      bet1 = SQRT(1-Mas1**2/E1**2)
      b1ln = 2*LOG( (1+bet1)*E1/Mas1 )
      BVR_A4sng = -1/Mas1**2/bet1 *b1ln
      END

      DOUBLE PRECISION   FUNCTION BVR_A4(p1p2,En1,En2,xm1,xm2)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//! This function provides an analytical result for the integral         !                 //
*//! A4(p1,p2) being a part of the YFS IR function B-tilde.               !                 //
*//! Note: This is a general case without any approximation!              !                 //
*//! INPUT: p1p2    - scalar product of the 4-momenta p1 and p2;          !                 //
*//!        E1,E2   - particles energies;                                 !                 //
*//!        xm1,xm2 - particles masses;                                   !                 //
*//!----------------------------------------------------------------------!                 //
*//! Written by:  Wieslaw Placzek                Knoxville, January  1996 !                 //
*//! Last update: 30.01.1996                by: W.P.                      !                 //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION     p1p2,En1,En2,xm1,xm2
      DOUBLE PRECISION     E1,E2,Mas1,Mas2,p1s,p2s,p2,Ep,Em,sm,dm,Q2,xl,xq,qp,qm
      DOUBLE PRECISION     et0,et1,vet0,vet1,Eln,y1,y2,y3,y4
      DOUBLE PRECISION     BVR_Yijeta
! Statement function
      DOUBLE PRECISION     etaln,x1,x2,x3,x4,z
      etaln(x1,x2,x3,x4,z) = LOG(ABS( (z-x1)*(z-x2)/(z-x3)/(z-x4) ))
*-----------
! Some auxiliary variables
      E1 = En1
      E2 = En2
      Mas1 = xm1
      Mas2 = xm2
      p1s = E1**2 - Mas1**2
      p2s = E2**2 - Mas2**2
      IF (p1s.lt.p2s) THEN
        Mas1 = xm2
        Mas2 = xm1
        E1 = En2
        E2 = En1
      ENDIF
      Ep  = E1 + E2
      Em  = E1 - E2
      sm  = Mas1 + Mas2 
      dm  = Mas1 - Mas2
      Q2  = 2*p1p2 - Mas1**2 - Mas2**2
      xl  = SQRT( (Q2 + sm**2)*(Q2 + dm**2) )
      xq  = SQRT(Q2 + Em**2)
      qp = xq + Em
      qm = xq - Em
      et0 = SQRT(E2**2 - Mas2**2)
      IF (p1p2.gt.E1*E2) et0 = -et0
      et1 = SQRT(E1**2 - Mas1**2) + xq
      y1  = 0.5d0*( (xq - Ep) + (sm*dm + xl)/qp )
      y2  = y1 - xl/qp
      y3  = 0.5d0*( (xq + Ep) + (sm*dm + xl)/qm )
      y4  = y3 - xl/qm       
! Some auxiliary functions
      IF (ABS(Em).gt.1d-10) THEN
        Eln = LOG(ABS(qm/qp))*( etaln(y1,y4,y2,y3,et1) 
     &                        - etaln(y1,y4,y2,y3,et0) )
      ELSE
        Eln = 0
      ENDIF
      Vet0 = BVR_Yijeta(y1,y4,et0) + BVR_Yijeta(y2,y1,et0)
     &     + BVR_Yijeta(y3,y2,et0) - BVR_Yijeta(y3,y4,et0)
     &     + 0.5d0*etaln(y1,y2,y3,y4,et0)*etaln(y2,y3,y1,y4,et0)
      Vet1 = BVR_Yijeta(y1,y4,et1) + BVR_Yijeta(y2,y1,et1)
     &     + BVR_Yijeta(y3,y2,et1) - BVR_Yijeta(y3,y4,et1)
     &     + 0.5d0*etaln(y1,y2,y3,y4,et1)*etaln(y2,y3,y1,y4,et1)
! Function A4(p1,p2) 
      BVR_A4 = 1/xl*(Eln + Vet1 - Vet0 )
      END

      DOUBLE PRECISION   FUNCTION BVR_Yijeta(yi,yj,eta)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*/ !----------------------------------------------------------------------!                 //
*/ ! Some auxiliary function (combination of Logs and Dilogs) used in     !                 //
*/ ! the function A4anal for A4(p1,p2).                                   !                 //
*/ !----------------------------------------------------------------------!                 //
*/ ! Written by:  Wieslaw Placzek                Knoxville, January  1996 !                 //
*/ ! Last update: 30.01.1996                by: W.P.                      !                 //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION    yi,yj,eta
      DOUBLE PRECISION    BVR_Dilog
*-----
      BVR_Yijeta = 2*BVR_Dilog( ( yj-yi)/(eta-yi) ) 
     &         + 0.5d0*LOG(ABS( (eta-yi)/(eta-yj) ))**2
      END



      DOUBLE PRECISION FUNCTION BVR_Btilde(alfinv,p1,p2,am1,am2,aKmax,MasPhot)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//  UNUSED!!!, Btilde is replaced by the equivalent function Btildf                        //
*//                                                                                         //
*//  !----------------------------------------------------------------------!               //
*//  ! This function provides a value of YFS real photon IR function        !               //
*//  ! B-tilde for any pair of charged particles.                           !               //
*//  ! INPUT: p1,p2   - particles 4-momenta;                                !               //
*//  !        am1,am2 - particles masses;                                   !               //
*//  !        MasPhot   - 'photon mass' (IR regulator)                      !               //
*//  !        aKmax   - maximum soft photon energy [GeV]                    !               //
*//  !----------------------------------------------------------------------!               //
*//  ! Written by:  Wieslaw Placzek                Knoxville, January  1996 !               //
*//  ! Last update: 30.01.1996                by: W.P.                      !               //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      DOUBLE PRECISION pi
      PARAMETER ( pi = 3.1415926535897932D0)
      DOUBLE PRECISION  p1(4),p2(4)
      DOUBLE PRECISION  BVR_A4
      DOUBLE PRECISION  alfpi,bet1,bet2,xlam,a,a4,btian,b1ln,b2ln,p1p2
      DOUBLE PRECISION  am2,akmax,alfinv,am1,e2,am12,masphot,e1
*---------------------------------------------------------------------------------------------
      alfpi = 1/alfinv/pi
      BVR_Btilde = 0
      E1 = p1(4)
      E2 = p2(4)
      am12 = am1*am2
      p1p2 = p1(4)*p2(4) - p1(3)*p2(3) - p1(2)*p2(2) - p1(1)*p2(1)
      IF (p1p2-am12.lt.1d-10) RETURN
      xlam = SQRT( (p1p2 - am12)*(p1p2 + am12) )
! Function A(p1,p2)
      A  = 1/xlam *LOG( (p1p2 + xlam)/am12 )
      bet1 = SQRT(1-am1**2/E1**2)
      bet2 = SQRT(1-am2**2/E2**2)
      b1ln = 2*LOG( (1+bet1)*E1/am1 )
      b2ln = 2*LOG( (1+bet2)*E2/am2 )
! Function A4(p1,p2)
      A4 = BVR_A4(p1p2,E1,E2,am1,am2)
! B-tilde(p1,p2;aKmax,MasPhot)
      Btian = (p1p2*A - 1) *2*LOG(2*aKmax/MasPhot)
     &       + p1p2*A4 + 0.5/bet1*b1ln + 0.5/bet2*b2ln
      BVR_Btilde = alfpi*Btian
      END

      SUBROUTINE BVR_wform( CMSene,alfinv,q1,q2,amf,delta,eps,dyfs)
*/////////////////////////////////////////////////////////////////////////////////////////////
*// UNUSED, kept for some future tests, small mass approx.                                  //
*// Yennie-Frautschi-Suura formfactors for the final state ferm. pair                       //
*// S.J. 1987                                                                               //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION CMSene,alfinv,q1(4),q2(4),amf,delta,eps,dyfs

      DOUBLE PRECISION pi
      PARAMETER (pi=3.1415926535897932d0)
      DOUBLE PRECISION alf1,q1q2,em,ep,remn,dl,e1,e2,ene,svar,delb,gamf2
      DOUBLE PRECISION bvr_dilog
*---------------------------------------------------------------------------------------------
      alf1=1/alfinv/pi
      svar  = cmsene**2
      ene= cmsene/2
* momenta q1,q2 should be in cms
      e1 = q1(4)
      e2 = q2(4)
      gamf2 = 2*alf1* dlog(svar /amf**2)
      DelB  = gamf2*dlog(ene/SQRT(e1*e2)*eps/delta)
      ep    = e1+e2
      em    = e1-e2
      q1q2  = q1(4)*q2(4)-q1(3)*q2(3)-q1(2)*q2(2)-q1(1)*q2(1)
      dl    = SQRT( 2*q1q2 +em**2 )
      remn  = pi**2/2
     $        -0.50d0*dlog(e1/e2)**2
     $        -0.25d0*dlog((dl+em)**2/(4*e1*e2))**2
     $        -0.25d0*dlog((dl-em)**2/(4*e1*e2))**2
     $        - BVR_Dilog((dl+ep)/(dl+em)) -BVR_Dilog((dl-ep)/(dl-em))
     $        - BVR_Dilog((dl-ep)/(dl+em)) -BVR_Dilog((dl+ep)/(dl-em))
      dyfs  = exp( DelB +alf1*remn )
      END



      DOUBLE PRECISION FUNCTION BVR_Dilog(x)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*// dilogarithm FUNCTION: dilog(x)=int( -ln(1-z)/z ) , 0 < z < x .                          //
*// this is the cernlib version.                                                            //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  x
*
      DOUBLE PRECISION  a,b,y,s,t,z
*---------------------------------------------------------------------------------------------
      z=-1.644934066848226d0
      IF(x  .LT. -1.d0) GOTO 1
      IF(x  .LE.  0.5d0) GOTO 2
      IF(x  .EQ.  1.d0) GOTO 3
      IF(x  .LE.  2.d0) GOTO 4
      z=3.289868133696453d0
    1 t=1.d0/x
      s=-0.5d0
      z=z-0.5d0*dlog(dabs(x))**2
      GOTO 5
    2 t=x
      s=0.5d0
      z=0.d0
      GOTO 5
    3 BVR_Dilog=1.644934066848226d0
      RETURN
    4 t=1.d0-x
      s=-0.5d0
      z=1.644934066848226d0-dlog(x)*dlog(dabs(t))
    5 y=2.666666666666667d0*t+0.666666666666667d0
      b=      0.000000000000001d0
      a=y*b  +0.000000000000004d0
      b=y*a-b+0.000000000000011d0
      a=y*b-a+0.000000000000037d0
      b=y*a-b+0.000000000000121d0
      a=y*b-a+0.000000000000398d0
      b=y*a-b+0.000000000001312d0
      a=y*b-a+0.000000000004342d0
      b=y*a-b+0.000000000014437d0
      a=y*b-a+0.000000000048274d0
      b=y*a-b+0.000000000162421d0
      a=y*b-a+0.000000000550291d0
      b=y*a-b+0.000000001879117d0
      a=y*b-a+0.000000006474338d0
      b=y*a-b+0.000000022536705d0
      a=y*b-a+0.000000079387055d0
      b=y*a-b+0.000000283575385d0
      a=y*b-a+0.000001029904264d0
      b=y*a-b+0.000003816329463d0
      a=y*b-a+0.000014496300557d0
      b=y*a-b+0.000056817822718d0
      a=y*b-a+0.000232002196094d0
      b=y*a-b+0.001001627496164d0
      a=y*b-a+0.004686361959447d0
      b=y*a-b+0.024879322924228d0
      a=y*b-a+0.166073032927855d0
      a=y*a-b+1.935064300869969d0
      BVR_Dilog=s*t*(a-b)+z
      END

      DOUBLE COMPLEX  FUNCTION BVR_Spence(Y,E)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//  Spence function of y+i*REAL(E) where E is an infinitesimal                             //
*//  Programmed probably by R. Stuart                                                       //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      DOUBLE COMPLEX  Y,E
      DOUBLE PRECISION  B(9),FACT
      DOUBLE COMPLEX  A,CLN,PISQ6,PROD,TERM,X,Z,ZSQ
      DOUBLE COMPLEX  BVR_CDLN
      INTEGER    j,i1,i2
*---------------------------------------------------------------------------------------------
      B(1)=1.D0/6.D0
      B(2)=-1.D0/30.D0
      B(3)=1.D0/42.D0
      B(4)=B(2)
      B(5)=5.D0/66.D0
      B(6)=-691.D0/2730.D0
      B(7)=7.D0/6.D0
      B(8)=-3617.D0/510.D0
      B(9)=43867.D0/798.D0
      PISQ6=(1.6449340668482264D0,0.D0)
      I1=0
      I2=0
      X=Y
      A=E
      IF(X.EQ.(0.D0,0.D0))THEN
        BVR_Spence=(0.D0,0.D0)
        RETURN
      ENDIF
      IF(X.EQ.(1.D0,0.D0))THEN
        BVR_Spence=PISQ6
        RETURN
      ENDIF
C  IF X LIES OUTSIDE THE UNIT CIRCLE THEN EVALUATE BVR_Spence(1/X)
      IF(CDABS(X).GT.1.D0)THEN
        X=1.D0/X
        A=-A
        I1=1
      ENDIF
C  IF REAL(X)>1/2 THEN EVALUATE BVR_Spence(1-X)
      IF(DREAL(X).GT.0.5D0)THEN
        X=1.D0-X
        A=-A
        I2=1
      ENDIF
C  EVALUATE SERIES FOR BVR_Spence(X)
      Z=-BVR_CDLN(1.D0-X,-A)
      ZSQ=Z*Z
      BVR_Spence=Z-ZSQ/4.D0
      PROD=Z
      FACT=1.D0
      DO 10 J=2,18,2
      FACT=FACT*DCMPLX(DBLE((J+1)*J))
      PROD=PROD*ZSQ
      TERM=B(J/2)/FACT*PROD
      BVR_Spence=BVR_Spence+TERM
      IF(CDABS(TERM/BVR_Spence).LT.1.D-20) GOTO 20
10    CONTINUE
C  ADD APPROPRIATE LOGS TO OBTAIN SPENCE FUNCTION OF ORIGINAL ARGUEMENT
20    IF(I2.EQ.1)THEN
        BVR_Spence=-BVR_Spence+PISQ6-BVR_CDLN(X,A)*BVR_CDLN(1.D0-X,-A)
        X=1.D0-X
        A=-A
      ENDIF
      IF(I1.EQ.1)THEN
        CLN=BVR_CDLN(-X,-A)
        BVR_Spence=-BVR_Spence-PISQ6-CLN*CLN/2.D0
      ENDIF
      RETURN
      END

      DOUBLE COMPLEX  FUNCTION BVR_CDLN(X,A)
*/////////////////////////////////////////////////////////////////////////////////////////////
*//                                                                                         //
*//  Complex functions that take account of the I*Epsilon prescription                      //
*//  Complex logarithm of X+I*REAL(A) where a is an infinitesimal                           //
*//  Programmed probably by R. Stuart                                                       //
*//                                                                                         //
*/////////////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE COMPLEX  A,X
      DOUBLE COMPLEX  PI
*--------
      PI=(3.141592653589793238462643D0,0.D0)
      IF(DIMAG(X) .EQ. 0.D0 .AND. DREAL(X) .LE. 0.D0) THEN
         BVR_CDLN =CDLOG(-X) +(0.D0,1.D0)*PI*DSIGN(1.D0,DREAL(A))
      ELSE
         BVR_CDLN =CDLOG(X)
      END IF
      IF(DIMAG(BVR_CDLN).GT.DREAL(PI) ) BVR_CDLN =BVR_CDLN -(0.D0,1.D0)*PI
      IF(DIMAG(BVR_CDLN).LT.DREAL(-PI)) BVR_CDLN =BVR_CDLN +(0.D0,1.D0)*PI
      RETURN
      END


*/////////////////////////////////////////////////////////////////////////////////////////////
*//                   End of class BVR                                                      //
*/////////////////////////////////////////////////////////////////////////////////////////////
