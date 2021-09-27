      SUBROUTINE eexx_wt_cor(wtcrud,svar,amel,iflav,vvmin,wtcort)
*     ***********************************************************   
*===================================================================*
*             NOTE: This is a DUMMY ROUTINE !!!                     *
* The real one is still under development and can be obtained from  *
* the authors only on a special request.                            *
*===================================================================*
!-------------------------------------------------------------------!
! The t-channel radiation correction weight for eexx final states   !
! with hard high PT radiative photons.                              !
! INPUT: wtcrud - "crude" weight                                    !
!        svar  - CMS energy squared                                 !
!        amel  - electron mass                                      !
!        iflav - final state particles flavours (PDG)               !
!        vvmin   - soft photon cut-off in CMS (min: E_gamma/E_beam) !
! OUTPUT: wtcort - correction weight                                !
!-------------------------------------------------------------------!
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      COMMON / momset / qeff1(4),qeff2(4),sphum(4),sphot(100,4),nphot
      COMMON / momdec / q1(4),q2(4),p1(4),p2(4),p3(4),p4(4)
      COMMON / cms_eff_momdec /
     $      effb1(4),effb2(4),p1e(4),p2e(4),p3e(4),p4e(4)
      SAVE / momset /, / momdec /, / cms_eff_momdec /

      DIMENSION iflav(4)

*! DUMMY DUMMY DUMMY ... 

*! Total correction weight 
      wtcort = 1d0
      END
