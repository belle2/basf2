*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//                     Pseudo-CLASS  QED3                                          //
*//                                                                                 //
*//   Calculation of QED matrix element with Yennie-Frautschi-Suura exponentiation  //
*//   for s-chanel exchange-exchange fermion-anfifermion production processe.       //
*//   Order alpha^1 is complete, beyond O(alf^1) leading-log is mainly exploited.   //
*//                                                                                 //
*//   e+ e- ---> f + fbar + n gamma                                                 //
*//                                                                                 //
*//   The following contributions are included:                                     //
*//                                                                                 //
*//   ISR:  O(L^0*alf^0)                                                            //
*//         O(L^1*alf^1)  O(L^0*alf^1)                                              //
*//         O(L^2*alf^2)                                                            //
*//         O(L^3*alf^3)                                                            //
*//   FSR:  O(L^0*alf^0)                                                            //
*//         O(L^1*alf^1)  O(L^0*alf^1)                                              //
*//         O(L^2*alf^2)                                                            //
*//                                                                                 //
*//   Neglected:                                                                    //
*//      ISR*FSR interferences, spin polarization                                   //
*//      t-chanel exchanges                                                         //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////


      SUBROUTINE QED3_Initialize(xpar)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Initialization directly from basic input                                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  xpar(*)
*
      INCLUDE 'QED3.fi'
*
      m_IdeWgt = xpar(11)
      m_alfinv = xpar(30)
      m_KeyISR = xpar(20)
      m_vvmin  = xpar(16)
      END

      SUBROUTINE QED3_Make
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   main routine for calculation of long list of the weights                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      SAVE
      INCLUDE 'QED3.fi'
*
      DOUBLE PRECISION   xx(4)
      DOUBLE PRECISION   pf1(4),pf2(4)
      DOUBLE PRECISION   qf1(4),qf2(4)
      INTEGER nphox,nphoy
      DOUBLE PRECISION   xphot(100,4),yphot(100,4)
      DOUBLE PRECISION   yini(100),zini(100),yfin(100),zfin(100)
* Elements of beta calculation, Auxiliary/temporary
      DOUBLE PRECISION   beta20,beta21,beta30
      DOUBLE PRECISION   betx12,bety12
      DOUBLE PRECISION   beti12,beti21
      DOUBLE PRECISION   betf01
* Contributions from individual photons
      DOUBLE PRECISION  
     $  betx10(npmx),              ! ISR beta1 tree     times (1+delf1), factorization!
     $  betx11(npmx),              ! ISR beta1 one-loop times (1+delf1), factorization!
     $  bety10(npmx),              ! FSR beta1 tree     times (1+deli1), factorization!
     $  bety11(npmx),              ! FSR beta1 one-loop times (1+deli1), factorization!
     $  betxx20(npmx,npmx),        ! beta2 ISR*ISR tree 
     $  betxy20(npmx,npmx),        ! beta2 ISR*FSR tree 
     $  betyy20(npmx,npmx),        ! beta2 FSR*FSR tree 
     $  beti10(npmx),              ! beta1 tree   ISR only
     $  beti11(npmx),              ! beta1 1-loop ISR only
     $  beti20(npmx,npmx),         ! beta2 tree   ISR only
     $  betf10(npmx),              ! beta1 tree   FSR only, for beta2, beta3
     $  betf11(npmx)               ! beta1 1-loop FSR only, for beta2, beta3
***********************************************************************
      DOUBLE PRECISION   qq(4),pp(4)
      DOUBLE PRECISION   ggf1,ggf2,gggi1,gggi2,gi1,gi2,ggi1,ggi2,gf1,gf2
      DOUBLE PRECISION   cth21,cth12,cth22,cth11
      DOUBLE PRECISION   DisCru
      DOUBLE PRECISION   dist20,disi11,dist10,dist11,dist12,dist30
      DOUBLE PRECISION   andi11,andi21,andi12,andi22,andis,dist21
      DOUBLE PRECISION   hfac1,hfac2,hfac,hfac3,sfacj
      DOUBLE PRECISION   zz,yy,y3,z3,z1,y1,z2,y2,uu,vv,z,y
      DOUBLE PRECISION   ForFin,ForIni,fYFS,fYFSu
      DOUBLE PRECISION   Bor1
c{{{{
      DOUBLE PRECISION   ph(4),kq1,kq2,p1p2,q1q2,tt,tt1,uu1,borc,dig1,sofc
c}}}}

      DOUBLE PRECISION   gami,gamf,delp,delq,deli3,deli2,delf2,delf1,deli1,delf3
      DOUBLE PRECISION   svar,svar1,svar2
      DOUBLE PRECISION   amel,amfin,charge,charg2

      DOUBLE PRECISION   BornV_GetMass,BornV_GetCharge,BornV_Differential

      INTEGER i,j,k,jph,ntree
      INTEGER jph1,jph2,jph3
*
      INTEGER IsFSR,KFbeam,KFfin
*
      INTEGER icont             !debug
      SAVE    icont
      DOUBLE PRECISION   wtm2,wtm0,wtm1
***********************************************************************
**                     Inline functions                              **
***********************************************************************
* Multiplicative mass-correction to truncated S-factor 
      DOUBLE PRECISION   wm0,wm1,wmd,wm7,a,b,del
      wm0(del,a,b)= 1d0 -2d0*del -del*(a/b+b/a)
*
*     O(alf1) amplitude, bremsstrahlung factor, del->0 limit
      wm1(del,a,b)=
     $  1d0 - del*(a/b+b/a)*(1d0-a)*(1d0-b)*2d0/((1d0-a)**2+(1d0-b)**2)
*
* wmd as in BHLUMI, (what about exact mass terms???)
      wmd(del,a,b) =
     $  1d0 + del*(a/b+b/a)*(a**2+b**2)/((1-a)**2+(1-b)**2)
*
* Factorized wm1, as in BHLUMI, to improve numerical stability.
*     Identity wm1=wm7=wm0*wmd, is true up to delta**4 terms
      wm7(del,a,b) = (1d0 +2d0*del -del*(a/b+b/a))
     $ *(1d0 + del*(a**2+b**2)*(a**2+b**2)/((1-a)**2+(1-b)**2)/(a*b))
*
**                 End of inline functions                           **
***********************************************************************
      DATA icont /0/
*     =================================================================
      icont=icont+1
      m_WtBest=1d0

      DO i=1,m_lenwt
         m_WtSet(i)=0d0
      ENDDO
*
      KFbeam = 11                      ! KF=11 is electron
      amel   = BornV_GetMass(KFbeam)
* Actual KFcode of final fermion
      CALL KarLud_GetKFfin(KFfin)
      amfin  = BornV_GetMass(KFfin)
* Final state charge, -1 for electron
      charge = BornV_GetCharge(KFfin)
      charg2 = charge**2

* Check dynamicaly on FSR (quarks!!!)
      CALL KarFin_GetIsFSR(IsFSR)

* ISR
      CALL KarLud_GetSudakov(nphox,yini,zini)
      CALL KarLud_GetPhotons(nphox,xphot)
      CALL KarLud_GetBeams(pf1,pf2)
* FSR
      CALL KarFin_GetSudakov(nphoy,yfin,zfin)
      CALL KarFin_GetPhotons(nphoy,yphot)
      CALL KarFin_GetFermions(qf1,qf2)

* Define 4-mometa for initial/final states and Z
      DO k=1,4
         pp(k)= pf1(k)+ pf2(k)
         qq(k)= qf1(k)+ qf2(k)
         xx(k)= qq(k)
      ENDDO
      DO j=1,nphoy
         DO k=1,4
            xx(k) = xx(k)+yphot(j,k)
         ENDDO
      ENDDO

      svar  = pp(4)**2-pp(3)**2-pp(2)**2-pp(1)**2
      svar1 = xx(4)**2-xx(3)**2-xx(2)**2-xx(1)**2
      svar2 = qq(4)**2-qq(3)**2-qq(2)**2-qq(1)**2
      vv = 1d0 -svar1/svar
      uu = 1d0 -svar2/svar1
      gami =         2d0/m_alfinv/pi*(dlog(svar/amel**2)  -1d0)
      gamf = charg2* 2d0/m_alfinv/pi*(dlog(svar2/amfin**2)-1d0)

* Crude Born distribution
      CALL KK2f_GetBornCru(DisCru)
*
      delp=  amel**2/svar
      delq= amfin**2/svar2
*
      CALL KinLib_ThetaR(xx,pf1,pf2,qf1,qf2,cth11,cth12,cth21,cth22)
      andi11= BornV_Differential(1,KFfin,svar1,cth11,0d0,0d0,0d0,0d0)
      andi12= BornV_Differential(1,KFfin,svar1,cth12,0d0,0d0,0d0,0d0)
      andi21= BornV_Differential(1,KFfin,svar1,cth21,0d0,0d0,0d0,0d0)
      andi22= BornV_Differential(1,KFfin,svar1,cth22,0d0,0d0,0d0,0d0)
*-----------------------------------------------------------
*                beta0 
*-----------------------------------------------------------
* Beta0 components
      CALL QED3_bvirt0(m_alfinv,   1d0,svar , amel,deli1,deli2,deli3)
      CALL QED3_bvirt0(m_alfinv,charg2,svar2,amfin,delf1,delf2,delf3)
*
      IF(m_KeyISR .EQ. 0)  deli1   = 0d0
      IF(m_KeyISR .EQ. 0)  deli2   = 0d0
      IF(IsFSR    .EQ. 0)  delf1   = 0d0
      IF(IsFSR    .EQ. 0)  delf2   = 0d0

* Beta0, initial+final, factorized form
      andis = (andi11 +andi12 +andi21 +andi22)/4
      m_Beta03 = andis*(1d0+deli1+deli2+deli3)*(1d0+delf1+delf2) !O(alf3)
      m_Beta02 = andis*(1d0+deli1+deli2)      *(1d0+delf1+delf2) !O(alf2)
      m_Beta01 = andis*(1d0+deli1)            *(1d0+delf1)       !O(alf1)
      m_Beta00 = andis                                           !O(alf0)
* Initial only
      m_beti03 = andis*(1d0+deli1+deli2+deli3)
      m_beti02 = andis*(1d0+deli1+deli2)
      m_beti01 = andis*(1d0+deli1)
      m_beti00 = andis
* Auxiliary
      betf01 = andis*(1d0+delf1)

c[[[[[[[
c      IF( nphox+nphoy .EQ.0) THEN
c      write(*,*) '||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||'
c      write(*,*) ' QED3: nphox+nphoy =',nphox+nphoy
c      write(*,*) ' QED3: m_Beta01,m_Beta02 = ',m_Beta01,m_Beta02
c      write(*,*) ' QED3: (1d0+deli1)*(1d0+delf1)= ',(1d0+deli1)*(1d0+delf1)
c      write(*,*) ' QED3: (1d0+deli1+deli2)*(1d0+delf1+delf2)= ',(1d0+deli1+deli2)*(1d0+delf1+delf2)
c      ENDIF
c]]]]]]]

*-----------------------------------------------------------
*                beta1 initial
*-----------------------------------------------------------
      m_xBet10 = 0d0
      m_xBet11 = 0d0
      m_xBet12 = 0d0
      m_sbti10 = 0d0
      m_sbti11 = 0d0
      m_sbti12 = 0d0
      IF(m_KeyISR .NE. 0  .AND.  vv .GT. m_vlim1) THEN
         DO jph=1,nphox
            y = yini(jph)
            z = zini(jph)
            sfacj  =  2d0/(y*z) *wm0(delp,y,z)
            m_xSfac(jph) = sfacj
            hfac = wmd(delp,y,z) *sfacj
            gf1 = 0.5d0
            gf2 = 0.5d0
            CALL QED3_Disr1(gami,yini,zini,jph, gi1,gi2,ggi1,ggi2,gggi1,gggi2)
*---- O(alf1) ----,  tree_level --------
*     The unconventional (1+delf1) in betx10 helps ISR*FSR factorization
*     in the O(alf2) semi-analytical x-check
            dist10= (  gi1*gf1*andi11   +gi1*gf2*andi12
     $                +gi2*gf1*andi21   +gi2*gf2*andi22)*hfac
            betx10(jph)=(dist10 -m_Beta00*sfacj )*(1+delf1)
            m_xBet10 = m_xBet10 +betx10(jph) /sfacj
*---- O(alf2) ----,  one_loop_level --------
            dist11= ( ggi1*gf1*andi11  +ggi1*gf2*andi12
     $               +ggi2*gf1*andi21  +ggi2*gf2*andi22)*hfac
            betx11(jph)= dist11*(1+delf1)       -m_Beta01*sfacj
            m_xBet11 = m_xBet11 +betx11(jph) /sfacj
*---- O(alf3) ----,  two_loop_level -------- !!!NEW!!!
            dist12= (gggi1*gf1*andi11 +gggi1*gf2*andi12
     $              +gggi2*gf1*andi21 +gggi2*gf2*andi22)*hfac
            betx12     = dist12*(1+delf1+delf2) -m_Beta02*sfacj
            m_xBet12 = m_xBet12 +betx12      /sfacj
***** pure ISR
            beti10(jph) =  dist10 -m_beti00*sfacj   !O(alf1)
            m_sbti10 = m_sbti10 +beti10(jph) /sfacj
            beti11(jph) =  dist11 -m_beti01*sfacj   !O(alf2)
            m_sbti11 = m_sbti11 +beti11(jph) /sfacj
            beti12      =  dist12 -m_beti02*sfacj   !O(alf3) !!!NEW
            m_sbti12 = m_sbti12 +beti12      /sfacj
         ENDDO
      ELSE
         DO jph=1,nphox
            m_xSfac(jph)  = -1d0
            betx10(jph) =  0d0
            betx11(jph) =  0d0
            beti10(jph) =  0d0
            beti11(jph) =  0d0
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta1 final
*-----------------------------------------------------------
      m_yBet10=0d0
      m_yBet11=0d0
      m_yBet12=0d0
      IF(IsFSR .NE. 0  .AND.  uu .GT. m_vlim1) THEN
         DO jph=1,nphoy
            yy = yfin(jph)
            zz = zfin(jph)
            y  = yy/(1 +yy+zz)
            z  = zz/(1 +yy+zz)
            sfacj  =  2d0/(yy*zz)*wm0(delq,yy,zz)
            m_ySfac(jph) = sfacj
            hfac = wmd(delq,y,z) *sfacj
            gi1 = 0.5d0
            gi2 = 0.5d0
            CALL QED3_Dfsr1(gamf,yfin,zfin,jph, gf1,gf2,ggf1,ggf2)
*---- O(alf1) ---,  tree level
*     unconventional (1+deli1) in bety10 helps ISR*FSR factorization 
*     in the O(alf2) semi-analytical x-check
            dist10= (gi1*gf1*andi11   + gi1*gf2*andi12
     $              +gi2*gf1*andi21   + gi2*gf2*andi22)*hfac
            bety10(jph) = (dist10 -m_Beta00*sfacj  )*(1d0+deli1) !!!
            m_yBet10 = m_yBet10 +bety10(jph) /sfacj
*---- O(alf2) ---, one loop level
            dist11= (gi1*ggf1*andi11 + gi1*ggf2*andi12
     $              +gi2*ggf1*andi21 + gi2*ggf2*andi22)*hfac
            bety11(jph) =  dist11*(1+deli1) -m_Beta01*sfacj
            m_yBet11 = m_yBet11 +bety11(jph) /sfacj
*---- O(alf3) ---, two loop level for ISR !!!NEW
*     Additional O(alf2) ISR virtual correction deli2 only
            bety12 = (1+deli1+deli2)*(dist11 -m_Beta00*(1+delf1)*sfacj)
            m_yBet12 = m_yBet12 +bety12 /sfacj
*****  pure FSR *****, for construction of beta2, beta3
            betf10(jph) =  dist10 -m_Beta00*sfacj
            betf11(jph) =  dist11 -betf01*sfacj
         ENDDO
      ELSE
        DO jph=1,nphoy
           m_ySfac(jph)  = -1d0
           bety10(jph) =  0d0
           bety11(jph) =  0d0
        ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta2 initial-initial
*-----------------------------------------------------------
      m_xxBet20=0d0
      m_xxBet21=0d0
      m_sbti20=0d0
      m_sbti21=0d0
      IF(m_KeyISR .NE. 0  .AND.  vv .GT. m_vlim2) THEN
         DO jph2=2,nphox
            DO jph1=1,jph2-1
               hfac1  =  m_xSfac(jph1)*wmd(delp,yini(jph1),zini(jph1))
               hfac2  =  m_xSfac(jph2)*wmd(delp,yini(jph2),zini(jph2))
*     Summation over two LL fragmentation trees for 2 ISR ohotons,
*     photon jph1 is always harder because of low level M.C. generation
               ntree = 2  ! for 2 ISR fragmentation trees
               gi1  = 0d0
               gi2  = 0d0
               ggi1 = 0d0
               ggi2 = 0d0
               CALL QED3_Disr2(gami,yini,zini,jph1, jph1,jph2,
     $                                  gi1,gi2,ggi1,ggi2) ! 1-st tree
               CALL QED3_Disr2(gami,yini,zini,jph1, jph2,jph1,
     $                                  gi1,gi2,ggi1,ggi2) ! 2-nd tree
               gf1 = 0.5d0      ! 0.5d0 for averaging over 2 choices
               gf2 = 0.5d0      ! of Born angles in final state
*---  O(alf2) ---, tree level---------
               dist20= (gi1*gf1*andi11 + gi1*gf2*andi12
     $                 +gi2*gf1*andi21 + gi2*gf2*andi22)
     $                 *hfac1*hfac2/ntree
* In beta20 we use beti10 instead of betx10,
* Reason: unusual definition of betx10, see relevant comment above
               beta20 = dist20
     $              -m_Beta00*m_xSfac(jph1)*m_xSfac(jph2)
     $              -beti10(jph1)*m_xSfac(jph2) -beti10(jph2)*m_xSfac(jph1)
               betxx20(jph1,jph2)=beta20
               m_xxBet20=m_xxBet20 +beta20 /m_xSfac(jph1)/m_xSfac(jph2)
*---  O(alf3) ---, one loop level ---------!!!!NEW!!!!
               dist21= (ggi1*gf1*andi11+ ggi1*gf2*andi12
     $                 +ggi2*gf1*andi21+ ggi2*gf2*andi22)
     $                 *hfac1*hfac2/ntree
               beta21 = dist21*(1+delf1)
     $              -m_Beta01*m_xSfac(jph1)*m_xSfac(jph2)
     $              -betx11(jph1)*m_xSfac(jph2) -betx11(jph2)*m_xSfac(jph1)
               m_xxBet21=m_xxBet21 +beta21 /m_xSfac(jph1)/m_xSfac(jph2)
***** Pure ISR *****
               m_sbti20=m_sbti20 +beta20 /m_xSfac(jph1)/m_xSfac(jph2)
               beti21 = dist21
     $              -m_beti01*m_xSfac(jph1)*m_xSfac(jph2)
     $              -beti11(jph1)*m_xSfac(jph2) -beti11(jph2)*m_xSfac(jph1)
               m_sbti21=m_sbti21 +beti21 /m_xSfac(jph1)/m_xSfac(jph2)
            ENDDO
         ENDDO
      ELSE
         DO  jph2=2,nphox
            DO  jph1=1,nphox
               betxx20(jph1,jph2)= 0d0
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta2 final-final
*-----------------------------------------------------------
      m_yyBet20=0d0
      m_yyBet21=0d0
      IF(IsFSR .NE. 0  .AND.  uu .GT. m_vlim2) THEN
         DO  jph2=2,nphoy
            DO jph1=1,jph2-1
               y1  = yfin(jph1) /(1 +yfin(jph1) +zfin(jph1) )
               z1  = zfin(jph1) /(1 +yfin(jph1) +zfin(jph1) )
               y2  = yfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
               z2  = zfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
*     Note y1,z1<1 (yfin,zfin cant be used directly in wmd(...))
               hfac1  =  m_ySfac(jph1)*wmd(delq,y1,z1)
               hfac2  =  m_ySfac(jph2)*wmd(delq,y2,z2)
*              sum over two FSR fragmentation trees
               ntree = 2  ! for 2 FSR fragmentation trees
               gf1 = 0d0
               gf2 = 0d0
               CALL QED3_Dfsr2(yfin,zfin,jph1,jph1,jph2,gf1,gf2) ! 1-st tree
               CALL QED3_Dfsr2(yfin,zfin,jph1,jph2,jph1,gf1,gf2) ! 2-nd tree
               gi1 = 0.5d0        ! 0.5d0 for averaging over 2 choices
               gi2 = 0.5d0        ! of Born angles in initial state
*---- O(alf2) ----, tree level
               dist20 =
     $              (gi1*gf1*andi11+ gi1*gf2*andi12
     $              +gi2*gf1*andi21+ gi2*gf2*andi22)
     $              *hfac1*hfac2/ntree
               beta20 = dist20 
     $              -m_Beta00*m_ySfac(jph1)*m_ySfac(jph2)
     $              -betf10(jph1)*m_ySfac(jph2) -betf10(jph2)*m_ySfac(jph1)
               betyy20(jph1,jph2)=beta20
               m_yyBet20=m_yyBet20 +beta20 /m_ySfac(jph1)/m_ySfac(jph2)
*---- O(alf3) ----, one loop level  !!!NEW
* Primitive ISR virtual correction only
               beta21 = (1d0+deli1)*beta20
               m_yyBet21=m_yyBet21 +beta21 /m_ySfac(jph1)/m_ySfac(jph2)
            ENDDO
         ENDDO
      ELSE
         DO  jph2=2,nphoy
            DO  jph1=1,nphoy
               betyy20(jph1,jph2)= 0d0
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta2 initial-final
*-----------------------------------------------------------
* or in other terminology   beta1_init - beta1_final
      m_xyBet20=0d0
      m_xyBet21=0d0
      IF(m_KeyISR*IsFSR .NE. 0 .AND. vv.GT.m_vlim1 .AND. uu.GT.m_vlim1) THEN
         DO jph1=1,nphox
            DO jph2=1,nphoy
               y1  = yini(jph1)
               z1  = zini(jph1)
               y2  = yfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
               z2  = zfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
               hfac1 =    m_xSfac(jph1) *wmd(delp,y1,z1)
               hfac2 =    m_ySfac(jph2) *wmd(delq,y2,z2)
               CALL QED3_Disr1(gami,yini,zini,jph1,
     $                  gi1,gi2,ggi1,ggi2,gggi1,gggi2)
               CALL QED3_Dfsr1(gamf,yfin,zfin,jph2,gf1,gf2,ggf1,ggf2)
*---- O(alf2) -----, tree level
               dist20 = 
     $              (gi1*gf1*andi11+ gi1*gf2*andi12
     $              +gi2*gf1*andi21+ gi2*gf2*andi22)*hfac1*hfac2
               beta20 = dist20 
     $              -m_Beta00*m_xSfac(jph1)*m_ySfac(jph2)
     $              -beti10(jph1)*m_ySfac(jph2) -betf10(jph2)*m_xSfac(jph1)
               betxy20(jph1,jph2)=beta20
               m_xyBet20=m_xyBet20 +beta20 /m_xSfac(jph1)/m_ySfac(jph2)
*---- O(alf3) -----, one loop level  !!!!!!!NEW
* Note that virtual correction is factorized ISR*FSR, as usual
               dist21 = 
     $              (ggi1*ggf1*andi11+ ggi1*ggf2*andi12
     $              +ggi2*ggf1*andi21+ ggi2*ggf2*andi22)*hfac1*hfac2
               beta21 = dist21 
     $              -m_Beta01*m_xSfac(jph1)*m_ySfac(jph2)
     $              -betx11(jph1)*m_ySfac(jph2) -bety11(jph2)*m_xSfac(jph1)
               m_xyBet21=m_xyBet21 +beta21 /m_xSfac(jph1)/m_ySfac(jph2)
            ENDDO
         ENDDO
      ELSE
         DO  jph1=1,nphox
            DO  jph2=1,nphoy
               betxy20(jph1,jph2)= 0d0
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta3 initial-initial-initial
*-----------------------------------------------------------
      m_xxxBet30=0d0
      m_sbti30=0d0
      IF(m_KeyISR .NE. 0  .AND.  vv .GT. m_vlim2) THEN
         DO jph3 = 3,nphox
            DO jph2 = 2,jph3-1
               DO jph1 = 1,jph2-1
                  hfac1  =  m_xSfac(jph1)*wmd(delp,yini(jph1),zini(jph1))
                  hfac2  =  m_xSfac(jph2)*wmd(delp,yini(jph2),zini(jph2))
                  hfac3  =  m_xSfac(jph3)*wmd(delp,yini(jph3),zini(jph3))
*      Summation over 6 LL fragmentation trees for 3 ISR photons,
*      photon jph1 is always harder because of low level M.C. generation
                  ntree = 6     ! for 2 ISR fragmentation trees
                  gi1 = 0d0
                  gi2 = 0d0
                  CALL QED3_Disr3(yini,zini,jph1, jph1,jph2,jph3, gi1,gi2)
                  CALL QED3_Disr3(yini,zini,jph1, jph2,jph1,jph3, gi1,gi2)
                  CALL QED3_Disr3(yini,zini,jph1, jph1,jph3,jph2, gi1,gi2)
                  CALL QED3_Disr3(yini,zini,jph1, jph2,jph3,jph1, gi1,gi2)
                  CALL QED3_Disr3(yini,zini,jph1, jph3,jph1,jph2, gi1,gi2)
                  CALL QED3_Disr3(yini,zini,jph1, jph3,jph2,jph1, gi1,gi2)
                  gf1 = 0.5d0   ! 0.5d0 for averaging over 2 choices
                  gf2 = 0.5d0   ! of Born angles in final state
*---- O(alf3) -----, tree level  !!!!!!!NEW
                  dist30 = 
     $                 (gi1*gf1*andi11+ gi1*gf2*andi12
     $                 +gi2*gf1*andi21+ gi2*gf2*andi22)
     $                 *hfac1*hfac2*hfac3/ntree
                  beta30 = dist30
     $                 -m_Beta00 *m_xSfac(jph1) *m_xSfac(jph2) *m_xSfac(jph3)
     $                 -beti10(jph1) *m_xSfac(jph2) *m_xSfac(jph3)
     $                 -beti10(jph2) *m_xSfac(jph1) *m_xSfac(jph3)
     $                 -beti10(jph3) *m_xSfac(jph1) *m_xSfac(jph2)
     $                 -betxx20(jph1,jph2) *m_xSfac(jph3)
     $                 -betxx20(jph1,jph3) *m_xSfac(jph2)
     $                 -betxx20(jph2,jph3) *m_xSfac(jph1)
                  m_xxxBet30 = m_xxxBet30 
     $                 +beta30/m_xSfac(jph1)/m_xSfac(jph2)/m_xSfac(jph3)
* Pure ISR, simply the same
                  m_sbti30 = m_sbti30 
     $                 +beta30/m_xSfac(jph1)/m_xSfac(jph2)/m_xSfac(jph3)
               ENDDO
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta3 initial-initial-final
*-----------------------------------------------------------
      m_xxyBet30 = 0d0
      IF(  m_KeyISR .NE. 0  .AND.  vv .GT. m_vlim2 .AND.
     $     IsFSR .NE. 0  .AND.  uu .GT. m_vlim1) THEN
         DO jph2=2,nphox
            DO jph1=1,jph2-1
               DO jph3=1,nphoy
                  hfac1  =  m_xSfac(jph1)*wmd(delp,yini(jph1),zini(jph1))
                  hfac2  =  m_xSfac(jph2)*wmd(delp,yini(jph2),zini(jph2))
                  y3  = yfin(jph3) /(1 +yfin(jph3) +zfin(jph3) )
                  z3  = zfin(jph3) /(1 +yfin(jph3) +zfin(jph3) )
                  hfac3  =  m_ySfac(jph3)*wmd(delq,y3,z3)
                  ntree = 2     ! for 2 ISR fragmentation trees
                  gi1  = 0d0    ! initialization
                  gi2  = 0d0    ! initialization
                  CALL QED3_Disr2(gami,yini,zini,jph1, jph1,jph2,
     $                                    gi1,gi2,ggi1,ggi2) ! 1-st tree
                  CALL QED3_Disr2(gami,yini,zini,jph1, jph2,jph1,
     $                                    gi1,gi2,ggi1,ggi2) ! 2-nd tree
                  CALL QED3_Dfsr1(gamf,yfin,zfin,jph3,gf1,gf2,ggf1,ggf2)
*---- O(alf3) -----, tree level  !!!!!!!NEW
                  dist30= (gi1*gf1*andi11 + gi1*gf2*andi12
     $                    +gi2*gf1*andi21 + gi2*gf2*andi22)
     $                    *hfac1*hfac2*hfac3/ntree
                  beta30 = dist30 
     $                 -m_Beta00 *m_xSfac(jph1) *m_xSfac(jph2) *m_ySfac(jph3)
     $                 -beti10(jph1) *m_xSfac(jph2) *m_ySfac(jph3)
     $                 -beti10(jph2) *m_xSfac(jph1) *m_ySfac(jph3)
     $                 -betf10(jph3) *m_xSfac(jph1) *m_xSfac(jph2)
     $                 -betxx20(jph1,jph2) *m_ySfac(jph3)
     $                 -betxy20(jph1,jph3) *m_xSfac(jph2)
     $                 -betxy20(jph2,jph3) *m_xSfac(jph1)
                  m_xxyBet30 = m_xxyBet30 
     $                 +beta30/m_xSfac(jph1)/m_xSfac(jph2)/m_ySfac(jph3)
               ENDDO
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------
*                beta3 initial-final-final
*-----------------------------------------------------------
      m_xyyBet30 = 0d0
      IF(  m_KeyISR .NE. 0  .AND.  vv .GT. m_vlim2 .AND.
     $      IsFSR .NE. 0  .AND.  uu .GT. m_vlim1) THEN
         DO  jph2=2,nphoy
            DO jph1=1,jph2-1
               DO jph3=1,nphox
                  y1  = yfin(jph1) /(1 +yfin(jph1) +zfin(jph1) )
                  z1  = zfin(jph1) /(1 +yfin(jph1) +zfin(jph1) )
                  y2  = yfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
                  z2  = zfin(jph2) /(1 +yfin(jph2) +zfin(jph2) )
                  hfac1  =  m_ySfac(jph1)*wmd(delq,y1,z1)
                  hfac2  =  m_ySfac(jph2)*wmd(delq,y2,z2)
                  hfac3  =  m_xSfac(jph3)*wmd(delp,yini(jph3),zini(jph3))
*     sum over two FSR fragmentation trees
                  ntree = 2    ! for 2 FSR fragmentation trees
                  gf1 = 0d0    ! initialization
                  gf2 = 0d0    ! initialization
                  CALL QED3_Dfsr2(yfin,zfin,jph1, jph1,jph2, gf1,gf2)
                  CALL QED3_Dfsr2(yfin,zfin,jph1, jph2,jph1, gf1,gf2)
                  CALL QED3_Disr1(gami,yini,zini,jph3,
     $                  gi1,gi2,ggi1,ggi2,gggi1,gggi2)
*---- O(alf3) -----, tree level  !!!!!!!NEW
                  dist30= (gi1*gf1*andi11 + gi1*gf2*andi12
     $                    +gi2*gf1*andi21 + gi2*gf2*andi22)
     $                    *hfac1*hfac2*hfac3/ntree
                  beta30 = dist30 
     $                 -m_Beta00 *m_ySfac(jph1) *m_ySfac(jph2) *m_xSfac(jph3)
     $                 -betf10(jph1) *m_ySfac(jph2) *m_xSfac(jph3)
     $                 -betf10(jph2) *m_ySfac(jph1) *m_xSfac(jph3)
     $                 -beti10(jph3) *m_ySfac(jph1) *m_ySfac(jph2)
     $                 -betyy20(jph1,jph2) *m_xSfac(jph3)
     $                 -betxy20(jph3,jph1) *m_ySfac(jph2)
     $                 -betxy20(jph3,jph2) *m_ySfac(jph1)
                  m_xyyBet30 = m_xyyBet30 
     $                 +beta30/m_ySfac(jph1)/m_ySfac(jph2)/m_xSfac(jph3)
               ENDDO
            ENDDO
         ENDDO
      ENDIF
*-----------------------------------------------------------------
*     Finite part of the YFS formfactor for the ISR/FSR
*-----------------------------------------------------------------
      CALL  BornV_GetYFSkon(ForIni)
      CALL KarFin_GetYFSkon(ForFin)
      IF(m_KeyISR .EQ. 0)   ForIni=1d0
      IF(   IsFSR .EQ. 0)   ForFin=1d0
      fYFS = ForIni*ForFin
*-----------------------------------------------------------------
*     and the rejection weights = (new.distr/crude.distr)
*-----------------------------------------------------------------
*     ============================================
*     ========== INITIAL + FINAL =================
*     ============================================
*
* Note that m_xyBet20 (which is genuine O(alf2)) is added to O(alf1),
* because our semianalytical programs are only able to deal 
* with factorized ini/fin, see also O(alf1) definitions of beta1's.
*
* Total's, all beta's ---------------------------------
      m_WtSet(71) =   fYFS*  m_Beta00/DisCru
      m_WtSet(72) =   fYFS*( m_Beta01 +m_xBet10 +m_yBet10 +m_xyBet20)/DisCru
      m_WtSet(73) =   fYFS*( m_Beta02 +m_xBet11 +m_yBet11 
     $                      +m_xxBet20 +m_xyBet20 +m_yyBet20 )/DisCru
* !!!NEW!!!
      m_WtSet(74) =   fYFS*( m_Beta03 +m_xBet12 +m_yBet12
     $                    +m_xxBet21 +m_xyBet21 +m_yyBet21
     $                    +m_xxxBet30 +m_xxyBet30 +m_xyyBet30 )/DisCru
* First order, individual beta's -------------
      m_WtSet(80) =   fYFS*m_Beta01/DisCru
      m_WtSet(81) =   fYFS*(m_xBet10+m_yBet10)/DisCru
      m_WtSet(82) =   fYFS*(m_xBet10)/DisCru
      m_WtSet(83) =   fYFS*(m_yBet10)/DisCru
      m_WtSet(84) =   fYFS*(m_xyBet20)/DisCru
* Second order, individual beta's ------------
      m_WtSet(90) =   fYFS*m_Beta02/DisCru
      m_WtSet(91) =   fYFS*(m_xBet11+m_yBet11)/DisCru
      m_WtSet(92) =   fYFS*(m_xxBet20+m_xyBet20+m_yyBet20)/DisCru
      m_WtSet(93) =   fYFS*(m_xBet11)/DisCru
      m_WtSet(94) =   fYFS*(m_yBet11)/DisCru
      m_WtSet(95) =   fYFS*(m_xxBet20)/DisCru
      m_WtSet(96) =   fYFS*(m_xyBet20)/DisCru
      m_WtSet(97) =   fYFS*(m_yyBet20)/DisCru
* Third order, individual beta's ------------!!!NEW!!!
      m_WtSet(100) =   fYFS*m_Beta03/DisCru
      m_WtSet(101) =   fYFS*(m_xBet12 +m_yBet12)/DisCru
      m_WtSet(102) =   fYFS*(m_xxBet21+m_xyBet21+m_yyBet21)/DisCru
      m_WtSet(103) =   fYFS*m_xBet12/DisCru
      m_WtSet(104) =   fYFS*m_yBet12/DisCru
      m_WtSet(105) =   fYFS*m_xxBet21/DisCru
      m_WtSet(106) =   fYFS*m_xyBet21/DisCru
      m_WtSet(107) =   fYFS*m_yyBet21/DisCru
      m_WtSet(108) =   fYFS*(m_xxxBet30+m_xxyBet30+m_xyyBet30)/DisCru
      m_WtSet(109) =   fYFS*m_xxxBet30/DisCru
      m_WtSet(110) =   fYFS*m_xxyBet30/DisCru
      m_WtSet(111) =   fYFS*m_xyyBet30/DisCru

*     ============================================
*     ========= INITIAL STATE ALONE ==============
*     ============================================
* Total's, all beta's ---------------------------------
      m_WtSet( 1) =   ForIni* m_beti00/DisCru
      m_WtSet( 2) =   ForIni*(m_beti01+m_sbti10)/DisCru
      m_WtSet( 3) =   ForIni*(m_beti02+m_sbti11+m_sbti20)/DisCru
!!!NEW
      m_WtSet( 4) =   ForIni*(m_beti03+m_sbti12+m_sbti21+m_sbti30)/DisCru
* First order, individual beta's -------------
      m_WtSet(10) =   ForIni*m_beti01/DisCru
      m_WtSet(11) =   ForIni*m_sbti10/DisCru
* Second order, individual beta's ------------
      m_WtSet(20) =   ForIni*m_beti02/DisCru
      m_WtSet(21) =   ForIni*m_sbti11/DisCru
      m_WtSet(22) =   ForIni*m_sbti20/DisCru
!!!NEW
* Third order, individual beta's ------------
      m_WtSet(30) =   ForIni*m_beti03/DisCru
      m_WtSet(31) =   ForIni*m_sbti12/DisCru
      m_WtSet(32) =   ForIni*m_sbti21/DisCru
      m_WtSet(33) =   ForIni*m_sbti30/DisCru
*
*//=================================================================//
*//            ISR   Non-exponentiated version                      //
*//            Not yet fully implemented......                      //
*//=================================================================//
* Entire 0,1,2-photon distributions
      fYFSu = exp( -gami*dlog(1/m_vvmin) )
      m_dis0   =0d0
      m_dis1   =0d0
      m_dis2   =0d0
      m_dig1   =0d0
      IF( (nphox+nphoy) .EQ. 0) THEN
         m_dis0 = 1
         m_dis1 = 1+gami*dlog(m_vvmin)
         m_dis2 = 1+gami*dlog(m_vvmin)+0.5*(gami*dlog(m_vvmin))**2
      ELSEIF( nphox .EQ. 1) THEN
         y = yini(1)
         z = zini(1)
         gf1 = 0.5d0
         gf2 = 0.5d0
         gi1 = ((1-y)**2)/2d0
         gi2 = ((1-z)**2)/2d0
         Bor1=  gi1*gf1*andi11   +gi1*gf2*andi12 
     $         +gi2*gf1*andi21   +gi2*gf2*andi22
         m_dis1 = Bor1  *wmd(delp,y,z)            !! S-factor divided off
* standard O(alf1) for comparisons with spinor methods
         m_dig1 = Bor1  *2d0/(y*z)*wm1(delp,y,z)*svar/svar1
         m_dis2 = m_dis1*(1 +gami*dlog(m_vvmin))
cc         m_dis1 = 1                       !!!! blank matrix elm.
cc         m_dis2 = 1 +gami*dlog(m_vvmin)   !!!! blank matrix elm.
      ELSEIF( nphoy .EQ. 1) THEN
         yy = yfin(1)
         zz = zfin(1)
         y  = yy/(1 +yy+zz)
         z  = zz/(1 +yy+zz)
         gi1 = 0.5d0
         gi2 = 0.5d0
         gf2 = ((1-y)**2 ) /2d0 !!! y,z are swapped! correct d_fsr !!!!
         gf1 = ((1-z)**2 ) /2d0
         Bor1=    gi1*gf1*andi11   +gi1*gf2*andi12 
     $           +gi2*gf1*andi21   +gi2*gf2*andi22
* standard O(alf1) for comparisons with spinor methods
         m_dig1 = Bor1  *2d0/(y*z)*wm1(delq,y,z)
         m_dis1 = Bor1  *wmd(delq,y,z)            !! S-factor divided off
c{{{
c         IF((y+z).GT.0.9d0) THEN
c            WRITE(*,'(a)') '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
c            DO k=1,4
c               ph(k) = yphot(1,k)
c            ENDDO
c            kq1 = 2*(ph(4)*qf1(4)-ph(3)*qf1(3)-ph(2)*qf1(2)-ph(1)*qf1(1))
c            kq2 = 2*(ph(4)*qf2(4)-ph(3)*qf2(3)-ph(2)*qf2(2)-ph(1)*qf2(1))
c            p1p2= 2*(pf1(4)*pf2(4)-pf1(3)*pf2(3)-pf1(2)*pf2(2)-pf1(1)*pf2(1))
c            q1q2= 2*(qf1(4)*qf2(4)-qf1(3)*qf2(3)-qf1(2)*qf2(2)-qf1(1)*qf2(1))
c            tt  = 2*(pf1(4)*qf1(4)-pf1(3)*qf1(3)-pf1(2)*qf1(2)-pf1(1)*qf1(1))
c            uu  = 2*(pf1(4)*qf2(4)-pf1(3)*qf2(3)-pf1(2)*qf2(2)-pf1(1)*qf2(1))
c            tt1 = 2*(pf2(4)*qf2(4)-pf2(3)*qf2(3)-pf2(2)*qf2(2)-pf2(1)*qf2(1))
c            uu1 = 2*(pf2(4)*qf1(4)-pf2(3)*qf1(3)-pf2(2)*qf1(2)-pf2(1)*qf1(1))
c            WRITE(*,'(a,5g20.10)') 'yy ',yy,kq1/q1q2
c            borc = (tt**2+uu**2+tt1**2+uu1**2)/p1p2**2
c            sofc = 2d0*p1p2**2/kq1/kq2
c            WRITE(*,'(a,5g20.10)') 'borc    ', borc,Bor1,borc/bor1
c            WRITE(*,'(a,5g20.10)') 'sofc    ', sofc, 2/(y*z)
c            dig1 =  2d0 *(tt**2+uu**2+tt1**2+uu1**2)/kq1/kq2
c            WRITE(*,'(a,5g20.10)')  'dig1  ', dig1, m_dig1, dig1/m_dig1
c         ENDIF
c}}}
      ELSEIF( nphox .EQ. 2) THEN
         m_dis2 = wm0(delp,yini(1),zini(1)) *wm0(delp,yini(2),zini(2))
cc         m_dis2 = 1             !!!! blank matrix elm.
      ENDIF
***
* UNEXP Total O(alf0),O(alf1),O(alf2)
      m_WtSet(160) =    m_dis0 /fYFSu
      m_WtSet(161) =    m_dis1 /fYFSu
      m_WtSet(162) =    m_dis2 /fYFSu

*|=================================================================|
*|        Model weight (the best)                                  |
*|=================================================================|
      m_WtBest = m_WtSet(m_IdeWgt)


*********[[[[[[[[[[[[*********DEBUG*********
c      wtm2= m_WtSet(73)*wtcrud
c      wtm1= m_WtSet(72)*wtcrud
c      wtm0= m_WtSet(71)*wtcrud
**      IF(wtm1  .LT.  0d0 .OR. wtm1  .GT.  5d0) THEN
c      IF(wtm2  .LT.  0d0 .OR. wtm2  .GT.  5d0) THEN
c         WRITE(6,*) '++++++++++++++++++++++++++++++++++++++++++++++++++'
c         WRITE(6,*) 'icont,kffin= ',icont,kffin
c         WRITE(6,*) 'nphox,nphoy,vv,uu=',nphox,nphoy,vv,uu
c         WRITE(6,*) 'wtm2,wtm1,wtm0= ',wtm2,wtm1,wtm0
c         WRITE(6,*) 'm_WtSet(71),wtcrud= ',m_WtSet(71),wtcrud
**         CALL dumps(6)
**         CALL dumpi(6)
**         CALL dumpf(6)
c      ENDIF
*********]]]]]]]]]]]]*********DEBUG*********
      END


      SUBROUTINE QED3_bvirt0(alfinv,charg2,svar,am,dels1,dels2,dels3)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// ISR/FSR virtual corrections to beta0                                            //
*// beta0 is equal born*(1+dels1+dels2+dels3)                                       //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION alfinv,charg2,svar,am,dels1,dels2,dels3
* locals
      DOUBLE PRECISION pi,zet2,zet3
      PARAMETER(pi=3.1415926535897932d0)
      PARAMETER(zet2= pi**2/6d0)
      PARAMETER(zet3= 1.2020569031595942854d0)
      DOUBLE PRECISION gami,bilg,alfpi,Mlog
*-------------------------------------------------------------------------------------
      DOUBLE COMPLEX     F1_1, F1_2, cL
*-------------------------------------------------------------------------------------
      alfpi =  1d0/alfinv/pi
      bilg  =  DLOG(svar/am**2)
      Mlog  =  DLOG(svar/am**2) -1d0
      gami  =  2*charg2*alfpi*(bilg-1)
      dels1 =  gami/2d0
**    dels2 =  1/2d0 *(gami/2d0)**2
* ISR with subleading terms from Berends, Burgers, Van Neerveen
* (the effect of including NLL is negligible, below 1d-4)
      dels2 =  
     $      charg2**2*alfpi**2  *0.5d0*bilg**2                 ! LL
**     $     +charg2**2*alfpi**2*(
**     $       -(13d0/16d0 +1.5d0*zet2 -3d0*zet3)*bilg           ! NLL
**     $       -16d0/5d0*zet2*zet2 +51d0/8d0*zet2 +13d0/4d0      ! NNLL
**     $       -4.5d0*zet3 -6d0*zet2*log(2d0)                    ! NNLL
**     $      )
      dels3 = 1/6d0 *(gami/2d0)**3
***/////////////////////////////////////////////////////////////////////////
*** The assignements below will get together O(alf1)CEEX and O(alf1)EEX
*** but it will spoil O(alf2)EEX because dels1 is also input for beta1
*      cL    = DCMPLX( DLOG(Svar/Am**2)-1d0, -1d0 )
*      F1_1  = Alfpi*charg2   *0.5d0*cL
*      dels1 = CDABS(1+ F1_1)**2 -1d0
*      F1_2 = F1_1
*     $     +(Alfpi*charg2)**2 *(
*     $              +cL**2/8d0 
*     $              +cL*( 3d0/32 -3d0/4*zet2 +3d0/2*zet3 ) 
*     $     )
*      dels2 = CDABS(1+ F1_2)**2 -(1d0+dels1)
***/////////////////////////////////////////////////////////////////////////
      END                       !!!QED3_bvirt0!!!

      SUBROUTINE QED3_Disr1(gami,y,z,j1,g1,g2,gg1,gg2,ggg1,ggg2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Ingredients for O(alf3)NLL ISR matrix element.                                  //
*// INPUT:                                                                          //
*//     alfinv=  QED coupling                                                       //
*//     charg2=  charge squared                                                     //
*//     y,z=     Sudakov variables                                                  //
*//     j1=      pointers to input-photon                                           //
*// OUTPUT:                                                                         //
*//     g's are set here: g=treelevel, gg=oneloop, ggg=twoloop                      //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  gami,g1,g2,gg1,gg2,ggg1,ggg2
      INTEGER           j1
      DOUBLE PRECISION  y(*),z(*)
* locals
      DOUBLE PRECISION  dels1,dels2,zz,a1,b1
*-------------------------------------------------------------------------------------
      a1 = y(j1)
      b1 = z(j1)
      zz = (1d0-a1)*(1d0-b1)
      IF(zz  .le.0d0) WRITE(*,*) '!!!! zz=',zz
      dels1 =  gami/2d0 -gami/4d0*dlog(zz)
      dels2 =  gami**2/8d0
     $        -gami**2/8d0  *dlog(zz)
     $        +gami**2/24d0 *dlog(zz)**2
* Exact O(alf1) matrix element for the hardest photon jhard
      g1   = ((1-a1)**2            )/2d0
      g2   = (            (1-b1)**2)/2d0
      gg1  = ((1-a1)**2            )/2d0 *(1+dels1)
      gg2  = (            (1-b1)**2)/2d0 *(1+dels1)
      ggg1 = ((1-a1)**2            )/2d0 *(1+dels1+dels2)
      ggg2 = (            (1-b1)**2)/2d0 *(1+dels1+dels2)
      END

      SUBROUTINE QED3_Disr2(gami,y,z,jhard,j1,j2,g1,g2,gg1,gg2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Ingredients for O(alf2)NLL ISR matrix element.                                  //
*// INPUT:                                                                          //
*//     gami  = 2*alfa/pi*(BigLog-1)                                                //
*//     y,z   = Sudakov variables                                                   //
*//     jhard = pointer of hardes photon                                            //
*//     j1,j2 = pointers of two input-photons                                       //
*// OUTPUT:                                                                         //
*//     g's, gg's are updated (have to be initialized in calling program)           //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  y(*),z(*)
      DOUBLE PRECISION  gami,g1,g2,gg1,gg2
      INTEGER           jhard,j1,j2
* locals
      DOUBLE PRECISION  a1,z1,p2,delvir1,z1z2,a2,b1,p1,b2
*-------------------------------------------------------------------------------------
      a1 = y(j1)
      b1 = z(j1)
      a2 = y(j2)/(1d0-y(j1))
      b2 = z(j2)/(1d0-z(j1))
* Exact O(alf1) matrix element for the hardest photon jhard
      IF(jhard .EQ. j1) THEN
         p1= ((1-a1)**2            ) *( (1-a2)**2 + (1-b2)**2 )/4d0
         p2= (            (1-b1)**2) *( (1-a2)**2 + (1-b2)**2 )/4d0
      ELSE
         p1= ((1-a1)**2 +(1-b1)**2 ) *( (1-a2)**2             )/4d0
         p2= ((1-a1)**2 +(1-b1)**2 ) *(             (1-b2)**2 )/4d0
      ENDIF
      g1 = g1 +p1
      g2 = g2 +p2
      z1 =  (1-y(j1))*(1-z(j1))
      z1z2= (1-y(j1)-y(j2))*(1-z(j1)-z(j2))
* soft limit to QED3_Disr1 OK, for 2 trees we get 3 terms gami/6d0*dlog(zz)
      delvir1 = gami/2d0 -gami/6d0*dlog(z1) -gami/6d0*dlog(z1z2)
      gg1=gg1 +p1*(1+delvir1)
      gg2=gg2 +p2*(1+delvir1)

      IF(z1  .le.0d0) WRITE(*,*) '!!!! z1=',z1
      IF(z1z2.le.0d0) WRITE(*,*) '!!!! z1z2=',z1z2
      END


      SUBROUTINE QED3_Disr3(y,z,jhard,j1,j2,j3,g1,g2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Ingredients for O(alf3)LL ISR matrix element.                                   //
*// INPUT:                                                                          //
*//     y,z Sudakov variables                                                       //
*//     jhard pointer of hardes photon                                              //
*//     j1,j2,j3 pointers of 3 input-photons                                        //
*// OUTPUT:                                                                         //
*//     g1,g2 are updated (have to be initialized in calling program)               //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION  y(*),z(*)
      DOUBLE PRECISION  g1,g2
      INTEGER           jhard,j1,j2,j3
* locals
      DOUBLE PRECISION  chi1,chi2,b3,a3,p2,p1,b1,a1,b2,a2,a,b
* inline functions
      chi1(a  )=  0.5d0*    (1d0-a)**2
      chi2(a,b)=  0.5d0*   ((1d0-a)**2+(1d0-b)**2)
*-------------------------------------------------------------------------------------
      a1 = y(j1)
      b1 = z(j1)
      a2 = y(j2)/(1d0-y(j1))
      b2 = z(j2)/(1d0-z(j1))
      a3 = y(j3)/(1d0-y(j2)-y(j1))
      b3 = z(j3)/(1d0-z(j2)-z(j1))
* Exact O(alf1) matrix element for the hardest photon jhard
      IF(jhard .EQ. j1) THEN
         p1= chi1(a1) *chi2(a2,b2) *chi2(a3,b3)
         p2= chi1(b1) *chi2(a2,b2) *chi2(a3,b3)
      ELSEIF(jhard .EQ. j2) THEN
         p1= chi2(a1,b1) *chi1(a2) *chi2(a3,b3)
         p2= chi2(a1,b1) *chi1(b2) *chi2(a3,b3)
      ELSE
         p1= chi2(a1,b1) *chi2(a2,b2) *chi1(a3)
         p2= chi2(a1,b1) *chi2(a2,b2) *chi1(b3)
      ENDIF
      g1=g1 +p1
      g2=g2 +p2
      END                       !!!QED3_Disr3!!!


      SUBROUTINE QED3_Dfsr1(gamf,y,z,j1,g1,g2,gg1,gg2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Ingredients for O(alf2)NLL FSR matrix element.                                  //
*// INPUT:                                                                          //
*//     y,z Sudakov variables                                                       //
*//     j1 pointer of input-photons                                                 //
*// OUTPUT:                                                                         //
*//     g's are set here                                                            //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER           j1
      DOUBLE PRECISION  gamf,g1,g2,gg1,gg2
      DOUBLE PRECISION  y(*),z(*)
* locals
      DOUBLE PRECISION  zz,dels1,a1,b1
*-------------------------------------------------------------------------------------
* normal definition as in O(alf1) single-photon case
      a1 = y(j1)/( 1d0 +y(j1) +z(j1) )
      b1 = z(j1)/( 1d0 +y(j1) +z(j1) )
      zz = (1d0-a1)*(1d0-b1)
      IF(zz  .LE. 0d0) WRITE(*,*) '!!!! zz=',zz
      dels1 =  gamf/2d0 +gamf/4d0*dlog(zz)
* Exact O(alf1) matrix element for the hardest photon jhard
      g2 = ((1-a1)**2            ) /2d0              ! corrected
      g1 = (            (1-b1)**2) /2d0              ! corrected
      gg2= ((1-a1)**2            ) /2d0 *(1+dels1)   ! corrected
      gg1= (            (1-b1)**2) /2d0 *(1+dels1)   ! corrected
      END                       !!!QED3_Dfsr1!!!

      SUBROUTINE QED3_Dfsr2(y,z,jhard,j1,j2,g1,g2)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Ingredients for O(alf2)NLL FSR matrix element.                                  //
*// INPUT:                                                                          //
*//     y,z Sudakov variables                                                       //
*//     jhard pointer of hardes photon                                              //
*//     j1,j2 pointers of two input-photons                                         //
*// OUTPUT:                                                                         //
*//     g1,g2 are updated (have to be initialized in calling program)               //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER           jhard,j1,j2
      DOUBLE PRECISION  g1,g2
      DOUBLE PRECISION  y(*),z(*)
* locals
      DOUBLE PRECISION  b2,a2,p1,p2,b1,a1,zp2,yp2
*-------------------------------------------------------------------------------------
* normal definition as in O(alf1) single-photon case
      a1 = y(j1)/( 1d0 +y(j1) +z(j1) )
      b1 = z(j1)/( 1d0 +y(j1) +z(j1) )
* take into account primary photon emission
      yp2 = y(j2)/( 1d0 +y(j1) )
      zp2 = z(j2)/( 1d0 +z(j1) )
* as in O(alf1) single-photon case
      a2 = yp2/(1d0 + yp2 +zp2)
      b2 = zp2/(1d0 + yp2 +zp2)

* Exact O(alf1) matrix element for the hardest photon jhard
      IF(jhard .EQ. j1) THEN
         p2= ((1-a1)**2            ) *( (1-a2)**2 + (1-b2)**2 )/4d0 ! corrected
         p1= (            (1-b1)**2) *( (1-a2)**2 + (1-b2)**2 )/4d0 ! corrected
      ELSE
         p2= ((1-a1)**2 +(1-b1)**2 ) *( (1-a2)**2             )/4d0 ! corrected
         p1= ((1-a1)**2 +(1-b1)**2 ) *(             (1-b2)**2 )/4d0 ! corrected
      ENDIF
      g1=g1 +p1
      g2=g2 +p2
      END


      SUBROUTINE  QED3_wtPrint(txt,nout,iev,ie1,ie2,wt_ISR,wt_FSR,wtbest,wtset)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*// Prints out all weights                                                          //
*// and the serial number of event iev on unit nout                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER           nout,iev,ie1,ie2
      DOUBLE PRECISION  wt_ISR,wt_FSR,wtbest
      DOUBLE PRECISION  wtset(*)
      CHARACTER*8 txt
* locals
      INTEGER           i
      DOUBLE PRECISION  wtmain,wtcrud
*-------------------------------------------------------------------------------------
      IF( (iev .GE. ie1) .AND. (iev .LE. ie2) ) THEN
         WRITE(nout,*) 
     $        '=========== ',txt,' =======weights========>',iev
         wtcrud  = wt_ISR*wt_FSR
         wtmain  = wtcrud*wtbest
         WRITE(nout,3000) 'wtmain,wt_ISR,wt_FSR,wtbest= ',
     $                     wtmain,wt_ISR,wt_FSR,wtbest
         WRITE(nout,3100) (' (',i,')=',wtset(i)*wtcrud, i=1,150)
         WRITE(nout,*) '   '
      ENDIF
 3000 FORMAT(a,4f18.14)
 3100 FORMAT(4(a3,i3,a2,f18.14))
      END                       !!!QED3_wtPrint!!!

      SUBROUTINE QED3_GetWtSet(WtBest,WtSet)
*/////////////////////////////////////////////////////////////////////////////////////
*//   Export list of weights                                                        //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'QED3.fi'
      DOUBLE PRECISION    WtBest,WtSet(*)
* locals
      INTEGER  j
*--------------------------------------------------------------
      WtBest = m_WtBest
* collection of all weights
      DO j=1,m_lenwt
         WtSet(j)= m_WtSet(j)
      ENDDO
      END                       !!!QED3_GetWtSet!!!

      SUBROUTINE QED3_GetDig1(dig1)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//   Provides O(alf1) ISR matrix element for tests                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INCLUDE 'QED3.fi'
      DOUBLE PRECISION   dig1
*---------------------------------------------------------------
      dig1= m_dig1
      END                       !!!QED3_GetDig1!!!

*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//              End of Pseudo-CLASS  QED3                                          //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////

