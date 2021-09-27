      subroutine boostv(idir,vv,pp,q)
*     *******************************
c Boost along arbitrary vector v (see eg. J.D. Jacson, "Classical 
c Electrodynamics).
c Four-vector pp is boosted from an actual frame to the rest frame 
c of the four-vector v (for idir=1) or back (for idir=-1). 
c q is a resulting four-vector.
c Note: v must be time-like, pp may be arbitrary.
c
c Written by: Wieslaw Placzek            date: 22.07.1994
c Last update: 3/29/95                     by: M.S.
c 
      implicit DOUBLE PRECISION (a-h,o-z)
      parameter (nout=6)
      DOUBLE PRECISION v(4),p(4),q(4),pp(4),vv(4)  
      save
!
      do 1 i=1,4
      v(i)=vv(i)
 1    p(i)=pp(i)
      amv=(v(4)**2-v(1)**2-v(2)**2-v(3)**2)
      if (amv.le.0d0) then
        write(6,*) 'bosstv: warning amv**2=',amv
      endif
      amv=sqrt(abs(amv))
      if (idir.eq.-1) then
        q(4)=( p(1)*v(1)+p(2)*v(2)+p(3)*v(3)+p(4)*v(4))/amv
        wsp =(q(4)+p(4))/(v(4)+amv)
      elseif (idir.eq.1) then
        q(4)=(-p(1)*v(1)-p(2)*v(2)-p(3)*v(3)+p(4)*v(4))/amv
        wsp =-(q(4)+p(4))/(v(4)+amv)
      else
        write(nout,*)' >>> boostv: wrong value of idir = ',idir
      endif
      q(1)=p(1)+wsp*v(1)
      q(2)=p(2)+wsp*v(2)
      q(3)=p(3)+wsp*v(3)
      end
             
 
      subroutine rotatv(mode,qq,pp,r)        
c     *******************************        
c rotation along arbitrary axis.
c pp rotated into r  from actual frame to frame with z-axis along qq  
c NOT TRUE 7/8/96 ms: (mode = 1) or back (mode = -1).      
c     TRUE 7/8/96 ms: (mode = -1) or back (mode = 1).      
c Written by: M. Skrzypek           date: 04.1995
c Last update: 7/9/96               by: M.S.   
      implicit double precision (a-h,o-z) 
      dimension qq(4),pp(4),r(4),tt(4)   
      parameter (pi = 3.1415926535897932D0)
      save
      the= asin(qq(1)/sqrt(qq(1)**2+qq(2)**2+qq(3)**2))
      phi= asin(qq(2)/sqrt(qq(2)**2+qq(3)**2))   
! ms 7/8/96, following line was missing (plus definition of PI)
      if(qq(3).lt.0d0) phi=pi-phi
      if(mode.eq.-1)then
        call rxtod1(phi,pp,tt)
        call rxtod2(-the,tt,r)
      elseif(mode.eq. 1)then
        call rxtod2(the,pp,tt)
        call rxtod1(-phi,tt,r)
      else
        write(6,*)'rotatv==> wrong mode:',mode
      endif
      end
 
      FUNCTION ANGLE(P,Q)
*     ******************
*  ANGLE BETWEEN TWO 3-COMPONENTS OF FOUR-VECTORS
*     ******************
      IMPLICIT DOUBLE PRECISION(A-H,O-Z)
      DIMENSION P(4),Q(4)
      PQ=P(3)*Q(3)+P(2)*Q(2)+P(1)*Q(1)
      PP=P(3)*P(3)+P(2)*P(2)+P(1)*P(1)
      QQ=Q(3)*Q(3)+Q(2)*Q(2)+Q(1)*Q(1)
      ARG=PQ/SQRT(PP*QQ)
      IF(ARG.GT. 1D0) ARG= 1D0
      IF(ARG.LT.-1D0) ARG=-1D0
      ANGLE=ACOS(ARG)
      END

      function dmas2(p)
c     *******************
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION p(4)
      d3m = dsqrt(p(3)**2 +p(2)**2 +p(1)**2)  
      dmas2= (p(4)-d3m)*(p(4)+d3m)
      end

      function dot(p,q)
c     *******************
      implicit DOUBLE PRECISION (a-h,o-z)
      DOUBLE PRECISION p(4),q(4)  
      dot= p(4)*q(4) -p(3)*q(3) -p(2)*q(2) -p(1)*q(1)
      end

      FUNCTION dd2(x,y)
!     *********************************
      implicit double precision (a-h,o-z) 
      dimension x(4),y(4)   
      dd2 = (x(4)+y(4))**2 -(x(3)+y(3))**2 -(x(2)+y(2))**2
     %     -(x(1)+y(1))**2 
      end

