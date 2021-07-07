      subroutine had1_init
      implicit real*8 (a-h,o-z)
      dimension p1(4),p2(4)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
      common /input/ su,su2,qq2,p1,p2,ngen,iseed,mode,iww,nhit
      common /param/ pi,alpha,f_max
      common /cbwgrho/ rhom2,rho1m2,rho2m2,omm2,rhomg,rho1mg,rho2mg
     1                ,ommg
      common /cbwga1/ a1m2,con 
      common /anom/amrop,gamrop,sig,amrop_2,amropg
      common /cbwgrho_t/ rho1m2_t,rho1mg_t,beta
c
      pi     = 3.141592653589793238d0
      alpha  = 1.d0/137.0359895d0
c
      gam1  = 0.38d0
      gam2  = 0.38d0
      fpi   = 0.0933d0
c      coupl = 2.d0*sqrt(3.d0)/fpi**2
      coupl = sqrt(6.d0)/fpi**2  ! normalization change /sqrt(2)
      a1m   = 1.251d0
      a1g   = 0.599d0
      rhom  = 0.773d0
      rhog  = 0.145d0
      rho1m = 1.35d0
      rho1g = 0.3d0
      rho2m = 1.7d0
      rho2g = 0.235d0
      omm   = 0.782d0
      omg   = 0.0085d0
      aa    = 0.d0    ! to compare with Finkemeier (no omega)
      bb1   = 0.08d0
      bb2   = -0.0075d0
      f0m   = 1.3d0
      f0g   = 0.6d0
      pim   = 0.14d0
c 
c the omega coupling changed
c
      sgo   = 1.55d0/sqrt(2.d0)  
CC      sgo   = 1.4d0/sqrt(2.d0)
c
       rhom2  = rhom**2
       rho1m2 = rho1m**2
       rho2m2 = rho2m**2
       omm2   = omm**2
       rhomg  = rhom*rhog
       rho1mg = rho1m*rho1g 
       rho2mg = rho2m*rho2g
       ommg   = omm*omg
c
        a1m2 = a1m**2
        con  = a1g*a1m/gfun8(a1m2)
c
        amrop  = 1.7d0
        gamrop = 0.26d0
        sig    = -0.1d0
        amrop_2 = amrop**2
        amropg  = amrop*gamrop
c
      beta  = -0.145d0
      rho1m_t = 1.37d0
      rho1g_t = 0.51d0 
c
       rho1m2_t = rho1m_t**2
       rho1mg_t = rho1m_t*rho1g_t

      return
      end
c*************************************************************************
      complex*16 function anom_bwg(q1_2,q2_2)
      implicit real*8 (a-h,o-z)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
      common /cbwgrho/ rhom2,rho1m2,rho2m2,omm2,rhomg,rho1mg,rho2mg
     1                ,ommg
      common /anom/amrop,gamrop,sig,amrop_2,amropg
c
      anom_bwg = (dcmplx(1.d0,0.d0)/dcmplx(rhom2-q1_2,-rhomg)
     1         + dcmplx(sig,0.d0)/dcmplx(amrop_2-q1_2,-amropg) )
     2     * dcmplx(1.d0,0.d0)/dcmplx(omm2-q2_2,-ommg)
      return
      end
c*************************************************************************
      complex*16 function bwga1(q1_2)
      implicit real*8 (a-h,o-z)
c
      common /cbwga1/ a1m2,con 
c
      ggm = gfun8(q1_2)*con 
      bwga1 = dcmplx(a1m2,0.d0)/dcmplx(a1m2-q1_2,-ggm)     
c
      return
      end
c*************************************************************************
      real*8 function gfun8(q1_2)
      implicit real*8 (a-h,o-z)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
c
      if(q1_2.gt.((rhom+pim)**2))then
        gfun8 = q1_2*1.623d0 + 10.38d0 - 9.32d0/q1_2 + 0.65d0/q1_2**2
      else
        c1   = q1_2 - 9.d0*pim**2
        gfun8 = 4.1d0 *c1**3 *(1.d0 - 3.3d0*c1 + 5.8d0*c1**2)
      endif
c
      return
      end
c*************************************************************************
      complex*16 function bwgrho(q1_2)
      implicit real*8 (a-h,o-z)
c
      complex*16 cbw,cbw1,cbw2,cbwo
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
      common /cbwgrho/ rhom2,rho1m2,rho2m2,omm2,rhomg,rho1mg,rho2mg,ommg
c
      c2     = 4.d0*pim**2/q1_2
      if(c2.lt.1.d0)then
c       
      c1     = rhom2/q1_2
      gamrho  = rhomg*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      c1      = rho1m2/q1_2
      gamrho1 = rho1mg*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      c1      = rho2m2/q1_2
      gamrho2 = rho2mg*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      c1      = omm2/q1_2
      gamom   = ommg*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      else 
       gamrho =0.d0
       gamrho1=0.d0
       gamrho2=0.d0
       gamom  =0.d0
      endif
c      
      cbw   = dcmplx(rhom2,0.d0)/dcmplx(rhom2-q1_2,-gamrho)
      cbw1  = dcmplx(rho1m2,0.d0)/dcmplx(rho1m2-q1_2,-gamrho1)
      cbw2  = dcmplx(rho2m2,0.d0)/dcmplx(rho2m2-q1_2,-gamrho2)
      cbwo  = dcmplx(omm2,0.d0)/dcmplx(omm2-q1_2,-gamom)
      bwgrho = ( cbw *(1.d0+aa*cbwo)/(1.d0+aa)
     1          + bb1*cbw1+bb2*cbw2)/(1.d0+bb1+bb2)
c
      return
      end
c*************************************************************************
      complex*16 function bwgrho_t(q1_2)
      implicit real*8 (a-h,o-z)
c
      complex*16 cbw,cbw1,cbw2,cbwo
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
      common /cbwgrho/ rhom2,rho1m2,rho2m2,omm2,rhomg,rho1mg,rho2mg
     1                ,ommg
      common /cbwgrho_t/ rho1m2_t,rho1mg_t,beta
c
      c2     = 4.d0*pim**2/q1_2
c       
      c1     = rhom2/q1_2
      if(c2.gt.1.d0)then
        gamrho = 0.d0
      else
       gamrho  = rhomg*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      endif
      c1     = rho1m2_t/q1_2
      if(c2.gt.1.d0)then
       gamrho1 =0
      else
       gamrho1  = rho1mg_t*sqrt(c1*((1.d0-c2)/(c1-c2))**3)
      endif
c      
      cbw    = dcmplx(rhom2,0.d0)/dcmplx(rhom2-q1_2,-gamrho)
      cbw1   = dcmplx(rho1m2,0.d0)/dcmplx(rho1m2-q1_2,-gamrho1)

      bwgrho_t = (cbw+beta*cbw1)/(1.d0+beta) 
c
      return
      end
c ************************************************************************
      complex*16 function bwgf0(q1_2)
      implicit real*8 (a-h,o-z)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
c
      f0m2  = f0m**2
      f0mg  = f0m*f0g
      bwgf0 = dcmplx(f0m2,-f0mg)/dcmplx(f0m2-q1_2,-f0mg)     
c
      return
      end
c***************************************************************************
c*************************************************************************
c the file contains all currents in 4pi mode
c the basic building block is rho(0) -> pi+ pi- 2pi0 mode
c other modes: rho(0) -> 2pi+ 2pi- 
c             rho(-) -> 3pi0 pi-
c             rho(-) -> 2pi- pi+ pi0
c
c*************************************************************************
c this is a code of hadronic current rho(0) -> 2pi+ 2pi- 
c
c q1,q4 : pi+'s four momenta
c q2,q3 : pi-'s four momenta
c
      subroutine had1(qq2,q1,q2,q3,q4,hadr)
      implicit real*8 (a-h,o-z)
c
      complex*16 hadr(4),hadr1(4),hadr2(4),hadr3(4),hadr4(4)
      dimension q1(4),q2(4),q3(4),q4(4)
c
      call had2(qq2,q1,q2,q3,q4,hadr1)
      call had2(qq2,q4,q2,q3,q1,hadr2)
      call had2(qq2,q1,q3,q2,q4,hadr3)
      call had2(qq2,q4,q3,q2,q1,hadr4)
c
      do i=1,4
       hadr(i) = hadr1(i)+hadr2(i)+hadr3(i)+hadr4(i)
      enddo  
c
      return
      end
c*************************************************************************
c this is a code of hadronic current rho(-) -> 3pi0 pi- 
c
c q1,q2,q3 : pi0's four momenta
c q4       : pi-'s four momentum
c
c
      subroutine had3(qq2,q1,q2,q3,q4,hadr)
      implicit real*8 (a-h,o-z)
c
      complex*16 hadr(4),hadr1(4),hadr2(4),hadr3(4)
      dimension q1(4),q2(4),q3(4),q4(4)
c
      call had2(qq2,q1,q2,q3,q4,hadr1)
      call had2(qq2,q1,q3,q2,q4,hadr2)
      call had2(qq2,q3,q2,q1,q4,hadr3)
c
      do i=1,4
       hadr(i) = (hadr1(i)+hadr2(i)+hadr3(i))*sqrt(2.d0)
      enddo  
c
      return
      end
c*************************************************************************
c this is a code of hadronic current rho(-) -> 2pi- pi+ pi0 
c
c q1,q2 : pi-'s four momenta
c q3    : pi0   four momentum
c q4    : pi+   four momentum
c
c
      subroutine had4(qq2,q1,q2,q3,q4,hadr)
c
      implicit real*8 (a-h,o-z)
c
      complex*16 hadr(4),hadr1(4),hadr2(4)
      dimension q1(4),q2(4),q3(4),q4(4)
c
      call had2(qq2,q3,q1,q2,q4,hadr1)
      call had2(qq2,q3,q2,q1,q4,hadr2)
c
      do i=1,4
       hadr(i) = (hadr1(i)+hadr2(i))*sqrt(2.d0)
      enddo  
c
      return
      end
c*************************************************************************
c*************************************************************************
c this is a code of hadronic current rho(0) -> pi+ pi- 2pi0
c
c the basic building block for other currents
c
c q1,q2 : pi0's four momenta
c q3    : pi-   four momentum
c q4    : pi+   four momentum
c
c the current was obtained in h1_t_f0.f(log)
c
      subroutine had2(qq2,q1,q2,q3,q4,hadr)
      implicit real*8 (a-h,o-z)
c
      complex*16 hadr(4),cfac(4),tt(4,4,4),ss(4,4,4,4)
      complex*16 bwga1,bwgrho,bwgrho_t,bwgf0,c0,c5,c6
      complex*16 c1_t,c2_t,c3_t,c4_t,anom_bwg
      dimension q1(4),q2(4),q3(4),q4(4),q2m4(4),q3m1(4),q4m1(4),q3m2(4)
      dimension q123(4),q124(4),qq(4),q3m4(4),q134(4),q234(4)
      dimension q2p4(4),q1p3(4),q2p3(4),q1p4(4),q1p2(4),q3p4(4)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
c
c     the dot products:
c
      do i=1,4
        q2m4(i) = q2(i)-q4(i)
        q3m1(i) = q3(i)-q1(i)
        q3m4(i) = q3(i)-q4(i)
        q4m1(i) = q4(i)-q1(i)
        q3m2(i) = q3(i)-q2(i)
        q2p4(i) = q2(i)+q4(i)
        q1p3(i) = q1(i)+q3(i)
        q1p2(i) = q1(i)+q2(i)
        q2p3(i) = q2(i)+q3(i)
        q1p4(i) = q1(i)+q4(i)
        q3p4(i) = q3(i)+q4(i)
        q123(i) = q2p3(i)+q1(i)
        q124(i) = q2p4(i)+q1(i)
        qq(i)   = q123(i) + q4(i)
      enddo
      q1_2m4 = q1(1)*q2m4(1)-q1(2)*q2m4(2)-q1(3)*q2m4(3)-q1(4)*q2m4(4)      
      q1_3m2 = q1(1)*q3m2(1)-q1(2)*q3m2(2)-q1(3)*q3m2(3)-q1(4)*q3m2(4)      
      q3_2m4 = q3(1)*q2m4(1)-q3(2)*q2m4(2)-q3(3)*q2m4(3)-q3(4)*q2m4(4)      
      q2_3m1 = q2(1)*q3m1(1)-q2(2)*q3m1(2)-q2(3)*q3m1(3)-q2(4)*q3m1(4)      
      q2_4m1 = q2(1)*q4m1(1)-q2(2)*q4m1(2)-q2(3)*q4m1(3)-q2(4)*q4m1(4)      
      q3_4m1 = q3(1)*q4m1(1)-q3(2)*q4m1(2)-q3(3)*q4m1(3)-q3(4)*q4m1(4)      
      q4_3m1 = q4(1)*q3m1(1)-q4(2)*q3m1(2)-q4(3)*q3m1(3)-q4(4)*q3m1(4)      
      q4_3m2 = q4(1)*q3m2(1)-q4(2)*q3m2(2)-q4(3)*q3m2(3)-q4(4)*q3m2(4)      
      qmq3_2 = q124(1)**2 -q124(2)**2 -q124(3)**2 -q124(4)**2
      qmq4_2 = q123(1)**2 -q123(2)**2 -q123(3)**2 -q123(4)**2
      q_q3   = qq(1)*q3(1)-qq(2)*q3(2)-qq(3)*q3(3)-qq(4)*q3(4)
      q_q4   = qq(1)*q4(1)-qq(2)*q4(2)-qq(3)*q4(3)-qq(4)*q4(4)
      q2p4_2 = q2p4(1)**2 - q2p4(2)**2 - q2p4(3)**2 - q2p4(4)**2
      q3p4_2 = q3p4(1)**2 - q3p4(2)**2 - q3p4(3)**2 - q3p4(4)**2
      q1p3_2 = q1p3(1)**2 - q1p3(2)**2 - q1p3(3)**2 - q1p3(4)**2
      q1p2_2 = q1p2(1)**2 - q1p2(2)**2 - q1p2(3)**2 - q1p2(4)**2
      q2p3_2 = q2p3(1)**2 - q2p3(2)**2 - q2p3(3)**2 - q2p3(4)**2
      q1p4_2 = q1p4(1)**2 - q1p4(2)**2 - q1p4(3)**2 - q1p4(4)**2
      q1p2_3m4 = q1p2(1)*q3m4(1)
     1          -q1p2(2)*q3m4(2)-q1p2(3)*q3m4(3)-q1p2(4)*q3m4(4)
      q1p3_2m4 = q1_2m4 + q3_2m4
      q1p4_3m2 = q1_3m2 + q4_3m2 
      q2p4_3m1 = q2_3m1 + q4_3m1
      q2p3_4m1 = q2_4m1 + q3_4m1 
c
      c0 = bwgrho(qq2)*coupl
c      c0 = coupl
c
      c1_t = bwgrho_t(q2p4_2)
      c2_t = bwgrho_t(q1p3_2)
      c3_t = bwgrho_t(q2p3_2)
      c4_t = bwgrho_t(q1p4_2)
c
      c5 = bwga1(qmq3_2)
      c6 = bwga1(qmq4_2)
c
      tt(1,2,4) = c5*c1_t*gam1
      tt(2,1,4) = c5*c4_t*gam1
      tt(2,3,1) = c6*c2_t*gam1
      tt(1,2,3) = c6*c3_t*gam1
c
      ss(3,4,1,2) = bwgrho(q3p4_2)*bwgf0(q1p2_2)*gam2
c
      cfac(1) =  tt(1,2,3) * (-1.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 1.d0 - q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 3.d0 + q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * (-3.d0 - q2_3m1/qmq4_2 )
c
      cfac(2) =  tt(1,2,3) * (-3.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 3.d0 - q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 1.d0 + q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * (-1.d0 - q2_3m1/qmq4_2 )
c
      cfac(3) =  tt(1,2,3) * ( 1.d0 - q1_3m2/qmq4_2 )
     1         + tt(1,2,4) * ( 1.d0 + q1_2m4/qmq3_2 )
     2         + tt(2,1,4) * ( 1.d0 - q2_4m1/qmq3_2 )
     3         + tt(2,3,1) * ( 1.d0 - q2_3m1/qmq4_2 )
     4      -3.d0*ss(3,4,1,2)
c
      cfac(4) =  tt(1,2,3) 
     1 *(1.d0 -2.d0/qq2*(q_q4*q1_3m2/qmq4_2 +q1p4_3m2) +q1_3m2/qmq4_2 )
     2         + tt(1,2,4) 
     3 *(-1.d0-2.d0/qq2*(q1_2m4/qmq3_2*q_q3 +q1p3_2m4) +q1_2m4/qmq3_2 )
     4         + tt(2,1,4) 
     5 *(-1.d0+2.d0/qq2*(q_q3*q2_4m1/qmq3_2 +q2p3_4m1) -q2_4m1/qmq3_2 )
     6         + tt(2,3,1)
     7 *(1.d0 -2.d0/qq2*(q2_3m1/qmq4_2*q_q4 +q2p4_3m1) +q2_3m1/qmq4_2 )
     8   +3.d0*ss(3,4,1,2)/qq2*q1p2_3m4
c
      do i=1,4
        cfac(i) = cfac(i)*c0
      enddo
c
      do i=1,4
        hadr(i) =  q1(i)  *cfac(1) + q2(i)*cfac(2) 
     1           + q3m4(i)*cfac(3) + qq(i)*cfac(4)
      enddo
c
c  from here Omega current
c
      fac3 = sgo * 1475.98d0*12.924d0 * 0.266d0 *rhom**2
c
c     the dot products:
c
      do i=1,4
        q134(i) = q1p3(i)+q4(i)
        q234(i) = q2p4(i)+q3(i)
      enddo
c
      q1_134 = q1(1)*q134(1)-q1(2)*q134(2)-q1(3)*q134(3)-q1(4)*q134(4)      
      q3_134 = q3(1)*q134(1)-q3(2)*q134(2)-q3(3)*q134(3)-q3(4)*q134(4)      
      q4_134 = q4(1)*q134(1)-q4(2)*q134(2)-q4(3)*q134(3)-q4(4)*q134(4)      
      q2_234 = q2(1)*q234(1)-q2(2)*q234(2)-q2(3)*q234(3)-q2(4)*q234(4)      
      q3_234 = q3(1)*q234(1)-q3(2)*q234(2)-q3(3)*q234(3)-q3(4)*q234(4)      
      q4_234 = q4(1)*q234(1)-q4(2)*q234(2)-q4(3)*q234(3)-q4(4)*q234(4)      
      q12    = q1(1)*q2(1)  - q1(2)*q2(2) - q1(3)*q2(3) - q1(4)*q2(4)
      q13    = q1(1)*q3(1)  - q1(2)*q3(2) - q1(3)*q3(3) - q1(4)*q3(4)
      q14    = q1(1)*q4(1)  - q1(2)*q4(2) - q1(3)*q4(3) - q1(4)*q4(4)
      q23    = q2(1)*q3(1)  - q2(2)*q3(2) - q2(3)*q3(3) - q2(4)*q3(4)
      q24    = q2(1)*q4(1)  - q2(2)*q4(2) - q2(3)*q4(3) - q2(4)*q4(4)
      q34    = q3(1)*q4(1)  - q3(2)*q4(2) - q3(3)*q4(3) - q3(4)*q4(4)
      q234_2 = q234(1)**2-q234(2)**2-q234(3)**2-q234(4)**2      
      q134_2 = q134(1)**2-q134(2)**2-q134(3)**2-q134(4)**2      
c 
      cfac(1) = anom_bwg(qq2,q134_2) *(q3_134*q24 -q4_134*q23)
      cfac(2) = anom_bwg(qq2,q234_2) *(q3_234*q14 -q4_234*q13)
      cfac(3) = anom_bwg(qq2,q134_2) *(q4_134*q12 -q1_134*q24)
     1        + anom_bwg(qq2,q234_2) *(q4_234*q12 -q2_234*q14)
      cfac(4) = anom_bwg(qq2,q134_2) *(q1_134*q23 -q3_134*q12)
     1        + anom_bwg(qq2,q234_2) *(q2_234*q13 -q3_234*q12)
c
      do i =1,4
        hadr(i) = hadr(i) + fac3* (q1(i)*cfac(1) + q2(i)*cfac(2)
     1                            + q3(i)*cfac(3) + q4(i)*cfac(4) )
      enddo
c
      return
      end
c*************************************************************************
c this is a code of hadronic current rho(0) -> pi+ pi- 2pi0
c
c the basic building block for other currents: omega part
c
c q1,q2 : pi0's four momenta
c q3    : pi-   four momentum
c q4    : pi+   four momentum
c
c the current was obtained in h1_t_f0.f(log)
c
      subroutine had2_om(qq2,q1,q2,q3,q4,hadr)
      implicit real*8 (a-h,o-z)
c
      complex*16 hadr(4),cfac(4),tt(4,4,4),ss(4,4,4,4)
      complex*16 bwga1,bwgrho,bwgrho_t,bwgf0,c0,c5,c6
      complex*16 c1_t,c2_t,c3_t,c4_t,anom_bwg
      dimension q1(4),q2(4),q3(4),q4(4),q2m4(4),q3m1(4),q4m1(4),q3m2(4)
      dimension q123(4),q124(4),qq(4),q3m4(4),q134(4),q234(4)
      dimension q2p4(4),q1p3(4),q2p3(4),q1p4(4),q1p2(4),q3p4(4)
c
      common /had_par/ gam1,gam2,coupl,a1m,a1g,rhom,rhog,rho1m,rho1g
     1                ,rho2m,rho2g,omm,omg,aa,bb1,bb2,f0m,f0g,pim,sgo
c
c     the dot products:
c
      do i=1,4
        q2m4(i) = q2(i)-q4(i)
        q3m1(i) = q3(i)-q1(i)
        q3m4(i) = q3(i)-q4(i)
        q4m1(i) = q4(i)-q1(i)
        q3m2(i) = q3(i)-q2(i)
        q2p4(i) = q2(i)+q4(i)
        q1p3(i) = q1(i)+q3(i)
        q1p2(i) = q1(i)+q2(i)
        q2p3(i) = q2(i)+q3(i)
        q1p4(i) = q1(i)+q4(i)
        q3p4(i) = q3(i)+q4(i)
        q123(i) = q2p3(i)+q1(i)
        q124(i) = q2p4(i)+q1(i)
        qq(i)   = q123(i) + q4(i)
      enddo
      q1_2m4 = q1(1)*q2m4(1)-q1(2)*q2m4(2)-q1(3)*q2m4(3)-q1(4)*q2m4(4)      
      q1_3m2 = q1(1)*q3m2(1)-q1(2)*q3m2(2)-q1(3)*q3m2(3)-q1(4)*q3m2(4)      
      q3_2m4 = q3(1)*q2m4(1)-q3(2)*q2m4(2)-q3(3)*q2m4(3)-q3(4)*q2m4(4)      
      q2_3m1 = q2(1)*q3m1(1)-q2(2)*q3m1(2)-q2(3)*q3m1(3)-q2(4)*q3m1(4)      
      q2_4m1 = q2(1)*q4m1(1)-q2(2)*q4m1(2)-q2(3)*q4m1(3)-q2(4)*q4m1(4)      
      q3_4m1 = q3(1)*q4m1(1)-q3(2)*q4m1(2)-q3(3)*q4m1(3)-q3(4)*q4m1(4)      
      q4_3m1 = q4(1)*q3m1(1)-q4(2)*q3m1(2)-q4(3)*q3m1(3)-q4(4)*q3m1(4)      
      q4_3m2 = q4(1)*q3m2(1)-q4(2)*q3m2(2)-q4(3)*q3m2(3)-q4(4)*q3m2(4)      
      qmq3_2 = q124(1)**2 -q124(2)**2 -q124(3)**2 -q124(4)**2
      qmq4_2 = q123(1)**2 -q123(2)**2 -q123(3)**2 -q123(4)**2
      q_q3   = qq(1)*q3(1)-qq(2)*q3(2)-qq(3)*q3(3)-qq(4)*q3(4)
      q_q4   = qq(1)*q4(1)-qq(2)*q4(2)-qq(3)*q4(3)-qq(4)*q4(4)
      q2p4_2 = q2p4(1)**2 - q2p4(2)**2 - q2p4(3)**2 - q2p4(4)**2
      q3p4_2 = q3p4(1)**2 - q3p4(2)**2 - q3p4(3)**2 - q3p4(4)**2
      q1p3_2 = q1p3(1)**2 - q1p3(2)**2 - q1p3(3)**2 - q1p3(4)**2
      q1p2_2 = q1p2(1)**2 - q1p2(2)**2 - q1p2(3)**2 - q1p2(4)**2
      q2p3_2 = q2p3(1)**2 - q2p3(2)**2 - q2p3(3)**2 - q2p3(4)**2
      q1p4_2 = q1p4(1)**2 - q1p4(2)**2 - q1p4(3)**2 - q1p4(4)**2
      q1p2_3m4 = q1p2(1)*q3m4(1)
     1          -q1p2(2)*q3m4(2)-q1p2(3)*q3m4(3)-q1p2(4)*q3m4(4)
      q1p3_2m4 = q1_2m4 + q3_2m4
      q1p4_3m2 = q1_3m2 + q4_3m2 
      q2p4_3m1 = q2_3m1 + q4_3m1
      q2p3_4m1 = q2_4m1 + q3_4m1 
c
c
c  from here Omega current
c
      fac3 = sgo * 1475.98d0*12.924d0 * 0.266d0 *rhom**2
c
c     the dot products:
c
      do i=1,4
        q134(i) = q1p3(i)+q4(i)
        q234(i) = q2p4(i)+q3(i)
      enddo
c
      q1_134 = q1(1)*q134(1)-q1(2)*q134(2)-q1(3)*q134(3)-q1(4)*q134(4)      
      q3_134 = q3(1)*q134(1)-q3(2)*q134(2)-q3(3)*q134(3)-q3(4)*q134(4)      
      q4_134 = q4(1)*q134(1)-q4(2)*q134(2)-q4(3)*q134(3)-q4(4)*q134(4)      
      q2_234 = q2(1)*q234(1)-q2(2)*q234(2)-q2(3)*q234(3)-q2(4)*q234(4)      
      q3_234 = q3(1)*q234(1)-q3(2)*q234(2)-q3(3)*q234(3)-q3(4)*q234(4)      
      q4_234 = q4(1)*q234(1)-q4(2)*q234(2)-q4(3)*q234(3)-q4(4)*q234(4)      
      q12    = q1(1)*q2(1)  - q1(2)*q2(2) - q1(3)*q2(3) - q1(4)*q2(4)
      q13    = q1(1)*q3(1)  - q1(2)*q3(2) - q1(3)*q3(3) - q1(4)*q3(4)
      q14    = q1(1)*q4(1)  - q1(2)*q4(2) - q1(3)*q4(3) - q1(4)*q4(4)
      q23    = q2(1)*q3(1)  - q2(2)*q3(2) - q2(3)*q3(3) - q2(4)*q3(4)
      q24    = q2(1)*q4(1)  - q2(2)*q4(2) - q2(3)*q4(3) - q2(4)*q4(4)
      q34    = q3(1)*q4(1)  - q3(2)*q4(2) - q3(3)*q4(3) - q3(4)*q4(4)
      q234_2 = q234(1)**2-q234(2)**2-q234(3)**2-q234(4)**2      
      q134_2 = q134(1)**2-q134(2)**2-q134(3)**2-q134(4)**2      
c 
      cfac(1) = anom_bwg(qq2,q134_2) *(q3_134*q24 -q4_134*q23)
      cfac(2) = anom_bwg(qq2,q234_2) *(q3_234*q14 -q4_234*q13)
      cfac(3) = anom_bwg(qq2,q134_2) *(q4_134*q12 -q1_134*q24)
     1        + anom_bwg(qq2,q234_2) *(q4_234*q12 -q2_234*q14)
      cfac(4) = anom_bwg(qq2,q134_2) *(q1_134*q23 -q3_134*q12)
     1        + anom_bwg(qq2,q234_2) *(q2_234*q13 -q3_234*q12)
c
      do i =1,4
        hadr(i) =  fac3* (q1(i)*cfac(1) + q2(i)*cfac(2)
     1                            + q3(i)*cfac(3) + q4(i)*cfac(4) )
      enddo
c
      return
      end
c************************************************************************
      SUBROUTINE CURR_KARLS(MNUM,PIM1,PIM2,PIM3,PIM4,HADCUR)
      INTEGER MNUM,I
      REAL  PIM1(4),PIM2(4),PIM3(4),PIM4(4)
      COMPLEX HADCUR(4)
      REAL*8 QQ2,Q1(4),Q2(4),Q3(4),Q4(4)
      COMPLEX*16 HADR(4)

      LOGICAL INIT
      DATA INIT /.TRUE./
      SAVE INIT

      IF (INIT) THEN
         CALL had1_init
         INIT = .FALSE.
      ENDIF

      IF (MNUM.EQ.1) THEN  !  PI- PI- PI+ PI0
         Q1(1)=PIM1(4) ! PI-
         Q2(1)=PIM2(4) ! PI-
         Q3(1)=PIM3(4) ! PI0
         Q4(1)=PIM4(4) ! PI+
         DO I=1,3
            Q1(1+I)=PIM1(I)       
            Q2(1+I)=PIM2(I)       
            Q3(1+I)=PIM3(I)       
            Q4(1+I)=PIM4(I)       
         ENDDO
         QQ2=(Q1(1)+Q2(1)+Q3(1)+Q4(1))**2
         DO I=2,4
            QQ2=QQ2-(Q1(I)+Q2(I)+Q3(I)+Q4(I))**2
         ENDDO
C  Tomasz Pierzchala : in HAD4() position of PI+ and Pi0 is changed 
C                      to be in correct order according to TAUOLA routines.
C         CALL HAD4(QQ2,Q1,Q2,Q3,Q4,HADR)
C
          CALL HAD4(QQ2,Q1,Q2,Q4,Q3,HADR)
C         
      ELSEIF(MNUM.EQ.2) THEN ! PI0 PI0 PI0 PI-   
         Q1(1)=PIM1(4) ! PI0
         Q2(1)=PIM2(4) ! PI0
         Q3(1)=PIM3(4) ! PI0
         Q4(1)=PIM4(4) ! PI-
         DO I=1,3
            Q1(1+I)=PIM1(I)       
            Q2(1+I)=PIM2(I)       
            Q3(1+I)=PIM3(I)       
            Q4(1+I)=PIM4(I)       
         ENDDO
         QQ2=(Q1(1)+Q2(1)+Q3(1)+Q4(1))**2
         DO I=2,4
            QQ2=QQ2-(Q1(I)+Q2(I)+Q3(I)+Q4(I))**2
         ENDDO
         CALL HAD3(QQ2,Q1,Q2,Q3,Q4,HADR)
C         
      ELSE
         WRITE(*,*)' WRONG PARAMITER IN CURR_CPC; MNUM=',MNUM
         STOP
      ENDIF
C
      HADCUR(4)=HADR(1)
      DO I=1,3
         HADCUR(I)=HADR(I+1)
      ENDDO
      RETURN
      END
      




