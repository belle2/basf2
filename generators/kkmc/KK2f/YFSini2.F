*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  YFSini2                                //
*//                                                                          //
*//                  !!!!! SPECIAL TESTS !!!!!!                              //
*//                                                                          //
*//   Purpose:  ISR photon emission, photon multiplicity and momenta         //
*//   ====================================================================== //
*//   Simplistic ISR generation without modulation of s', for SPECIAL TESTS. //
*//   (!!!!No dilatation !!!!)                                               //
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//  The algorithm in this subprogram was ALSO described in:                 //
*//  ``Yennie-Frautschi-Suura soft photons in Monte Carlo event generators'' //
*//             Unpublished report by S. Jadach,                             //
*//          MPI-Munchen, MPI-PAE/PTh 6/87, Jan. 1987.                       //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////


      SUBROUTINE YFSini2_Initialize( amel,alfinv,vvmin,nmax,out,KeyWtm,MltISR)
*
      INCLUDE 'YFSini2.fi'
      SAVE
*
      DOUBLE PRECISION    amel,alfinv,vvmin
      INTEGER  nmax,out,KeyWtm,MltISR
*
      m_amel    = amel
      m_alfinv  = alfinv
      m_vvmin   = vvmin
      m_nmax    = nmax
      m_out     = out
      m_KeyWtm  = KeyWtm
      m_MltISR  = MltISR
      END


      SUBROUTINE YFSini2_Make(CMSene,                   ! Input
     $     vv,p1,p2,nphot,sphot,sphum,ygr,zet,qq,wtini) ! Output
*     *************************************************************
      IMPLICIT NONE
      DOUBLE PRECISION  pi
      PARAMETER( pi=3.1415926535897932d0)
      DOUBLE PRECISION  CMSene,vv,sphum(4),sphot(100,4),ygr(100),zet(100)
*
      INCLUDE 'KarLud.fi'
*
      DOUBLE PRECISION  xphot(100,4)    ! photon momenta before rescaling
      DOUBLE PRECISION  pp(4),pk(4)
      DOUBLE PRECISION  p1(4),p2(4),qq(4),xph(100)
      DOUBLE PRECISION  rrdummy(100)
      REAL              rr(100)
      SAVE
*
      DOUBLE PRECISION   eta1,eta2,beta
      DOUBLE PRECISION   dist0,dist1,cg,sg,sprim,gami,gami2
      DOUBLE PRECISION   wtm,xk,vmax,am2,ene,average
      DOUBLE PRECISION   phi,wt_cut,wtini,wt_mas
      DOUBLE PRECISION   alf1,del1,del2
      REAL               rvec(10)
      INTEGER i,j,k,nphot
*
      alf1 = 1d0/pi/m_alfinv
      ene  = CMSene/2d0
* Define 4-momenta of the initial charged particles (emitters)
      CALL KinLib_givpair(CMSene,m_amel,m_amel,p1,p2,beta,eta1,eta2)
* Here gami2 must be used instead of gami (mass term neglected)
      am2  = (m_amel/ene)**2
      beta = sqrt(1d0-am2)
      gami  = 2d0*alf1* (dlog((1+beta)**2/am2) - 1d0)
      gami2 = 2d0*alf1*  dlog((1+beta)**2/am2)
      IF(m_KeyWtm .EQ. 1) gami2 = gami
      wt_mas  = (1d0/m_vvmin)**(gami2-gami)
      DO i=1,m_nmax
         xph(i)=0d0
         ygr(i)=0d0
         zet(i)=0d0
         DO j=1,4
            xphot(i,j)=0d0
            sphot(i,j)=0d0
         ENDDO
      ENDDO
* generate photon multiplicity
      average = gami2*dlog(1d0/m_vvmin)
      CALL YFSini2_PoissGen(average,m_nmax,nphot,rrdummy)

* refill rr in order to avoid ordering
      IF(nphot .GT. 0) CALL PseuMar_MakeVec(rr,nphot)

      wt_cut  = 1d0
      DO i=1,nphot
         xph(i)=(m_vvmin)**rr(i)
      ENDDO
      DO i=1,nphot
         xk=xph(i)
*****    CALL YFSini2_angbre(am2,del1,del2,cg,sg,dist0,dist1)    ! standard
         CALL YFSini2_angbre2(i,am2,del1,del2,cg,sg,dist0,dist1) ! test
         wtm=dist1/dist0
         wt_mas    =wt_mas*wtm
         CALL PseuMar_MakeVec(rvec,1)
         phi=2d0*pi*rvec(1)
         xphot(i,1)=xk*sg*cos(phi)
         xphot(i,2)=xk*sg*sin(phi)
         xphot(i,3)=xk*cg
         xphot(i,4)=xk
         ygr(i)    =xk*del1/2d0
         zet(i)    =xk*del2/2d0
      ENDDO
*
* photon momenta rescaled into GEV units
      DO j=1,4
         sphum(j)=0d0
      ENDDO
      DO  i=1,nphot
         DO  j=1,4
            sphot(i,j)=xphot(i,j)*ene
            sphum(j)=sphum(j)+sphot(i,j)
         ENDDO
      ENDDO

* 4-momentum left after photon emission
      DO k=1,4
         qq(k)=-sphum(k)
      ENDDO
      qq(4)=qq(4)+CMSene
      sprim = qq(4)**2 -qq(3)**2 -qq(2)**2 -qq(1)**2
      vv = 1d0 -sprim/CMSene**2
      IF( qq(4) .LE. 0d0) wt_cut  = 0d0  !!!! important !!!!
cc      IF( vv .GT. vmax)   wt_cut  = 0d0 ???? vmax undefined !!!!

* Total ISR weight
      IF(m_KeyWtm .EQ. 1) wt_mas=1d0
      wtini = wt_mas *wt_cut   ! =ypar(251)
*------------------------------------
*  for debug
      CALL KK2f_SetOneY(252,wt_mas)
      CALL KK2f_SetOneY(254,wt_cut)
      CALL KK2f_SetOneY( 10,vv)
      CALL KK2f_SetOneY(255,vv)
*------------------------------------
      END  !yfs_ini2

      SUBROUTINE YFSini2_AngBre(am2,del1,del2,costhg,sinthg,dist0,dist1)
*     *****************************************************************
* This routine generates photon angular distribution
* in the rest frame of the fermion pair.
* The distribution is the S-factor without mass term,
* i.e. without terms 2p_1p_2/(kp_1)(kp_2)
* Fermion mass is treated exactly!
* INPUT:  
*     am2 = 4*massf**2/s where massf is fermion mass
*     and s is effective mass squared of the parent fermion-pair.
* OUTPUT:
*     del1= 1-beta*cos(theta)
*     del2= 1+beta*cos(theta)
*     costhg, sinthg, cos and sin of the photon
*     angle with respect to fermions direction
*     dist0 = distribution generated, without m**2/(kp)**2 terms
*     dist1 = distribution with m**2/(kp)**2 terms
*     ***************************************
      IMPLICIT NONE
      DOUBLE PRECISION  am2,del1,del2,costhg,sinthg,dist0,dist1
* locals
      REAL              rn(10)
      DOUBLE PRECISION  a,eps,beta
*------------------------------------------------------------------------------
      CALL PseuMar_MakeVec(rn,2)
      beta =sqrt(1.d0-am2)
      eps  =am2/(1.d0+beta)                     != 1-beta
      del1 =(2.d0-eps)*(eps/(2.d0-eps))**rn(1)  != 1-beta*costhg
      del2 =2.d0-del1                           != 1+beta*costhg
* calculation of sin and cos theta from internal variables
      costhg=(del2-del1)/(2*beta)               ! exact
      sinthg=sqrt(del1*del2-am2*costhg**2)      ! exact
* symmetrization
      IF(rn(2) .LE. 0.5d0) THEN
        a=del1
        del1=del2
        del2=a
        costhg= -costhg
      ENDIF
      dist0=1d0/(del1*del2)*(1d0 -am2/2d0)
      dist1=1d0/(del1*del2) 
     $     *(1d0 -am2/2d0 -am2/4d0*(del1/del2+del2/del1))
* totaly equivalent formula is the following
*     dist1=1d0/(del1*del2)   *beta*sinthg**2/(del1*del2)
      END

      SUBROUTINE YFSini2_angbre2(iph,am2,del1,del2,costhg,sinthg,dist0,dist1)
*     **********************************************************
* modified version of angbre, added iph argument which enforces
* forward or backward peak in photon distribution
*     ***************************************
      IMPLICIT NONE
      INTEGER           iph
      DOUBLE PRECISION  am2,del1,del2,costhg,sinthg,dist0,dist1
* locals
      REAL              rn(10)
      DOUBLE PRECISION  a,eps,beta
*---------------------------------------------------------------------------------
      CALL PseuMar_MakeVec(rn,2)

cccc[[[[ very special tests
*      IF( iph .LE. 2) rn(2)=1d0 ! both photons 1 and 2 are forward
* lub
*      IF( iph .EQ. 1) rn(2)=1d0 ! photons 1 forward
*      IF( iph .EQ. 2) rn(2)=0d0 ! photons 2 backward
cccc]]]]

      beta =sqrt(1.d0-am2)
      eps  =am2/(1.d0+beta)                     != 1-beta
      del1 =(2.d0-eps)*(eps/(2.d0-eps))**rn(1)  != 1-beta*costhg
      del2 =2.d0-del1                           != 1+beta*costhg
* calculation of sin and cos theta from internal variables
      costhg=(del2-del1)/(2*beta)               ! exact
      sinthg=sqrt(del1*del2-am2*costhg**2)      ! exact
* symmetrization
      IF(rn(2) .LE. 0.5d0) THEN
        a=del1
        del1=del2
        del2=a
        costhg= -costhg
      ENDIF
      dist0=1d0/(del1*del2)*(1d0 -am2/2d0)
      dist1=1d0/(del1*del2) 
     $     *(1d0 -am2/2d0 -am2/4d0*(del1/del2+del2/del1))
* totaly equivalent formula is the following
*     dist1=1d0/(del1*del2)   *beta*sinthg**2/(del1*del2)
      END


      SUBROUTINE YFSini2_PoissGen(average,nmax,mult,rr)
*     ************************************************
* Last corr. nov. 91
* This generates photon multipl. nphot according to poisson distr.
* Input:  average = average multiplicity
*         nmax  = maximum multiplicity
* Output: mult = generated multiplicity
*         rr(1:100) list of ordered uniform random numbers,
*         a byproduct result, to be eventually used for some further
*         purpose (i.e.  generation of photon energies).
*     ************************
      IMPLICIT NONE
      DOUBLE PRECISION  average
      DOUBLE PRECISION  rr(*)
      INTEGER           nmax,mult
* locals
      DOUBLE PRECISION  y,sum
      INTEGER           it,nfail,nn
      REAL              rvec(10)
      DATA nfail/0/
*----------------------------------------------------------------
 50   nn=0
      sum=0d0
      DO it=1,nmax
         CALL PseuMar_MakeVec(rvec,1)
         y= log(rvec(1))
         sum=sum+y
         nn=nn+1
         rr(nn)=sum/(-average)
         IF(sum .LT. -average) GOTO 130
      ENDDO
      nfail=nfail+1
      IF(nfail .GT. 100) GOTO 900
      GOTO 50
 130  mult=nn-1
      RETURN
 900  WRITE(*,*) ' YFSini2_PoissGen: to small nmax ',nmax
      STOP
      END

*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  YFSini                                //
*//////////////////////////////////////////////////////////////////////////////

