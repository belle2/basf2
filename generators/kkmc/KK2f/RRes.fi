************************************************************************************
*                                                                                  *
*                         Pseudo Class RRes                                        *
*                                                                                  *
************************************************************************************
* 
* Parameters and physical constants
*
* Maarten Boonekamp, sept. 2001
************************************************************************
      DOUBLE PRECISION  alQED,pi,xsmu
      PARAMETER ( alQED = 1d0/137d0 )
      PARAMETER ( pi    = 3.14159265354 )
      PARAMETER ( xsmu  = 4d0*pi*alQED**2/3d0 )

************************************************************************
* 
*                          PARTICLE DATA
*
* Rho(770) and Omega(782) are taken from CMD-2 F_pi fit 
* (hep-ex/9904027), all others are from PDG'2000.
* Only 1-- resonances with known widths into electron pairs are 
* included (others are taken out by setting zero width into electrons).
*
* Maarten Boonekamp, sept. 2001
************************************************************************

C Declarations
C ============
      DOUBLE PRECISION xmd, xmu, xms, xmc, xmb
      DOUBLE PRECISION xcd, xcu, xcs, xcc, xcb
      DOUBLE PRECISION xmpic, xmpi0, xmkac, xmka0, xmDcd, xmDcu, xmDcs
      DOUBLE PRECISION xmBbu, xmBbd, xmBbs, xmBbc
      DOUBLE PRECISION xmrho, xwrho, rhoee, xmrh2, xwrh2, rh2ee
      DOUBLE PRECISION xmrh3, xwrh3, rh3ee, xmome, xwome, omeee, omepi
      DOUBLE PRECISION xmom2, xwom2, om2ee, xmom3, xwom3, om3ee
      DOUBLE PRECISION xmphi, xwphi, phiee, xmph2, xwph2, ph2ee
      DOUBLE PRECISION xmpsi, xwpsi, psiee, xmps2, xwps2, ps2ee
      DOUBLE PRECISION xmps3, xwps3, ps3ee, xmps4, xwps4, ps4ee
      DOUBLE PRECISION xmps5, xwps5, ps5ee, xmps6, xwps6, ps6ee
      DOUBLE PRECISION xmups, xwups, upsee, xmup2, xwup2, up2ee
      DOUBLE PRECISION xmup3, xwup3, up3ee, xmup4, xwup4, up4ee
      DOUBLE PRECISION xmup5, xwup5, up5ee, xmup6, xwup6, up6ee
      DOUBLE PRECISION xmz, xwz, zwee, xmw, xww

C QUARKS
C ======
c ... d
      PARAMETER ( xmd = 0.006d0, xcd = -1d0/3d0 )
c ... u
      PARAMETER ( xmu = 0.003d0, xcu =  2d0/3d0 )
c ... s
      PARAMETER ( xms = 0.122d0, xcs = -1d0/3d0 )
c ... c
      PARAMETER ( xmc = 1.250d0, xcc =  2d0/3d0 )
c ... b
      PARAMETER ( xmb = 4.200d0, xcb = -1d0/3d0 )

C SCALAR MESONS
C =============

c ... pion
      PARAMETER ( xmpic= 0.13957d0, xmpi0= 0.13497d0 )
c ... kaon
      PARAMETER ( xmkac= 0.49368d0, xmka0= 0.49767d0 )
c ... D mesons
      PARAMETER ( xmDcd= 1.8693 d0 ) ! D+/-
      PARAMETER ( xmDcu= 1.8645 d0 ) ! D0
      PARAMETER ( xmDcs= 1.9686 d0 ) ! Ds
c ... B mesons
      PARAMETER ( xmBbu= 5.2790 d0 ) ! B+/-
      PARAMETER ( xmBbd= 5.2794 d0 ) ! B0
      PARAMETER ( xmBbs= 5.3696 d0 ) ! Bs
      PARAMETER ( xmBbc= 6.4    d0 ) ! Bc

C VECTOR MESONS
C =============

C ... Rho family
c ...... rho(770)
      PARAMETER ( xmrho= 0.77457d0, xwrho=147.65d-3, rhoee=6.89  d-6 )
c ...... rho(1450) <-- NOT INCLUDED
      PARAMETER ( xmrh2= 1.465  d0, xwrh2=310   d-3, rh2ee=0.0000d-6 )
c ...... rho(1700) <-- NOT INCLUDED
      PARAMETER ( xmrh3= 1.700  d0, xwrh3=240   d-3, rh3ee=0.0000d-6 )

C ... Omega family
c ...... omega(782)
      PARAMETER ( xmome= 0.78194d0, xwome=8.41  d-3, omeee=0.60  d-6 )
      PARAMETER ( omepi= 0.00011d0 )
c ...... omega(1420) <-- NOT INCLUDED
      PARAMETER ( xmom2= 1.419  d0, xwom2=174   d-3, om2ee=0.0000d-6 )
c ...... omega(1650) <-- NOT INCLUDED
      PARAMETER ( xmom3= 1.649  d0, xwom3=220   d-3, om3ee=0.0000d-6 )

C ... Phi family
c ...... phi(1020)
      PARAMETER ( xmphi= 1.01942d0, xwphi=4.46  d-3, phiee=1.33  d-6 )
c ...... phi(1680) <-- NOT INCLUDED
      PARAMETER ( xmph2= 1.680  d0, xwph2=150   d-3, ph2ee=0.0000d-6 )

C ... Psi family
c ...... J/Psi(1S)
      PARAMETER ( xmpsi= 3.09687d0, xwpsi=0.087 d-3, psiee=5.26  d-6 )
c ...... Psi(2S)
      PARAMETER ( xmps2= 3.68596d0, xwps2=0.277 d-3, ps2ee=2.12  d-6 )
c ...... Psi(3770)
      PARAMETER ( xmps3= 3.7699 d0, xwps3=23.6  d-3, ps3ee=0.26  d-6 )
c ...... Psi(4040)
      PARAMETER ( xmps4= 4.040  d0, xwps4=52    d-3, ps4ee=0.75  d-6 )
c ...... Psi(4160)
      PARAMETER ( xmps5= 4.159  d0, xwps5=78    d-3, ps5ee=0.77  d-6 )
c ...... Psi(4415)
      PARAMETER ( xmps6= 4.415  d0, xwps6=43    d-3, ps6ee=0.47  d-6 )

C ... Upsilon family
c ...... Upsilon(1S)
      PARAMETER ( xmups= 9.46030d0, xwups=0.0525d-3, upsee=1.32  d-6 )
c ...... Upsilon(2S)		  	     
      PARAMETER ( xmup2=10.02326d0, xwup2=0.044 d-3, up2ee=0.52  d-6 )
c ...... Upsilon(3S) <-- NOT INCLUDED
      PARAMETER ( xmup3=10.3552 d0, xwup3=0.026 d-3, up3ee=0.0000d-6 )
c ...... Upsilon(4S)		  	     
      PARAMETER ( xmup4=10.5800 d0, xwup4= 14   d-3, up4ee=0.248 d-6 )
c ...... Upsilon(10860)		  	     
      PARAMETER ( xmup5=10.865  d0, xwup5=110   d-3, up5ee=0.31  d-6 )
c ...... Upsilon(11020)		  	     
      PARAMETER ( xmup6=11.019  d0, xwup6= 79   d-3, up6ee=0.13  d-6 )

C VECTOR BOSONS
C =============

c ... SU(2)
c ...... Z
      PARAMETER ( xmz = 91.1876d0, xwz = 2.4952d0, zwee = 0.08391d0)
c ...... W
      PARAMETER ( xmw = 80.43  d0, xww = 2.11  d0                  )


************************************************************************
*     Some declarations
************************************************************************

      DOUBLE PRECISION p_pi   ! decaying pion momentum
      DOUBLE PRECISION sdepw  ! s-dependent width
      DOUBLE COMPLEX   BrWig  ! vector meson Breit-Wigner

************************************************************************
* 
* Inline function definitions
*
* Maarten Boonekamp, sept. 2001
************************************************************************
      DOUBLE PRECISION beta
      DOUBLE PRECISION X,Y,Z,BW_sq
C Variables : X == nominal width, Y == nominal mass, Z == c.o.m. energy squared
c ... kinematic suppression
      beta(Y,Z)    = DSQRT(1d0 - MIN(Z,4d0*Y**2)/Z)
c ... pion momentum in 2-pi decay
      p_pi(Z)      = 0.5d0 * DSQRT(MAX(Z,4d0*xmpic**2) - 4d0*xmpic**2)
c ... s-dependent width
      sdepw(X,Y,Z) = X * (p_pi(Z) / p_pi(Y**2))**3 * DSQRT(Y**2 / Z)
c ... Breit-Wigner
      BrWig(X,Y,Z) = DCMPLX(Y**2,0d0)/DCMPLX(Y**2 - Z,Y * sdepw(X,Y,Z))
      BW_sq(X,Y,Z) = X / ((DSQRT(Z)-Y)**2 + X**2/4d0)

************************************************************************************
*                                                                                  *
*                End of   Pseudo Class RRes                                        *
*                                                                                  *
************************************************************************************
