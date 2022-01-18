       subroutine tens_red3_new_Re_Com_G_QUAD(m0,m1,m2,p1sq,p2sq,s12,
     # B0r_23,B0r_13,B0r_12, 
     # B0I_23,B0I_13,B0I_12, 
     # B1r_23,B1r_13,B1r_12, 
     # B1I_23,B1I_13,B1I_12, 
     # C0,!Cij,
     # C0r,C0I,Cijr,CijI)
C                tens_red3 = 3-point tensors
C     Francisco Campanario
C    Email: Francam@particle.uni-karlsruhe.de
C Date: 25/02/2010
c
c  determine the Passarino-Veltman tensor decomposition for the three-point
c  tensor integrals
c
c                       d^4k           1;  k_mu;   k_mu k_nu
c C0;C_mu;C_munu =Int -------  -----------------------------------------
c                     (2pi)^4    [k^2-m^2][(k+p1)^2-m^2][(k+p1+p2)^2-m^2] 
c
c with
c
c    C_mu = p1_mu C11  +  p_2_mu C12
c
c  C_munu = p1_mu p1_nu C21 + p2_mu p2_nu C22 + 
c           (p1_mu p2_nu + p1_nu p2_mu) C23  -  g_munu C24
c
c  for notation see Passarino&Veltman, NP B160 (1979) 151 and my notes
c
C INPUT:  p1sq, p2sq, s12          external invariants: p1^2, p2^2, s12=(p1+p2)^2
C         B0, C0                   4 scalar integrals; the 3 B0 are, 
c                                  in PV notation:
c         B0(1) = B0(1,2) = B0(p1)  B_0 function with subtraction of 
c         B0(2) = B0(2,3) = B0(p2)  divergent term
c         B0(3) = B0(1,3) = B0(s12)
c
c OUTPUT: Cij(n,m) = C_nm          form factors in the tensor integrals
c          n=1,2,3,4; n=1,2        a la PV
c

      implicit none
      real*16  p1sq, p2sq, s12
      complex*32 B0_23, B0_13, B0_12, C0
      real*16 r1, r2r1,p1p2,r1r0,det
      real*16 B1r_12,B1r_13,B1r_23,Cijr(4,2)
      real*16 B1I_12,B1I_13,B1I_23,CijI(4,2)
      real*16 B0r_23, B0r_13, B0r_12, C0r
      real*16 B0I_23, B0I_13, B0I_12, C0I 
      real*16 z11,z12,z21,z22,iz11,iz22
      real*16 Rr(2),RI(2),PRr(2),PRI(2)
      real*16 m0,m0sq,m1,m1sq,m2,m2sq
      real*16 deter,detAbs,ratio
      Logical Singular
      COMMON Singular
      integer difference,ihD(0:80),ihC(0:80),binsize,offset,diffmin
      integer diffminD
       common/Det/ihC,ihD,binsize,offset,diffmin,diffminD
      p1p2 = (s12 - p1sq - p2sq)*0.5q0

      m0sq=m0*m0
      m1sq=m1*m1
      m2sq=m2*m2

      r1 = p1sq -m1sq
      r1r0= r1 + m0sq
      r2r1 = (s12-m2sq) - r1

      deter = abs(2.q0*(p1sq*p2sq - p1p2*p1p2))
      detAbs=  abs(2.q0*(abs(p1sq*p2sq)+abs(p1p2*p1p2)))
      ratio=deter/detAbs

      difference=binsize*(log10(ratio)+offset)
c      print*, 'diffmin',diffmin
c      print*, 'difference',difference
      if(difference.lt.diffmin) diffmin = difference
c      print*, 'diffmin',diffmin
c      print*, ''
      ihC(difference)=ihC(difference)+1
      
      If( (deter/detAbs).lt.1d-6) Singular=.true.
    
 
      C0r=REALPART(C0)
      C0I=IMAGPART(C0)


  
      If(abs(p1sq).gt.abs(p1p2)) then
          z11=2q0*p1sq
          iz11=1q0/z11
          z12=2q0*p1p2 
          z21=z12*iz11
          z22=2q0*p2sq-z12*z21
          iz22=1q0/z22
c          iorder(1)=1
c          iorder(2)=2
          det=z11*z22 
c 1-2
       PRr(1) = (B0r_13 - B0r_23 - C0r*r1r0)
       PRr(2) = (B0r_12 - B0r_13 - C0r*r2r1)
   
       Rr(2)=(PRr(2)-z21*PRr(1))*iz22 
       Rr(1)=(PRr(1)-z12*Rr(2))*iz11 

       Cijr(1,1) = Rr(1)
       Cijr(2,1) = Rr(2)

       PRI(1) = (B0I_13 - B0I_23 - C0I*r1r0)
       PRI(2) = (B0I_12 - B0I_13 - C0I*r2r1)
      
       RI(2)=(PRI(2)-z21*PRI(1))*iz22 
       RI(1)=(PRI(1)-z12*RI(2))*iz11 

       CijI(1,1) = RI(1)
       CijI(2,1) = RI(2)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
c C00
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      Cijr(4,2) = ( B0r_23 +2q0*m0sq*C0r + Cijr(1,1)*r1r0 +
     1 Cijr(2,1)*r2r1 +1.q0)*0.25q0
      CijI(4,2) = ( B0I_23 +2q0*m0sq*C0I + CijI(1,1)*r1r0 +
     1 CijI(2,1)*r2r1)*0.25q0    
c 3-5
       PRr(1) = (B1r_13 + B0r_23 - Cijr(1,1)*r1r0 - Cijr(4,2)*2.q0)
       PRr(2) = (B1r_12 - B1r_13 - Cijr(1,1)*r2r1)

       Rr(2)=(PRr(2)-z21*PRr(1))*iz22 
       Rr(1)=(PRr(1)-z12*Rr(2))*iz11 

       Cijr(1,2) =Rr(1)
       Cijr(3,2) =Rr(2)

       PRI(1) = (B1I_13 + B0I_23 - CijI(1,1)*r1r0 - CijI(4,2)*2.q0)
       PRI(2) = (B1I_12 - B1I_13 - CijI(1,1)*r2r1)

       RI(2)=(PRI(2)-z21*PRI(1))*iz22 
       RI(1)=(PRI(1)-z12*RI(2))*iz11 

       CijI(1,2) =RI(1)
       CijI(3,2) =RI(2)

c 4-6
       PRr(1) = (  B1r_13 - B1r_23 - Cijr(2,1)*r1r0)
       PRr(2) = (- B1r_13          -Cijr(2,1)*r2r1 -Cijr(4,2)*2.q0)
      
       Rr(2)=(PRr(2)-z21*PRr(1))*iz22 
c      Rr(1)=(PRr(1)-z12*Rr(2))*iz11 

       Cijr(2,2) = Rr(2)
c      Cijr(3,2) = Rr(1)


       PRI(1) = (  B1I_13 - B1I_23 - CijI(2,1)*r1r0)
       PRI(2) = (- B1I_13          - CijI(2,1)*r2r1 -CijI(4,2)*2.q0)

       RI(2)=(PRI(2)-z21*PRI(1))*iz22 
c      RI(1)=(PRI(1)-z12*RI(2))*iz11 

       CijI(2,2) = RI(2)
c      CijI(3,2) = RI(1)

       return
ccccccccccccccccc
cccccccccccccccc
ccccccccccccccccc
       else
		  z11=2q0*p1p2
		  iz11=1q0/z11
		  z21=2q0*p1sq*iz11
		  z12=2q0*p2sq
		  z22=z11-z12*z21
		  iz22=1q0/z22
c          iorder(1)=2
c          iorder(2)=1
          det=-z11*z22

c 1-2
       PRr(1) = (B0r_13 - B0r_23 - C0r*r1r0)
       PRr(2) = (B0r_12 - B0r_13 - C0r*r2r1)
   
       Rr(2)=(PRr(1)-z21*PRr(2))*iz22 
       Rr(1)=(PRr(2)-z12*Rr(2))*iz11 
 
       Cijr(1,1) = Rr(1)
       Cijr(2,1) = Rr(2)
 
       PRI(1) = (B0I_13 - B0I_23 - C0I*r1r0)
       PRI(2) = (B0I_12 - B0I_13 - C0I*r2r1)
      
       RI(2)=(PRI(1)-z21*PRI(2))*iz22 
       RI(1)=(PRI(2)-z12*RI(2))*iz11 
 
       CijI(1,1) = RI(1)
       CijI(2,1) = RI(2)
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
c C00
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      Cijr(4,2) = ( B0r_23 +2q0*m0sq*C0r + Cijr(1,1)*r1r0 +
     1 Cijr(2,1)*r2r1 +1.q0)*0.25q0
      CijI(4,2) = ( B0I_23 +2q0*m0sq*C0I + CijI(1,1)*r1r0 +
     1 CijI(2,1)*r2r1)*0.25q0
c 3-5
       PRr(1) = (B1r_13 + B0r_23 - Cijr(1,1)*r1r0 - Cijr(4,2)*2.q0)
       PRr(2) = (B1r_12 - B1r_13 - Cijr(1,1)*r2r1)
       
       Rr(2)=(PRr(1)-z21*PRr(2))*iz22 
       Rr(1)=(PRr(2)-z12*Rr(2))*iz11 
       
       Cijr(1,2) =Rr(1)
       Cijr(3,2) =Rr(2)
       
       PRI(1) = (B1I_13 + B0I_23 - CijI(1,1)*r1r0 - CijI(4,2)*2.q0)
       PRI(2) = (B1I_12 - B1I_13 - CijI(1,1)*r2r1)
       
       RI(2)=(PRI(1)-z21*PRI(2))*iz22 
       RI(1)=(PRI(2)-z12*RI(2))*iz11 
       
       CijI(1,2) =RI(1)
       CijI(3,2) =RI(2)

c 4-6
       PRr(1) = (  B1r_13 - B1r_23 - Cijr(2,1)*r1r0)
       PRr(2) = (- B1r_13          -Cijr(2,1)*r2r1 -Cijr(4,2)*2.q0)
       
       Rr(2)=(PRr(1)-z21*PRr(2))*iz22 
c      Rr(1)=(PRr(2)-z12*Rr(2))*iz11 

       Cijr(2,2) = Rr(2)
c      Cijr(3,2) = Rr(1)


       PRI(1) = (  B1I_13 - B1I_23 - CijI(2,1)*r1r0)
       PRI(2) = (- B1I_13          - CijI(2,1)*r2r1 -CijI(4,2)*2.q0)
       
       RI(2)=(PRI(1)-z21*PRI(2))*iz22 
c      RI(1)=(PRI(2)-z12*RI(2))*iz11 


       CijI(2,2) = RI(2)
c      CijI(3,2) = RI(1)

       endif
	return
	end

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
