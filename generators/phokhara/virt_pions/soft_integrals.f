      subroutine softintegral_p1p2(softint_p1p2)
      include '../phokhara_10.0.inc'
      real*8 dme,el_m2,p1(4),p2(4),lambda_photon,photon_cutoff
      real*8 discrim1,alpha1a,alpha1b,p1p2,alphaHV,mom1dl,mom2dl,vRatio
      real*8 zmienna1,zmienna2,softint_p1p2
      complex*16 cdilog
      common /cp1p2/p1,p2,dme,el_m2
c
c      lambda_photon = dme
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
c
      p1p2=momenta(1,0)*momenta(2,0)-momenta(1,1)*momenta(2,1)
     1 -momenta(1,2)*momenta(2,2)-momenta(1,3)*momenta(2,3)

      discrim1 =  dsqrt((p1p2)**2 - el_m2**2)
c
      alpha1a = ((p1p2) - discrim1) / el_m2
      alpha1b = ((p1p2) + discrim1) / el_m2
c
      zmienna1 = alpha1a * momenta(1,0) - momenta(2,0)
      zmienna2 = alpha1b * momenta(1,0) - momenta(2,0)
c
      if ((momenta(2,0) .gt. 0) .and. (zmienna1 .gt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(2,0) .lt. 0) .and. (zmienna1 .lt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(2,0) .gt. 0) .and. (zmienna2 .gt. 0)) then
         alphaHV = alpha1b
      else if ((momenta(2,0) .lt. 0) .and. (zmienna2 .lt. 0)) then
         alphaHV = alpha1b
      else
      write(*,*) "INNY ZNAK alpha w obu przypadkach"
      endif      
c
      mom1dl = dsqrt(momenta(1,1)**2+momenta(1,2)**2+momenta(1,3)**2)
c
      mom2dl = dsqrt(momenta(2,1)**2+momenta(2,2)**2+momenta(2,3)**2)
c
      vRatio = el_m2 * (alphaHV**2 - 1.d0)/ 2.d0 / 
     &  (alphaHV * momenta(1,0) - momenta(2,0))
c
c softint=-alpha_em/(4pi^2)*p1p2*L_ij with L_ij in Eq.(7.3)[t'Hooft-Veltman]
c with substitutions for metrics (+---): v -> -v, q^2 -> -q^2, p^2 -> -p^2
      softint_p1p2 = 
     & - alpha/pi *  p1p2 *                
     & alphaHV / (el_m2 * (alphaHV**2 - 1.d0)) * (                     
     &   log(alphaHV**2)*log((2.d0*photon_cutoff/lambda_photon)**2)/2.d0
c
     & + ( (log(el_m2 /(momenta(1,0)+ mom1dl)**2))**2                  
     &   - (log(el_m2 /(momenta(2,0)+ mom2dl)**2))**2 )/4.d0
c
     & + cdilog(dcmplx((1.d0 - alphaHV*(momenta(1,0) + mom1dl)/vRatio)
     &         ,0.d0))         
     & - cdilog(dcmplx((1.d0 - (momenta(2,0) + mom2dl)/vRatio),0.d0))
c                
     & + cdilog(dcmplx((1.d0 - alphaHV*el_m2/vRatio/ 
     &            (momenta(1,0)+mom1dl)),0.d0))
     & - cdilog(dcmplx((1.d0 - el_m2/vRatio/ (momenta(2,0)+mom2dl))
     &         ,0.d0)) 
     &  )
c
c      write(*,*) vRatio
c
      return
      end

c***********************************************************************
      subroutine softintegral_pjpj(softint_pjpj)
      include '../phokhara_10.0.inc'
      real*8 dme,el_m2,p1(4),p2(4),lambda_photon,photon_cutoff
      real*8 softint_pjpj,mom1dl
      common /cp1p2/p1,p2,dme,el_m2
c
c      lambda_photon = dme
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
c
      mom1dl = sqrt(momenta(1,1)**2+momenta(1,2)**2+momenta(1,3)**2)
c
c softint=-alpha_em/(4pi^2)*me2*L_jj
      softint_pjpj = - alpha/pi * ( 
     & log(2.d0*photon_cutoff/lambda_photon) 
     & - momenta(1,0)/mom1dl
     & * log((mom1dl + momenta(1,0))**2/el_m2)/2.d0
     & )
c
      return
      end

c***********************************************************************
c FSR soft integral (m_pi)
c***********************************************************************
c     lambda (photon mass) is chosen as mpi
c     q1 and q2 are pion momenta

      subroutine softintegral_q1q2(softint_q1q2)
      include '../phokhara_10.0.inc'
      real*8 dme,el_m2,p1(4),p2(4),lambda_photon,photon_cutoff
      real*8 softint_q1q2,discrim1,alpha1a,alpha1b,pi1pi2,zmienna1,
     1 zmienna2,alphaHV,vRatio,mom1dl,mom2dl
      complex*16 cdilog
      common /cp1p2/p1,p2,dme,el_m2
c
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
    
       pi1pi2=momenta(6,0)*momenta(7,0)-momenta(6,1)*momenta(7,1)
     1 -momenta(6,2)*momenta(7,2)-momenta(6,3)*momenta(7,3)
c        
      discrim1 =  sqrt((pi1pi2)**2 - mpi**4)
c
      alpha1a = ((pi1pi2) - discrim1) / mpi**2
      alpha1b = ((pi1pi2) + discrim1) / mpi**2
c
      zmienna1 = alpha1a * momenta(6,0) - momenta(7,0)
      zmienna2 = alpha1b * momenta(6,0) - momenta(7,0)
c
      if ((momenta(7,0) .gt. 0) .and. (zmienna1 .gt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .lt. 0) .and. (zmienna1 .lt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .gt. 0) .and. (zmienna2 .gt. 0)) then
         alphaHV = alpha1b
      else if ((momenta(7,0) .lt. 0) .and. (zmienna2 .lt. 0)) then
         alphaHV = alpha1b
      else
      write(*,*) "INNY ZNAK alpha w obu przypadkach"
      endif      
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
c
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
c
      vRatio = mpi**2 * (alphaHV**2 - 1.d0)/ 2.d0 / 
     &  (alphaHV * momenta(6,0) - momenta(7,0))
c
c softint=-alpha_em/(4pi^2)*pi1pi2*L_ij with L_ij in Eq.(7.3)[t'Hooft-Veltman]
c with substitutions for metrics (+---): v -> -v, q^2 -> -q^2, p^2 -> -p^2
      softint_q1q2 = - alpha/pi *  pi1pi2 *                
     & alphaHV / (mpi**2 * (alphaHV**2 - 1.d0))*(                    
     & log(alphaHV**2)*log((2.d0*photon_cutoff/lambda_photon)**2)/2.d0)
c     & + 
c     &   (log(mpi**2 /(momenta(6,0)+ mom1dl)**2))**2/4.d0                  
c     & - (log(mpi**2 /(momenta(7,0)+ mom2dl)**2))**2/4.d0              
c     & + cdilog(dcmplx(1.d0 - alphaHV*(momenta(6,0) + mom1dl)/vRatio))         
c     & - cdilog(dcmplx(1.d0 - (momenta(7,0) + mom2dl)/vRatio))                 
c     & + cdilog(dcmplx(1.d0 - alphaHV*mpi**2 /vRatio/ 
c     &            (momenta(6,0)+mom1dl)))
c     & - cdilog(dcmplx(1.d0 - mpi**2 /vRatio/ (momenta(7,0)+mom2dl))) 
c     & )
c
c      write(*,*) "softint_q1q2 = ",softint_q1q2
c
      return
      end
c
c-------------------------------------------------------------------
      subroutine softintegral_qjqj(softint_qjqj)
      include '../phokhara_10.0.inc'
      real*8 lambda_photon,photon_cutoff
      real*8 softint_qjqj,mom1dl,mom2dl
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
c
c softint=-alpha_em/(4pi^2)*(mpi**2)*L_jj
      softint_qjqj = - alpha/pi*( 
     & log(2.d0*photon_cutoff/lambda_photon) )
c     & -( momenta(6,0)/mom1dl
c     &      *log((mom1dl + momenta(6,0))**2/mpi**2)/2.d0
c     &   + momenta(7,0)/mom2dl
c     &      *log((mom2dl + momenta(7,0))**2/mpi**2)/2.d0
c     &  )/2.d0
c     & )
c
c      write(*,*) "softint_qjqj = ",softint_qjqj
c
      print*, 'Log=',log((2.d0*photon_cutoff/lambda_photon)**2)
      return
      end
c***********************************************************************
c soft integrals for ISR and FSR interference (m_e, m_pi)
c*********************************************************************** 

      subroutine softintegral_interference(softint_sum)
      include '../phokhara_10.0.inc'
      real*8 p1(4),p2(4),q1(4),q2(4)
      real*8 dme,el_m2,softint_mom1mom2
      real*8 softint_sum,softint_p1q1,softint_p1q2,softint_p2q1,
     1 softint_p2q2,pi1p1,pi2p1,pi1p2,pi2p2 
      common /cp1p2/p1,p2,dme,el_m2
c
c      integer, parameter :: out_unit=20
c____________________________________________________________________
c  p1 - e+
      p1(1)=momenta(1,0)
      p1(2)=momenta(1,1)
      p1(3)=momenta(1,2)
      p1(4)=momenta(1,3)
c
c  p2 - e-
      p2(1)=momenta(2,0)
      p2(2)=momenta(2,1)
      p2(3)=momenta(2,2)
      p2(4)=momenta(2,3)
c
c q1 - pi+
      q1(1)=momenta(6,0)
      q1(2)=momenta(6,1)
      q1(3)=momenta(6,2)
      q1(4)=momenta(6,3)
c
c  q2 - pi-
      q2(1)=momenta(7,0)
      q2(2)=momenta(7,1)
      q2(3)=momenta(7,2)
      q2(4)=momenta(7,3)

       pi1p1=momenta(6,0)*momenta(1,0)-momenta(6,1)*momenta(1,1)
     1 -momenta(6,2)*momenta(1,2)-momenta(6,3)*momenta(1,3)

       pi2p1=momenta(7,0)*momenta(1,0)-momenta(7,1)*momenta(1,1)
     1 -momenta(7,2)*momenta(1,2)-momenta(7,3)*momenta(1,3)

       pi1p2=momenta(6,0)*momenta(2,0)-momenta(6,1)*momenta(2,1)
     1 -momenta(6,2)*momenta(2,2)-momenta(6,3)*momenta(2,3)

       pi2p2=momenta(7,0)*momenta(2,0)-momenta(7,1)*momenta(2,1)
     1 -momenta(7,2)*momenta(2,2)-momenta(7,3)*momenta(2,3)
c
c      open (unit=out_unit,file="results_02-22-2016.txt",action="write")
c
c      write(out_unit,*) "      q1(1)=",q1(1)
c      write(out_unit,*) "      q1(2)=",q1(2)
c      write(out_unit,*) "      q1(3)=",q1(3)
c      write(out_unit,*) "      q1(4)=",q1(4)
c      write(out_unit,*)
c      write(out_unit,*) "      q2(1)=",q2(1)
c      write(out_unit,*) "      q2(2)=",q2(2)
c      write(out_unit,*) "      q2(3)=",q2(3)
c      write(out_unit,*) "      q2(4)=",q2(4)
c
c      close (out_unit)
c____________________________________________________________________
c
      call softintegral_interf(q1,p1,mpi,dme,pi1p1,softint_mom1mom2)
      softint_p1q1 = softint_mom1mom2
c
      call softintegral_interf(q2,p1,mpi,dme,pi2p1,softint_mom1mom2)
      softint_p1q2 = softint_mom1mom2
c
      call softintegral_interf(q1,p2,mpi,dme,pi1p2,softint_mom1mom2)
      softint_p2q1 = softint_mom1mom2
c
      call softintegral_interf(q2,p2,mpi,dme,pi2p2,softint_mom1mom2)
      softint_p2q2 = softint_mom1mom2
c
      softint_sum =  
     1  - softint_p1q1 - softint_p2q2
     2  + softint_p1q2 + softint_p2q1 

      softint_sum =  
     1  - softint_p1q1 - softint_p2q2
     2  + softint_p1q2 + softint_p2q1
c
      return
      end

c***********************************************************************
c     gmin[w] is soft photon cutoff

      subroutine softintegral_interf(mom1,mom2,mass1,mass2,
     &  scalar_prod,softint_mom1mom2)
      include '../phokhara_10.0.inc'
      real*8 mom1(4),mom2(4),mass1,mass2,scalar_prod,softint_mom1mom2
      real*8 mom1mom1_3,mom2mom2_3
      real*8 lambda_photon,photon_cutoff
      real*8 discrim1,alpha1a,alpha1b,zmienna1,zmienna2,alphaHV,
     1 mom1dl,mom2dl,vRatio
      complex*16 cdilog


c
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
c
      discrim1 =  sqrt(scalar_prod**2 - mass2**2*mass1**2)
c
      alpha1a = (scalar_prod - discrim1) / mass1**2
      alpha1b = (scalar_prod + discrim1) / mass1**2
c
      zmienna1 = alpha1a * mom1(1) - mom2(1)
      zmienna2 = alpha1b * mom1(1) - mom2(1)
c
      if ((mom2(1) .gt. 0) .and. (zmienna1 .gt. 0)) then
         alphaHV = alpha1a
      else if ((mom2(1) .lt. 0) .and. (zmienna1 .lt. 0)) then
         alphaHV = alpha1a
      else if ((mom2(1) .gt. 0) .and. (zmienna2 .gt. 0)) then
         alphaHV = alpha1b
      else if ((mom2(1) .lt. 0) .and. (zmienna2 .lt. 0)) then
         alphaHV = alpha1b
      else
c
        write(*,*) "INNY ZNAK alpha w obu przypadkach"
c
      endif      
c
      call prod_3(mom1,mom1,mom1mom1_3)
      call prod_3(mom2,mom2,mom2mom2_3)
      mom1dl = sqrt(mom1mom1_3)
      mom2dl = sqrt(mom2mom2_3)

c
      vRatio = (alphaHV**2 * mass1**2 - mass2**2)/ 2.d0 / 
     &  (alphaHV * mom1(1) - mom2(1))
c
c softint=-alpha_em/(4pi^2)*pi1p1*L_ij with L_ij in Eq.(7.3)[t'Hooft-Veltman]
c with substitutions for metrics (+---): v -> -v, q^2 -> -q^2, p^2 -> -p^2
      softint_mom1mom2 = 
     & -alpha/pi *  scalar_prod *                
     &  alphaHV / (alphaHV**2*mass1**2 - mass2**2) * 
     & (                    
     & log(alphaHV**2*mass1**2/mass2**2
     &   )*log((2.d0*photon_cutoff/lambda_photon)**2)/2.d0
c     & ) 
     & + (log(mass1**2 /(mom1(1)+ mom1dl)**2))**2/4.d0                  
     & - (log(mass2**2 /(mom2(1)+ mom2dl)**2))**2/4.d0                  
     & + cdilog(dcmplx(1.d0 - alphaHV*(mom1(1) + mom1dl)/vRatio))         
     & - cdilog(dcmplx(1.d0 - (mom2(1) + mom2dl)/vRatio)) 
     & + cdilog(dcmplx(1.d0 - alphaHV*mass1**2 /vRatio/ 
     &            (mom1(1)+mom1dl)))
     & - cdilog(dcmplx(1.d0 - mass2**2 /vRatio/(mom2(1)+mom2dl)))
     & )
c
      return
      end
c
c
c
c
c subroutines for subtraction
c
c***********************************************************************
c FSR soft integral (m_pi)
c***********************************************************************
c     lambda (photon mass) is chosen as mpi
c     q1 and q2 are pion momenta

      subroutine softintegral_q1q2_sub(softint_q1q2)
      include '../phokhara_10.0.inc'
      real*8 dme,el_m2,p1(4),p2(4),lambda_photon,photon_cutoff
      real*8 softint_q1q2,discrim1,alpha1a,alpha1b,pi1pi2,zmienna1,
     1 zmienna2,alphaHV,vRatio,mom1dl,mom2dl,qq2
      complex*16 cdilog
      common /cp1p2/p1,p2,dme,el_m2
c
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)

      qq2=(momenta(6,0)+momenta(7,0))**2-(momenta(6,1)+momenta(7,1))**2
     1 -(momenta(6,2)+momenta(7,2))**2-(momenta(6,3)+momenta(7,3))**2
    
       pi1pi2=momenta(6,0)*momenta(7,0)-momenta(6,1)*momenta(7,1)
     1 -momenta(6,2)*momenta(7,2)-momenta(6,3)*momenta(7,3)
c        
      discrim1 =  sqrt((pi1pi2)**2 - mpi**4)
c
      alpha1a = ((pi1pi2) - discrim1) / mpi**2
      alpha1b = ((pi1pi2) + discrim1) / mpi**2
c
      zmienna1 = alpha1a * momenta(6,0) - momenta(7,0)
      zmienna2 = alpha1b * momenta(6,0) - momenta(7,0)

c
      if ((momenta(7,0) .gt. 0) .and. (zmienna1 .gt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .lt. 0) .and. (zmienna1 .lt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .gt. 0) .and. (zmienna2 .gt. 0)) then
         alphaHV = alpha1b
      else if ((momenta(7,0) .lt. 0) .and. (zmienna2 .lt. 0)) then
         alphaHV = alpha1b
      else
      write(*,*) "INNY ZNAK alpha w obu przypadkach"
      endif      
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
c
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
c
      vRatio = mpi**2 * (alphaHV**2 - 1.d0)/ 2.d0 / 
     &  (alphaHV * momenta(6,0) - momenta(7,0))
c
c softint=-alpha_em/(4pi^2)*pi1pi2*L_ij with L_ij in Eq.(7.3)[t'Hooft-Veltman]
c with substitutions for metrics (+---): v -> -v, q^2 -> -q^2, p^2 -> -p^2
      softint_q1q2 = - alpha/pi *  pi1pi2 *                
     & alphaHV / (mpi**2 * (alphaHV**2 - 1.d0))*(                    
     & log(alphaHV**2)*log((2.d0*photon_cutoff/lambda_photon)**2)/2.d0
     & -log(alphaHV**2)*log(qq2/Sp)/2.d0
     &   +(log(mpi**2 /(momenta(6,0)+ mom1dl)**2))**2/4.d0                  
     & - (log(mpi**2 /(momenta(7,0)+ mom2dl)**2))**2/4.d0              
     & + cdilog(dcmplx(1.d0 - alphaHV*(momenta(6,0) + mom1dl)/vRatio))         
     & - cdilog(dcmplx(1.d0 - (momenta(7,0) + mom2dl)/vRatio))                 
     & + cdilog(dcmplx(1.d0 - alphaHV*mpi**2 /vRatio/ 
     &            (momenta(6,0)+mom1dl)))
     & - cdilog(dcmplx(1.d0 - mpi**2 /vRatio/ (momenta(7,0)+mom2dl))) 
     & )
c
c      write(*,*) "softint_q1q2 = ",softint_q1q2
c
      return
      end
c
c-------------------------------------------------------------------
      subroutine softintegral_qjqj_sub(softint_qjqj)
      include '../phokhara_10.0.inc'
      real*8 lambda_photon,photon_cutoff
      real*8 softint_qjqj,mom1dl,mom2dl,qq2
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)

      qq2=(momenta(6,0)+momenta(7,0))**2-(momenta(6,1)+momenta(7,1))**2
     1 -(momenta(6,2)+momenta(7,2))**2-(momenta(6,3)+momenta(7,3))**2
c
c softint=-alpha_em/(4pi^2)*(mpi**2)*L_jj
      softint_qjqj = - alpha/pi*( 
     & log(2.d0*photon_cutoff/lambda_photon) - Log(qq2/Sp)/2.d0
     & -( momenta(6,0)/mom1dl
     &      *log((mom1dl + momenta(6,0))**2/mpi**2)/2.d0
     &   + momenta(7,0)/mom2dl
     &      *log((mom2dl + momenta(7,0))**2/mpi**2)/2.d0
     &  )/2.d0
     & )
c
c      write(*,*) "softint_qjqj = ",softint_qjqj
c
      return
      end

c***********************************************************************
c FSR soft integral (m_pi) for finite part only!!!!!!!
c***********************************************************************
c     lambda (photon mass) is chosen as mpi
c     q1 and q2 are pion momenta

      subroutine softintegral_q1q2_fin(softint_q1q2)
      include '../phokhara_10.0.inc'
      real*8 dme,el_m2,p1(4),p2(4),lambda_photon,photon_cutoff
      real*8 softint_q1q2,discrim1,alpha1a,alpha1b,pi1pi2,zmienna1,
     1 zmienna2,alphaHV,vRatio,mom1dl,mom2dl,qq2
      complex*16 cdilog
      common /cp1p2/p1,p2,dme,el_m2
c
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)
    
       pi1pi2=momenta(6,0)*momenta(7,0)-momenta(6,1)*momenta(7,1)
     1 -momenta(6,2)*momenta(7,2)-momenta(6,3)*momenta(7,3)

      qq2=(momenta(6,0)+momenta(7,0))**2-(momenta(6,1)+momenta(7,1))**2
     1 -(momenta(6,2)+momenta(7,2))**2-(momenta(6,3)+momenta(7,3))**2
c        
      discrim1 =  sqrt((pi1pi2)**2 - mpi**4)
c
      alpha1a = ((pi1pi2) - discrim1) / mpi**2
      alpha1b = ((pi1pi2) + discrim1) / mpi**2
c
      zmienna1 = alpha1a * momenta(6,0) - momenta(7,0)
      zmienna2 = alpha1b * momenta(6,0) - momenta(7,0)
c
      if ((momenta(7,0) .gt. 0) .and. (zmienna1 .gt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .lt. 0) .and. (zmienna1 .lt. 0)) then
         alphaHV = alpha1a
      else if ((momenta(7,0) .gt. 0) .and. (zmienna2 .gt. 0)) then
         alphaHV = alpha1b
      else if ((momenta(7,0) .lt. 0) .and. (zmienna2 .lt. 0)) then
         alphaHV = alpha1b
      else
      write(*,*) "INNY ZNAK alpha w obu przypadkach"
      endif      
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
c
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)
c
      vRatio = mpi**2 * (alphaHV**2 - 1.d0)/ 2.d0 / 
     &  (alphaHV * momenta(6,0) - momenta(7,0))
c
c softint=-alpha_em/(4pi^2)*pi1pi2*L_ij with L_ij in Eq.(7.3)[t'Hooft-Veltman]
c with substitutions for metrics (+---): v -> -v, q^2 -> -q^2, p^2 -> -p^2
      softint_q1q2 = - alpha/pi *  pi1pi2 *                
     & alphaHV / (mpi**2 * (alphaHV**2 - 1.d0))*(                    
c     & log(alphaHV**2)*log((2.d0*photon_cutoff/lambda_photon)**2)/2.d0
c     & + 
c     &  +log(alphaHV**2)*Log(Sp/qq2)/2.d0  
     &   +(log(mpi**2 /(momenta(6,0)+ mom1dl)**2))**2/4.d0                  
     & - (log(mpi**2 /(momenta(7,0)+ mom2dl)**2))**2/4.d0              
     & + cdilog(dcmplx(1.d0 - alphaHV*(momenta(6,0) + mom1dl)/vRatio))         
     & - cdilog(dcmplx(1.d0 - (momenta(7,0) + mom2dl)/vRatio))                 
     & + cdilog(dcmplx(1.d0 - alphaHV*mpi**2 /vRatio/ 
     &            (momenta(6,0)+mom1dl)))
     & - cdilog(dcmplx(1.d0 - mpi**2 /vRatio/ (momenta(7,0)+mom2dl))) 
     & )
c
c      write(*,*) "softint_q1q2 = ",softint_q1q2
c
      return
      end
c
c-------------------------------------------------------------------
      subroutine softintegral_qjqj_fin(softint_qjqj)
      include '../phokhara_10.0.inc'
      real*8 lambda_photon,photon_cutoff
      real*8 softint_qjqj,mom1dl,mom2dl,qq2
c      lambda_photon = mpi
      lambda_photon = dsqrt(Sp)
      photon_cutoff = w*dsqrt(Sp)

      
c
      mom1dl = sqrt(momenta(6,1)**2+momenta(6,2)**2+momenta(6,3)**2)
      mom2dl = sqrt(momenta(7,1)**2+momenta(7,2)**2+momenta(7,3)**2)

      qq2=(momenta(6,0)+momenta(7,0))**2-(momenta(6,1)+momenta(7,1))**2
     1 -(momenta(6,2)+momenta(7,2))**2-(momenta(6,3)+momenta(7,3))**2
c
c softint=-alpha_em/(4pi^2)*(mpi**2)*L_jj
      softint_qjqj = - alpha/pi*( 
c     & log(2.d0*photon_cutoff/lambda_photon) 
c     &   +Log(Sp/qq2)/2.d0
     & -( momenta(6,0)/mom1dl
     &      *log((mom1dl + momenta(6,0))**2/mpi**2)/2.d0
     &   + momenta(7,0)/mom2dl
     &      *log((mom2dl + momenta(7,0))**2/mpi**2)/2.d0
     &  )/2.d0
     & )
c
c      write(*,*) "softint_qjqj = ",softint_qjqj
c
      return
      end
