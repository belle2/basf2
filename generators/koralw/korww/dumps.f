      SUBROUTINE DUMPS(NOUT)                 
C     **********************                 
C THIS PRINTS OUT FOUR MOMENTA OF PHOTONS    
C ON UNIT NO. NOUT                           
C     **********************                 
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)    
      COMMON / MOMSET / QEFF1(4),QEFF2(4),SPHUM(4),SPHOT(100,4),NPHOT  
      SAVE   / MOMSET /
      SAVE
      DIMENSION SUM(4)                       
CC      WRITE(NOUT,*) '=====================DUMPS===================='
CC      WRITE(NOUT,3100) 'QF1',(  qeff1(  K),K=1,4) 
CC      WRITE(NOUT,3100) 'QF2',(  qeff2(  K),K=1,4) 
CC      DO 100 I=1,NPHOT                       
CC  100 WRITE(NOUT,3100) 'PHO',(SPHOT(I,K),K=1,4) 
      DO 200 K=1,4                           
  200 SUM(K)=qeff1(K)+qeff2(K)                   
      DO 210 I=1,NPHOT                       
      DO 210 K=1,4                           
  210 SUM(K)=SUM(K)+SPHOT(I,K)               
CC      WRITE(NOUT,3100) 'SUM',(  SUM(  K),K=1,4) 
 3100 FORMAT(1X,A3,1X,5F18.14)               
      END                                    

      subroutine moms(nout)
c     *********************
      implicit double precision (a-h,o-z)    
      common / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot  
      common / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      common / decays / iflav(4), amdec(4) 
      save   / momset /,/ momdec /,/ decays /
      save
 11   format(1G23.16,/,3G23.16,A5)
 12   format(1G23.16,/,3G23.16,I5)
CC      write(nout,11) qeff1(4),(qeff1(k),k=1,3),'  11'  
CC      write(nout,11) qeff2(4),(qeff2(k),k=1,3),' -11'
CC      write(nout,12) p4(4),(p4(k),k=1,3),iflav(4)
CC      write(nout,12) p3(4),(p3(k),k=1,3),iflav(3)
CC      write(nout,12) p1(4),(p1(k),k=1,3),iflav(1)  
CC      write(nout,12) p2(4),(p2(k),k=1,3),iflav(2)
CC      write(nout,*)
      end

      subroutine dumpw(nout)     
*     **********************     
c Prints out four-momenta and flavors of inermediate and 
c final particles on output unit nout
c 
c Written by: Wieslaw Placzek        date: 26.07.1994
c Last update: 27.07.1994            by: W.P.  
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / momset / QEFF1(4),QEFF2(4),sphum(4),sphot(100,4),nphot 
      common / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      common / decays / iflav(4), amdec(4) 
      save   / momset /,/ momdec /,/ decays /
      DOUBLE PRECISION sum(4)
       IF (IFLAV(1).eq.-iflav(2)) then
       iflwm= 23
       iflwp= 23
      else
       iflwm=-24
       iflwp= 24       
      endif
      iflph= 22
CC      write(nout,*) '=====================dumpw===================='
CC      write(nout,3200) ' p(1)',' p(2)',' p(3)',' p(4)',' pdg-code'
CC      do 100 i=1,nphot                       
CC  100 write(nout,3100) ' PHO',(sphot(i,k),k=1,4),iflph 
CC      write(nout,3100) 'Z/W-',(q1(k),k=1,4),iflwm   
CC      write(nout,3100) 'Z/W+',(q2(k),k=1,4),iflwp   
CC      write(nout,3100) '  p1',(p1(k),k=1,4),iflav(1)   
CC      write(nout,3100) '  p2',(p2(k),k=1,4),iflav(2)
CC      write(nout,3100) '  p3',(p3(k),k=1,4),iflav(3)   
CC      write(nout,3100) '  p4',(p4(k),k=1,4),iflav(4)
      do 101 k=1,4      
 101  sum(k)=p1(k)+p2(k)+p3(k)+p4(k)         
      do 210 i=1,nphot                       
      do 210 k=1,4                           
  210 sum(k)=sum(k)+sphot(i,k)               
      isfla=iflav(1)+iflav(2)+iflav(3)+iflav(4)
CC      write(nout,3100) 'sum',(sum(k),k=1,4),isfla 
CC      write(nout,*) '=============================================='  
 3100 format(1x,a4,1x,4f15.8,i7)   
c 3100 format(1x,a4,1x,5f18.14)               
 3200 format(5x,4a15,a10)
      end   

      subroutine dumpl(nout,P1,P2,P3,P4,QEFF1,QEFF2,SPHOT,nphot)     
*     ****************************************************************     
c Prints out four-momenta and flavors of inermediate and 
c final particles on output unit nout
c 
c Written by: M.Skrzypek        date: 17.03.95
c Last update:             by:  
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / decays / iflav(4), amdec(4) 
      save   / decays /
      DOUBLE PRECISION sum(4),pho(4),sphot(100,4)     
      dimension p1(4),p2(4),p3(4),p4(4),qeff1(4),qeff2(4)

      p4mass=dmas2(p4)
      p3mass=dmas2(p3)
      p2mass=dmas2(p2)
      p1mass=dmas2(p1)
      e2mass=dmas2(qeff2)
      e1mass=dmas2(qeff1)

      p1mass=dsign(dsqrt(abs(p1mass)),p1mass)
      p2mass=dsign(dsqrt(abs(p2mass)),p2mass)
      p3mass=dsign(dsqrt(abs(p3mass)),p3mass)
      p4mass=dsign(dsqrt(abs(p4mass)),p4mass)
      e1mass=dsign(dsqrt(abs(e1mass)),e1mass)
      e2mass=dsign(dsqrt(abs(e2mass)),e2mass)


CC      write(nout,*) '=====================dumpl===================='
CC      write(nout,3200) ' p(1)',' p(2)',' p(3)',' p(4)','  sign*mass'

      do 100 i=1,nphot    
      do 110 k=1,4    
 110  pho(k)=sphot(i,k)
      amphot=dmas2(pho)
      phmass=dsign(dsqrt(abs(amphot)),amphot)
 100  continue
CC 100  write(nout,3100) 'PHO',(sphot(i,k),k=1,4),phmass
CC      write(nout,3100) ' p1',(p1(k),k=1,4),p1mass,iflav(1)  
CC      write(nout,3100) ' p2',(p2(k),k=1,4),p2mass,iflav(2)
CC      write(nout,3100) ' p3',(p3(k),k=1,4),p3mass,iflav(3)
CC      write(nout,3100) ' p4',(p4(k),k=1,4),p4mass,iflav(4)
CC      write(nout,3100) 'qf1',(qeff1(k),k=1,4),e1mass  
CC      write(nout,3100) 'qf2',(qeff2(k),k=1,4),e2mass
      do 101 k=1,4      
 101  sum(k)=p1(k)+p2(k)+p3(k)+p4(k)         
      do 210 i=1,nphot                       
      do 210 k=1,4                           
  210 sum(k)=sum(k)+sphot(i,k)               
      isfla=iflav(1)+iflav(2)+iflav(3)+iflav(4)
      e2mass=dmas2(sum)
      sumas=dsign(dsqrt(abs(e2mass)),e2mass)
CC      write(nout,3100) 'sum',(sum(k),k=1,4), sumas !,isfla 
CC      write(nout,*) '=============================================='  
 3100 format(1x,a3,5f21.15,i4)   
c 3100 format(1x,a3,1x,5f18.14)               
 3200 format(5x,4a22,a10)
      end   

      subroutine dumpb_unused(nout)     
*     **********************     
c Prints out four-momenta and flavors of inermediate and 
c final particles on output unit nout
c 
c Written by: M.Skrzypek        date: 17.03.95
c Last update:             by:  
c
      implicit DOUBLE PRECISION (a-h,o-z)
      common / momset / QEFF1(4),QEFF2(4),sphum(4),sphot(100,4),nphot
! / bormom / does not exist any more, m.s. 10/18/97
      common / bormom / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      common / decays / iflav(4), amdec(4) 
      save   / momset /,/ bormom /,/ decays /
      DOUBLE PRECISION sum(4),pho(4)     

      p4mass=dmas2(p4)
      p3mass=dmas2(p3)
      p2mass=dmas2(p2)
      p1mass=dmas2(p1)
      q2mass=dmas2(q2)
      q1mass=dmas2(q1)
      e2mass=dmas2(qeff2)
      e1mass=dmas2(qeff1)

      q1mass=dsign(dsqrt(abs(q1mass)),q1mass)
      q2mass=dsign(dsqrt(abs(q2mass)),q2mass)
      p1mass=dsign(dsqrt(abs(p1mass)),p1mass)
      p2mass=dsign(dsqrt(abs(p2mass)),p2mass)
      p3mass=dsign(dsqrt(abs(p3mass)),p3mass)
      p4mass=dsign(dsqrt(abs(p4mass)),p4mass)
      e1mass=dsign(dsqrt(abs(e1mass)),e1mass)
      e2mass=dsign(dsqrt(abs(e2mass)),e2mass)


CC      write(nout,*) '=====================dump born================'
CC      write(nout,3200) ' p(1)',' p(2)',' p(3)',' p(4)','  sign*mass'

      do 100 i=1,nphot    
      do 110 k=1,4    
 110  pho(k)=sphot(i,k)
      amphot=dmas2(pho)
      phmass=dsign(dsqrt(abs(amphot)),amphot)
 100  continue
CC  100 write(nout,3100) 'PHO',(sphot(i,k),k=1,4),phmass
CC      write(nout,3100) 'bW-',(q1(k),k=1,4),q1mass   
CC      write(nout,3100) 'bW+',(q2(k),k=1,4),q2mass 
CC      write(nout,3100) 'bp1',(p1(k),k=1,4),p1mass  
CC      write(nout,3100) 'bp2',(p2(k),k=1,4),p2mass
CC      write(nout,3100) 'bp3',(p3(k),k=1,4),p3mass   
CC      write(nout,3100) 'bp4',(p4(k),k=1,4),p4mass
CC      write(nout,3100) 'ef1',(qeff1(k),k=1,4),e1mass  
CC      write(nout,3100) 'ef2',(qeff2(k),k=1,4),e2mass
      do 101 k=1,4      
 101  sum(k)=p1(k)+p2(k)+p3(k)+p4(k)         
      do 210 i=1,nphot                       
      do 210 k=1,4                           
  210 sum(k)=sum(k)+sphot(i,k)               
      isfla=iflav(1)+iflav(2)+iflav(3)+iflav(4)
CC      write(nout,3100) 'sum',(sum(k),k=1,4) !,isfla 
CC      write(nout,*) '=============================================='  
 3100 format(1x,a3,1x,5f22.15)   
c 3100 format(1x,a3,1x,5f18.14)               
 3200 format(5x,4a22,a10)
      end   
