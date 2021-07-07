      FUNCTION ALPHANSK(Q2,RR) 
      IMPLICIT REAL*8 (A-H,O-Z)
      parameter (np=7186)
      dimension s(0:np-1)
      dimension pres(0:np-1),pre(0:np-1),pim(0:np-1)
      dimension epres(0:np-1),epre(0:np-1),epim(0:np-1),cv(0:np-1)
      double complex ALPHANSK
      double complex im
      integer RR
      common/vpcnsk/s,pres,pre,pim,epres,epre,epim,cv,im,ifirst
      data ifirst/0/
      
!       CHARACTER(LEN=200) :: datadir
!       CHARACTER(LEN=200) :: datafile
!       CHARACTER(LEN=200) :: datafilefull
      CHARACTER(LEN=300) :: datadir
      CHARACTER(LEN=300) :: datafile
      CHARACTER(LEN=300) :: datafilefull
      INTEGER :: status_value = 0

      common/rescaledhaderr/scal_err_dhad

      if (ifirst.eq.0) then
         CALL GET_ENVIRONMENT_VARIABLE("BELLE2_LOCAL_DIR", datadir, STATUS=status_value)
         datafile="/generators/teegg/data/vpol_all_bare_sum_v1.dat"
         datafilefull=trim(datadir)//trim(datafile)
         
         WRITE(*,*) "Reading polarization data: ", datafilefull         
         
         im = (0.d0, 1.d0)
         open(45, file=datafilefull, status='old')
         do k = 0, np-1
            read(45,*) s(k), pres(k), pre(k), pim(k), epres(k), epre(k), 
     .           epim(k), cv(k)
         enddo
         close(45)
         ifirst=1
      endif
      
      k   = 0
      sk  = 0.d0
      aq2 = abs(q2)
      do while(aq2.gt.s(k))
         k = k + 1         
      enddo
      k = k - 1
      
      if (q2.gt.0.d0) then
         vpre = pre(k) + (pre(k+1)-pre(k))/(s(k+1)-s(k)) * (q2-s(k))
         vpim = pim(k) + (pim(k+1)-pim(k))/(s(k+1)-s(k)) * (q2-s(k))

         evpre = epre(k) + (epre(k+1)-epre(k))/(s(k+1)-s(k)) * (q2-s(k))
         evpim = epim(k) + (epim(k+1)-epim(k))/(s(k+1)-s(k)) * (q2-s(k))

      else
         vpre = pres(k) + (pres(k+1)-pres(k))/(s(k+1)-s(k))*(aq2-s(k))
         vpim = 0.d0

         evpre = epres(k)+(epres(k+1)-epres(k))/(s(k+1)-s(k))*(aq2-s(k))
         evpim = 0.d0
         
      endif

      vpre = vpre + scal_err_dhad * evpre
      vpim = vpim + scal_err_dhad * evpim

      if(RR.eq.0)then
	ALPHANSK = 1.d0/(1.d0  - vpre - im*0.0)
      elseif(RR.eq.1)then
	ALPHANSK = 1.d0/(1.d0  - vpre - im*vpim)
      else
      	ALPHANSK = 0.0
      end if
      
      return
      end
      