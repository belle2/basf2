      subroutine momenta_quad(qq,Ard,p1,q1,p2,q2,p4,jac0,jac1,cos1d)
      include 'phokhara_10.0.inc'
      real*16 q1(0:3),q2(0:3),p2(0:3),p1(0:3),p4(0:3),pboost(0:3)
      real*16 Ar(14)
      real*16 cos1min,cos1max,cos2min,cos2max,cos3min,cos3max
      real*8 Ard(14)
      real*16 cos3,phi3,qq,qqmin,qqmax
      real*16 gmin_q,q2_max_c_q,q2_min_c_q
      real*16 cos1,phi1,el_m2
      real*8 jac0,jac1,cos1d
      integer ii

      cos1min = COS(phot2cut*piq/180.q0)     ! photon1 angle cuts in the 
      cos1max = COS(phot1cut*piq/180.q0)     ! LAB rest frame            
      cos2min = -1.q0                        ! photon2 angle limits      
      cos2max =  1.q0                        !                           
      cos3min = -1.q0!cos(160.q0*piq/180.q0)!-1.q0                        ! hadrons/muons angle limits 
      cos3max = 1.q0!cos(20.q0*piq/180.q0) !1.q0 
      qqmin=(2.q0*0.13957018q0)**2
  
      do ii=1,14
       Ar(ii)=Ard(ii)
      enddo

      gmin_q=gmin
      q2_max_c_q=q2_max_c
      q2_min_c_q=q2_min_c

      qqmax = Sp_q-2.q0*sqrt(Sp_q)*gmin_q         ! if only one photon 
      if (q2_max_c_q.lt.qqmax) qqmax=q2_max_c_q  ! external cuts      
c -------------------
      if ( (q2_min_c_q.gt.qqmin).and.
     & (q2_min_c_q.lt. (Sp_q*(1.q0-2.q0*(gmin_q/sqrt(Sp_q)+w_q)))))then 
          qqmin=q2_min_c_q
       endif

c LEPTON MOMENTA
      p1(0) = ebeam_q
      p1(1) = 0.q0
      p1(2) = 0.q0
      p1(3) = sqrt(ebeam_q**2-el_m2q)
      p4(0) = ebeam_q
      p4(1) = 0.q0
      p4(2) = 0.q0
      p4(3) = -p1(3)

       call qquadrat_quad(qqmin,qqmax,cos3min,cos3max,Ar
     1,qq,cos3,phi3,jac0)

c       print*,'qq=', qq

      call photonangles1_quad(cos1min,cos1max,Ar,cos1,phi1,jac1)
      call leptonmomenta1_quad(qq,cos1,phi1,p2,pboost)
      call hadronmomenta_quad(qq,cos3,phi3,q1,q2,pboost)

      cos1d=dble(cos1)
      do ii=0,3
      momenta(3,ii)=dble(p2(ii))
      momenta(4,ii)=0.d0
      momenta(6,ii)=dble(q1(ii))
      momenta(7,ii)=dble(q2(ii))
      enddo


c       do ii=0,3
c       print*,'cons=', p1(ii)+p4(ii)-p2(ii)-q1(ii)-q2(ii)
c
c       enddo


      end
c---------------------------------------------------------------------
c---------------------------------------------------------------------     
       subroutine qquadrat_quad(qqmin,qqmax,cosmin,cosmax,Ar
     1,qq,costheta,phi,jac0)
      include 'phokhara_10.0.inc'
      real*16 qqmin,qqmax,Ar(14),qq,jacobian,fak1c,
     &  x,a,b,c,amin,amax,bmin,bmax,fak1,fak2,fak3,p,y,ppp,jac4,vv,eqt,
     &  cosmin,cosmax,costheta,phi,delcos,vol1,vol2,vol3,vol4,cmin,cmax,
     &  delyy,yy,ymin,ea,q0p,q2p,q0b,q2b,fak4,d,dmin,dmax,fak6,fak7,
     &  e_1,e_1min,e_1max
      real*16 pionFF,dps
      real*16 m_rho0_pion_quad,g_rho0_pion_quad
      real*8 jac0,jac0t


      m_rho0_pion_quad=0.773945036q0
      g_rho0_pion_quad=0.144923q0

      x = Ar(1) 

      if((FF_pion.eq.0).or.(FF_pion.eq.1)) then 
           fak4 = 1.q0/320.q0
           fak6 = 1.q0/320.q0
        endif

        if(FF_pion.eq.2) then
           fak4 = 1.q0/308.q0   
           fak6 = 1.q0/308.q0   
        endif

c  isr only
      if(fsr.eq.0)then

        call qquadrat_1_quad(qqmin,qqmax,x,Ar(7),
     1                  m_rho0_pion_quad,g_rho0_pion_quad
     2 ,fak4,fak6,qq,jac0)
c
        call pionangles_quad(cosmin,cosmax,Ar,costheta,phi,jac0t)

       jac0=jac0*jac0t
c     
c
c isr+fsr+int
      else

      fak1 = -1.q0/Sp_q
      amin = fak1*log(Sp_q-qqmin)
      a = fak1 *log((Sp_q-qqmax)/(Sp_q-qqmin))
      delcos = (cosmax-cosmin)
      vol1 = a*delcos
c
      fak2 = 1.q0/g_rho0_pion/m_rho0_pion
      bmin = fak2*atan((qqmin-m_rho0_pion**2)*fak2)
      bmax = fak2*atan((qqmax-m_rho0_pion**2)*fak2)
      b = bmax-bmin
      vol2 = b*delcos
c
      vv = sqrt(1.q0-4.q0*mpi_q**2/Sp_q)
      delyy = log((1.q0+vv*cosmax)*(1.q0-vv*cosmin)
     1          /(1.q0-vv*cosmax)/(1.q0+vv*cosmin)) /vv
      vol3 = a*delyy 
c
      phi = 2.q0*piq*Ar(5)
c
c --- three channels ---
c 1. soft photon + angles flat
c 2. BW q2       + angles flat
c 3. soft photon + angles fsr collinear
c
      p = Ar(7)      

        ppp  = vol1+vol2+vol3
        if(p.lt.vol1/ppp)then
          y  = amin+a*x
          qq = Sp_q-exp(y/fak1)                                       
          costheta = cosmin+delcos*Ar(4)
        elseif(p.lt.((vol1+vol2)/ppp))then
          y  = bmin+b*x
          qq = m_rho0_pion*(m_rho0_pion+g_rho0_pion*tan(y/fak2))
          costheta = cosmin+delcos*Ar(4)
        else
          y  = amin+a*x
          qq = Sp_q-exp(y/fak1) 
          ymin = log((1.q0+vv*cosmin)/(1.q0-vv*cosmin)) /vv
          yy = ymin + delyy *Ar(4)
          ea = exp(vv*yy)
          costheta = (ea-1.q0)/(1.q0+ea)/vv
        endif
        jac0 = 2.d0*pi*ppp/( (1.d0/(Sp*(Sp-qq)) +
     &     1.d0/(((qq-m_rho0_pion**2)**2+(g_rho0_pion*m_rho0_pion)**2))) 
     &    +1.d0/(Sp*(Sp-qq))
     &    *(1.d0/(1.d0-vv*costheta)+1.d0/(1.d0+vv*costheta)) )

      endif

      end
c----------------------------------------------------------------------
c----------------------------------------------------------------------
       subroutine qquadrat_1_quad(qqmin,qqmax,x,p,mmm,ggg,fak5,fak6,
     1                                                   qq,jac0)
      include 'phokhara_10.0.inc'
      real*16 qqmin,qqmax,Ar(14),qq,jacobian,mmm,ggg,
     &  x,a,b,amin,amax,bmin,bmax,fak1,fak2,p,y,ppp,fak5,c,cmin,cmax,
     2  fak6,d,dmin,dmax
c
      real*8 jac0


      
c
      fak1 = -1.q0/Sp_q
      amin = fak1*log(Sp_q-qqmin)
      amax = fak1*log(Sp_q-qqmax)
      a = amax-amin
      fak2 = 1.q0/ggg/mmm
      bmin = fak2*atan((qqmin-mmm**2)*fak2)
      bmax = fak2*atan((qqmax-mmm**2)*fak2)
      b = bmax-bmin

      



c --- 3 channels ---
c 1 1/Sp/(Sp-qq)
c 2 BW m_rho0_pion
c 3 J/Psi BW
c 4 Psi(2S) BW

!J/Psi
         if(narr_res.eq.1)then

           cmin=Qjp_q**2*mjp_q*gamjp_q*fak5*atan((qqmin-mjp_q**2)
     1 /(mjp_q*gamjp_q))
           cmax=Qjp_q**2*mjp_q*gamjp_q*fak5*atan((qqmax-mjp_q**2)
     1 /(mjp_q*gamjp_q))
           c = cmax-cmin 

            ppp  = a+b+c
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp_q-exp(y/fak1)                                       
            elseif(p.lt.((a+b)/ppp))then
               y  = bmin+b*x
               qq = mmm*(mmm+ggg*tan(y/fak2)) 
            else
               y = cmin+c*x
               qq = mjp_q*gamjp_q*tan(y/fak5/mjp_q/gamjp_q/Qjp_q**2)
     1  + mjp_q**2        
            endif
       jac0 = ppp/( (1.d0/(Sp*(Sp-qq)) 
     1      + 1.d0/(((qq-mmm**2)**2+(ggg*mmm)**2))) + Qjp_q**2*mjp**2
     2        *gamjp**2*fak5/((mjp**2-qq)**2+mjp**2*gamjp**2) )

         else
            ppp  = a+b
c
            if(p.lt.a/ppp)then
               y  = amin+a*x
               qq = Sp_q-exp(y/fak1)                                       
            else
               y  = bmin+b*x
               qq = mmm*(mmm+ggg*tan(y/fak2)) 
            endif
        jac0 = ppp/( (1.d0/(Sp*(Sp-qq)) 
     1               + 1.d0/(((qq-mmm**2)**2+(ggg*mmm)**2))) )
         endif
      return
      end
c----------------------------------------------------------------------
c----------------------------------------------------------------------
      subroutine pionangles_quad(cosmin,cosmax,Ar,costheta,phi,jac0t)
      include 'phokhara_10.0.inc'
      real*16 cosmin,cosmax,Ar(14),costheta,phi,jacobian,x
      real*8 jac0t

      x   = Ar(4)
      phi = 2.q0*piq*Ar(5)

c --- flat ---
      costheta = cosmin+(cosmax-cosmin)*x
      jac0t = 2.d0*dble(piq)*(cosmax-cosmin)
      return
      end
c---------------------------------------------------------------------
      subroutine photonangles1_quad(cosmin,cosmax,Ar,costheta,phi,jac1)
      include 'phokhara_10.0.inc'
      real*16 cosmin,cosmax,Ar(14),costheta,phi,jacobian,x,b,cmin,cmax,y
     1      , cosmin2,cosmax2,cmaxmcmin,rat
      real*8 jac1

      x   = Ar(2)
      phi = 2.q0*piq*Ar(3)


c      fak3 = 2.q0
      rat = 4.q0*me_q*me_q/Sp_q
      cosmin2 = cosmin**2
      cosmax2 = cosmax**2
c --- flat ---
c      costheta = cosmin+(cosmax-cosmin)*x
c      jacobian = 2.q0*pi*(cosmax-cosmin)
c --- peaked at costheta = +-1 ---

      b = sqrt(1.q0-rat)
      cmin = log((1.q0-cosmin2+rat*cosmin2)/(1.q0-b*cosmin)**2)
     1      /(2.q0*b)
      cmaxmcmin = log((1.q0+b*cosmax)**2*(1.q0-b*cosmin)**2
     1          /(1.q0-cosmax2+rat*cosmax2)/(1.q0-cosmin2+rat*cosmin2))
     2          / (2.q0*b)

         y = cmin+x*cmaxmcmin
         costheta = tanh(b*y)/b

c onemb2c2 is used in Bornvirtualsoft
      onemb2c2 = 1.q0/(cosh(b*y))**2
      jac1 = 2.d0*pi*onemb2c2*cmaxmcmin
c
c
      return
      end
c---------------------------------------------------------------------
      subroutine leptonmomenta1_quad(qq,costheta,phi,p2,pboost)
      include 'phokhara_10.0.inc'
      real*16 qq,q2,E,costheta,sintheta,phi,p2(0:3),pboost(0:3)
      integer i

      q2 = qq/Sp_q
      sintheta = sqrt(1.q0-costheta*costheta)
c --- real photon1 ---
      E = (1.q0-q2)*sqrt(Sp_q)/2.q0
      p2(0) = E
      p2(1) = E*sintheta*cos(phi)
      p2(2) = E*sintheta*sin(phi)
      p2(3) = E*costheta
      pboost(0) = (1.q0+q2)*sqrt(Sp_q)/2.q0
      do i = 1,3
        pboost(i) = -p2(i)
      enddo
      return
      end
c----------------------------------------------------------------------
c---------------------------------------------------------------------- 
      subroutine hadronmomenta_quad(qq,costheta,phi,q1,q2,pboost)
      include 'phokhara_10.0.inc'
      real*16 qq,p,costheta,sintheta,phi,m2,
     &        cmsvector(0:3),boostvector(0:3),labvector(0:3),
     & q1(0:3),q2(0:3),pboost(0:3)
      integer i

      sintheta = sqrt(1.q0-costheta*costheta)
      
         m2 = mpi_q*mpi_q

c --- pions/muons/protons/neutrons/kaons/lambdas in the qq-rest frame ---
c --- pi^+/mu^+/pbar/nbar/K^+/K^0/lambda bar ---
      q1(0) = sqrt(qq)/2.q0
      p            = q1(0)*sqrt(1.q0-4.q0*m2/qq)
      q1(1) = p*sintheta*cos(phi)
      q1(2) = p*sintheta*sin(phi)
      q1(3) = p*costheta
c --- pi^-/mu^-/p/n/K^-/K^0bar/lambda---
      q2(0) = q1(0) 
      do i = 1,3
        q2(i) = -q1(i)
      enddo
c --- boost the hadron momenta into the e^+ e^- CMS ---
      call hadronmomenta_1_quad(q1,q2,pboost)
      return
      end
c ------------------------------------------------------------------
c        boost four momenta from Q rest frame to e^+ e^- CMS             
c ------------------------------------------------------------------
      subroutine hadronmomenta_1_quad(q1,q2,pboost)
      include 'phokhara_10.0.inc'
      real*16 p,costheta,sintheta,phi,m2,
     &        cmsvector(0:3),boostvector(0:3),labvector(0:3),
     1        q1(0:3),q2(0:3),pboost(0:3)
      integer i,j,Np
c      
c --- boost the hadron momenta into the e^+ e^- CMS ---
c
      do i =0,3
         boostvector(i) = pboost(i)
      enddo   
c
         do i =0,3
            cmsvector(i) = q1(i)
         enddo   
         call boost_quad(cmsvector,boostvector,labvector)
         do i =0,3
            q1(i) = labvector(i) 
         enddo         

         do i =0,3
            cmsvector(i) = q2(i)
         enddo   
         call boost_quad(cmsvector,boostvector,labvector)
         do i =0,3
            q2(i) = labvector(i) 
         enddo
      return
      end   
c---------------------------------------------------------------------
      subroutine boost_quad(cmsvector,boostvector,labvector)
      implicit none
      real*16 cmsvector(0:3),boostvector(0:3),labvector(0:3),
     1   m(0:3,0:3),E,p,beta,gamma,costheta,sintheta,cosphi,sinphi
      integer i,j
      
      E = boostvector(0)
      p = sqrt(boostvector(1)**2+boostvector(2)**2+
     &    boostvector(3)**2)
      beta  = p/E
      gamma = 1.q0/sqrt(1.q0-beta*beta)
      costheta = boostvector(3)/p
      sintheta = sqrt(boostvector(1)**2+boostvector(2)**2)/p
      if(sintheta.ne.0.q0)then
        cosphi   = boostvector(1)/(p*sintheta)
        sinphi   = boostvector(2)/(p*sintheta)
      else
        cosphi = 1.q0
        sinphi = 0.q0
      endif

        m(0,0) = gamma
        m(0,1) = 0.q0
        m(0,2) = 0.q0
        m(0,3) = beta*gamma
        m(1,0) = beta*gamma*sintheta*cosphi
        m(1,1) = costheta*cosphi
        m(1,2) = -sinphi
        m(1,3) = gamma*sintheta*cosphi
        m(2,0) = beta*gamma*sintheta*sinphi
        m(2,1) = costheta*sinphi
        m(2,2) = cosphi
        m(2,3) = gamma*sintheta*sinphi
        m(3,0) = beta*gamma*costheta
        m(3,1) = -sintheta
        m(3,2) = 0.q0
        m(3,3) = gamma*costheta

      do i=0,3
         labvector(i) = 0.q0
         do j=0,3
            labvector(i) = labvector(i)+m(i,j)*cmsvector(j)
         enddo
      enddo

      return
      end
