
      SUBROUTINE fort_open(nout,fname)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  Interface used by c++ programs                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT  NONE
      CHARACTER fname*(*)
      INTEGER   nout,nout2
*-------------------------------------------------------------------------------------
      nout2 = nout
      OPEN(nout2,file=fname)
**      WRITE(6,'(A,A20,A)')    '======>',filename,'<========='
**      WRITE(nout,'(A,A20,A)') '======>',filename,'<========='
      END

      SUBROUTINE fort_close(nout)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  Interface used by c++ programs                                                 //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER  nout,nout2
*-------------------------------------------------------------------------------------
      nout2 = nout
      CLOSE(nout2)
      END


      SUBROUTINE ReaDataX(ninp,xpar,imax)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  Single data card is:    (a1,i4,d15.0,a60)                                      //
*//  First character * defines comment card!                                        //
*//                                                                                 //
*//  Note that this program does not clear xpar!!!                                  //
*//  one has to do it before calling it, if necessary!!!                            //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      DOUBLE PRECISION xpar(*)
      INTEGER          ninp,imax
*-------------------------------------------------------------------------------------
      CHARACTER*6      beg6
      CHARACTER*4      end4
      CHARACTER*1      mark1
      CHARACTER*60     comm60
      CHARACTER*80     comm80
      INTEGER          line,index
      DOUBLE PRECISION value
*-------------------------------------------------------------------------------------

*      WRITE(  *,*) '***************************'
*      WRITE(  *,*) '*  Parser ReaDataX starts *'
*      WRITE(  *,*) '***************************'

* Search for 'BeginX'
      DO line =1,10000
         READ(ninp,'(a6,a)') beg6,comm60
         IF(beg6 .EQ. 'BeginX') THEN
            WRITE(  *,'(a6,a)') beg6,comm60
            GOTO 200
         ENDIF
      ENDDO
 200  CONTINUE

* Read data, 'EndX' terminates data, '*' marks comment
      DO line =1,1000
         READ(ninp,'(a)') mark1
         IF(mark1 .EQ. ' ') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a1,i4,d15.0,a60)') mark1,index,value,comm60
            WRITE(  *,'(a1,i4,g15.6,a60)') mark1,index,value,comm60
            IF( (index .LE. 0) .OR. (index .GE. imax)) GOTO 990
            xpar(index) = value
         ELSEIF(mark1 .EQ. 'E') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a4,a)') end4,comm60
            WRITE(  *,'(a4,a)') end4,comm60
            IF(end4 .EQ. 'EndX') GOTO 300
            GOTO 991
         ELSEIF(mark1 .EQ. '*') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a)') comm80
            WRITE(  *,'(a)') comm80
         ENDIF
      ENDDO
 300  CONTINUE

*      WRITE(  *,*) '***************************'
*      WRITE(  *,*) '* Parser ReaDataX ends    *'
*      WRITE(  *,*) '***************************'
      RETURN
 990  WRITE(*,*) '+++ ReaDataX: wrong index= ',index
      STOP
      RETURN
 991  WRITE(*,*) '+++ ReaDataX: wrong end of data '
      STOP
      END

      SUBROUTINE ReaDataN(ninp,npar,imax)
*/////////////////////////////////////////////////////////////////////////////////////
*//                                                                                 //
*//  Single data card is:    (a1,i4,i15,a60)                                        //
*//  First character * defines comment card!                                        //
*//                                                                                 //
*//  Note that this program does not clear xpar!!!                                  //
*//  one has to do it before calling it, if necessary!!!                            //
*//                                                                                 //
*/////////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
      INTEGER          ninp,imax,npar(*)
*
      CHARACTER*6      beg6
      CHARACTER*4      end4
      CHARACTER*1      mark1
      CHARACTER*60     comm60
      CHARACTER*80     comm80   
      INTEGER          nvalue,index,line
*--------------------------------------------------------------------------------------
*      WRITE(  *,*) '***************************'
*      WRITE(  *,*) '* Parser  ReaDataN starts *'
*      WRITE(  *,*) '***************************'

* Search for 'BeginN'
      DO line =1,10000
         READ(ninp,'(a6,a)') beg6,comm60
         IF(beg6 .EQ. 'BeginN') THEN
            WRITE(  *,'(a6,a)') beg6,comm60
            GOTO 200
         ENDIF
      ENDDO
 200  CONTINUE

* Read data, 'EndN' terminates data, '*' marks comment
      DO line =1,1000
         READ(ninp,'(a)') mark1
         IF(mark1 .EQ. ' ') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a1,i4,i15,a60)') mark1,index,nvalue,comm60
            WRITE(  *,'(a1,i4,i15,a60)') mark1,index,nvalue,comm60
            IF( (index .LE. 0) .OR. (index .GE. imax)) GOTO 990
            npar(index) = nvalue
         ELSEIF(mark1 .EQ. 'E') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a4,a)') end4,comm60
            WRITE(  *,'(a4,a)') end4,comm60
            IF(end4 .EQ. 'EndN') GOTO 300
            GOTO 991
         ELSEIF(mark1 .EQ. '*') THEN
            BACKSPACE(ninp)
            READ(ninp,'(a)') comm80
            WRITE(  *,'(a)') comm80
         ENDIF
      ENDDO
 300  CONTINUE

*      WRITE(  *,*) '***************************'
*      WRITE(  *,*) '* Parser ReaDataN ends    *'
*      WRITE(  *,*) '***************************'
      RETURN
 990  WRITE(*,*) '+++ ReaDataN: wrong index= ',index
      STOP
      RETURN
 991  WRITE(*,*) '+++ ReaDataN: wrong end of data '
      STOP
      END

