* File smcpol.f
* 
*          Fortran source code generator
* 
************************************************************************
      subroutine smcpol(la, lvt, avt)
      implicit DOUBLE PRECISION(a-h,o-z)
      parameter (ltsize = 20, lasize = 1024 )
**    include 'incl1.f'
**    include 'inclk.f'
**    integer    lvt(0:nextn)
      integer    lvt(0:ltsize)
**    DOUBLE COMPLEX avt(0:lag-1)
      DOUBLE COMPLEX avt(0:lasize)
*
**    integer j(nextn), jv(nextn)
      integer j(ltsize), jv(ltsize)
      DOUBLE COMPLEX zi, zr, ztv, ztw
      data ifst/0/
      save ifst, zr, zi
*-----------------------------------------------------------------------
      if(ifst.eq.0) then
        ifst = 1
        r2 = 1.0d0/sqrt(2.0d0)
        zr = dcmplx(1.0d0,0.0d0)*r2
        zi = dcmplx(0.0d0,1.0d0)*r2
      endif

      nextn = lvt(0)
      print *,'smcpol:nextn',nextn
      ja = la
*     if(lt(ja) .ne. 2) then
*       write(*,*) 'smcpol: cannot calculate circlar polarization'
*       write(*,*) 'lt = ', lt
*       write(*,*) 'lt(',ja,') = ', lt(ja), ' <> 2'
*       stop
*     endif
      if(lvt(ja) .ne. 2) then
        write(*,*) 'smcpol: cannot calculate circlar polarization'
        write(*,*) 'lt = ', lvt
        write(*,*) 'lt(',ja,') = ', lvt(ja), ' <> 2'
        stop
      endif

      ibas  = 1
      do 10 i = 1, nextn
        jv(i) = ibas
        ibas  = ibas*lvt(i)
        j(i)  = 0
   10 continue

  100 continue
        iv = 0
        do 110 i = 1, nextn
          iv = iv +   jv(i)*j(i)
  110   continue
        iw = iv + jv(ja)

        ztv =   zr*avt(iv) + zi*avt(iw)
        ztw =   zr*avt(iv) - zi*avt(iw)
        avt(iv) = ztv
        avt(iw) = ztw

c       write(*,*) 'smcpol:', j
c       write(*,*) 'smcpol:', iv, iw, avt(iv)
c       write(*,*) 'smcpol:', iw, iv, avt(iw)

        ii = 1
  120   continue
          if(ii .eq. ja) then
            ii = ii + 1
            if(ii.gt.nextn) then
              goto 190
            endif
          endif
          j(ii) = j(ii) + 1
          if(j(ii).ge.lvt(ii)) then
            j(ii) = 0
            ii = ii + 1
            if(ii.le.nextn) then
              goto 120
            else
              goto 190
            endif
          endif
        goto 100
  190 continue
      return
      end
