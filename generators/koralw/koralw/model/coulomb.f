      FUNCTION culmc(s,s1,s2)
!     **************************
! Coulomb effect from Fadin, Khoze, Martin, Stirling, dtp/95/64
! first order, eq. 9

      IMPLICIT DOUBLE PRECISION (A-H,O-Z)  

      COMMON / MATPAR / pi,ceuler     
      COMMON / PHYPAR / alfinv,gpicob     
      COMMON / WEKING / ENE,AMAZ,GAMMZ,AMEL,AMFIN,XK0,SINW2,IDE,IDF 
      COMMON / WEKIN2 / AMAW,GAMMW,GMU,ALPHAW   
! This common can be everywhere, contains various switches
      COMMON / KeyKey/  KeyRad,KeyPhy,KeyTek,KeyMis,KeyDwm,KeyDwp

      KeyCul = MOD(KeyRad,10000)/1000

      if(keycul.eq.0) then
        culmc = 1d0
      elseif(keycul.eq.1) then
        pp = 1/(4*s) *( s**2 -2*s*(s1+s2) +(s1-s2)**2 )
        ppp= 1/(4*s) *((s-s1-s2)**2-4d0*s1*s2)
        if(pp.lt.0.) pp=ppp
        p  = dsqrt(pp)
        en = (s-4*amaw**2)/(4*amaw)
        ddee = dsqrt(en**2+gammw**2)
        p1 = dsqrt( amaw/2d0 *( ddee -en ) )
        p2 = dsqrt( amaw/2d0 *( ddee +en ) )
        dabskap2 = amaw *ddee
        drekap  =  p1
        dimkap  = -p2

        ff = 1 +sqrt(s)/(4*p*alfinv) 
     $           *( pi -2*datan( (dabskap2 -pp)/(2*p*drekap) ) )   

        culmc = ff -1  ! <========!!!!!!!!!
        culmc = ff
      else
        write(6,*) ' culMC==> wrong keycul=',keycul
        stop
      endif
      end

