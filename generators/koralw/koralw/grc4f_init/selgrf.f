************************************************************************
      subroutine selgrf(ibackgr)
************************************************************************
* Overwrites GRACE initialisation according to KORALW needs
* ibackgr = 0  - doubly resonant W-pairs
* ibackgr = 1  - complete 4fermion process
************************************************************************
      implicit DOUBLE PRECISION(a-h,o-z)
      include 'incl1.f'
      common /grc4fs/ nthprc
*-----------------------------------------------------------------------
      if( ibackgr .eq. 0 ) then
          do 10 n1 = 1, ngraph
             jselg(n1) = 0
   10     continue
* Graph selection : W-pairs
          if( nthprc .eq. 27 ) then
              jselg(15) = 1
              jselg(19) = 1
              jselg(23) = 1
          elseif( nthprc .eq. 62 ) then
              jselg(57) = 1
              jselg(61) = 1
              jselg(77) = 1
          else
              print *,'not yet selgrf',nthprc
              stop
          endif
      else
* Graph selection : all graphs
          do 20 n1 = 1, ngraph
             jselg(n1) = 1
   20     continue
      endif
*-----------------------------------------------------------------------
      return
      end
