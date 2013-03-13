*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                     Pseudo-CLASS  Vesk1                                  //
*//                                                                          //
*//   Purpose: generate one-dimensional arbitrary distribution rho(x)        //
*//   where x is in the range (0,1).                                         //
*//                                                                          //
*//                                                                          //
*//                                                                          //
*//                                                                          //
*//////////////////////////////////////////////////////////////////////////////
*
*  Class members:
*
      INTEGER     jlim1,jlim2
****  PARAMETER(  jlim1=64,    jlim2=1024)  ! usualy enough
****  PARAMETER(  jlim1=256,   jlim2=4096)  ! denser grid
      PARAMETER(  jlim1=512,   jlim2=8192)  ! even denser grid, standard
****  PARAMETER(  jlim1=1024,  jlim2=16384) ! very dense grid but slow

      DOUBLE PRECISION       xx,yy,zint,zsum
      DOUBLE PRECISION       swt,sswt
      INTEGER                nevs,jmax
      COMMON /Vesk1/
     *      xx(  jlim2+1),    ! x-grid
     *      yy(  jlim2+1),    ! f(x) values
     *      zint(jlim2+1),    ! cumulative integral from x=xx(1)=0 to x=xx(j)
     *      zsum,             ! estimated integral (zint is normalized to one)
     *      swt,              ! sum of weights
     *      sswt,             ! sum of squares of weights
     *      nevs,             ! serial counter
     *      jmax              ! grid length
      SAVE
*
*
*  Class procedures:
*
*     SUBROUTINE Vesk1_Initialize
*     SUBROUTINE Vesk1_Make
*     SUBROUTINE Vesk1_Finalize
*
*//////////////////////////////////////////////////////////////////////////////
*//                                                                          //
*//                      End of CLASS  Vesk1                                 //
*//////////////////////////////////////////////////////////////////////////////
