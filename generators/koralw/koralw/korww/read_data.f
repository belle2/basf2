      SUBROUTINE open_data(disk_file,delimiter,io_number)
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//   DiskFile  = input file to read                                           //
*//                                                                            //
*//   First data card: Begin://delimiter                                       //
*//   Last  data card: End://delimiter                                         //
*//   First character * defines comment card!                                  //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      CHARACTER*250 disk_file
      CHARACTER*50 delimiter
      CHARACTER*250 line
*
      INTEGER io_number,ninp,ll
*------
! here we open file for reading, if needed this line can be
! commented out and all input taken from one concatenated file 'xxx', 
      ninp = 13
      OPEN(ninp,file=disk_file,status='old')
!cccc      OPEN(ninp,file='xxx',status='old')
! here we open .... END
      REWIND(ninp)
* Search for 'Begin'
      DO ll =1,1000000
         READ(ninp,'(A)',end=201) line
         IF(line .EQ. 'Begin:'//delimiter) THEN
            GOTO 200
         ENDIF
      ENDDO

 201  CONTINUE
      WRITE(6,*)'open_data=> Begin:',delimiter,' not found, STOP'
      STOP

 200  CONTINUE
      io_number=ninp
      END


      SUBROUTINE close_data(disk_file,delimiter,io_number)
*////////////////////////////////////////////////////////////////////////////////
*//                                                                            //
*//   DiskFile  = input file to read                                           //
*//                                                                            //
*//   First data card: Begin://delimiter                                       //
*//   Last  data card: End://delimiter                                         //
*//   First character * defines comment card!                                  //
*//                                                                            //
*////////////////////////////////////////////////////////////////////////////////
      IMPLICIT NONE
*
      CHARACTER*250 disk_file
      CHARACTER*50 delimiter
      CHARACTER*250 line

      INTEGER io_number,ninp,ll
*------
      ninp = io_number
* Check for 'End'
      DO ll =1,1000000
         READ(ninp,'(a)',end=201) line
         IF(line .EQ. 'End:'//delimiter) THEN
            GOTO 200
         ENDIF
      ENDDO

 201  CONTINUE
      WRITE(6,*)'close_data=> End:',delimiter,' not found, STOP'
      STOP

 200  CONTINUE
      CLOSE(ninp)
      END




