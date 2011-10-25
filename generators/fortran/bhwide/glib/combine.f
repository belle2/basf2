      PROGRAM MAIN
C     ***********************************      
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      common / cglib / b(50000)
      COMMON / INOUT  / NINP,NOUT  
      DIMENSION  X(400),ER(100)
      CHARACTER*20 typop,file1,file2,file3,stop,merge
      DATA stop  /'stop'/
      DATA merge /'merge'/
      SAVE

      write(6,*) '>>>---------------------------------------<<<'
      write(6,*) '>>>        Welcome to   COMBINE           <<<'
      write(6,*) '>>>  Program for adding histogram files   <<<'
      write(6,*) '>>>---------------------------------------<<<'
      CALL GLIMIT(50000)                 
      NINP=  5           
      NOUT= 16           
      OPEN( NOUT, file='combine.out') 
      CALL GOUTPU(NOUT)

! Get target file name
      write(6,*) '>>> Give name of the TARGET file'
      read(5,'(a)') file3
      write(6,'(a)') file3
! Get type of operation
      write(6,*) '>>> add or merge?'
      read(5,'(a)') typop
      write(6,*) typop
! Get total number of histos
      write(6,*) '>>> Give total number of histos'
      read(5,*) Ntot
      write(6,*) 'Ntot=',Ntot
!
! Restore first histogram
      write(6,*) '>>> Give name of the FIRST histogram file on the disk'
      read(5,'(a)') file1
      write(6,'(a)') file1
      NINPH=0 
!*******************************************
      OPEN(10+NINPH,file=file1)
!*******************************************
      write(6,*) '>>> restoring:: ', file1
      CALL GRFILE(10+NINPH,' ',' ')
      CALL GRIN(   0,9999,0)

 600  CONTINUE
! Restore second histogram and ADD to first            
      write(6,*) '>>> Give name of the NEXT histogram or type stop'
      read(5,'(a)') file2
      write(6,'(a)') file2
      NINPH=NINPH+1
      IF(file2 .EQ. stop) GOTO 900
      IF(NINPH .EQ. Ntot) GOTO 900
!*******************************************
      OPEN(10+NINPH,file=file2)
!*******************************************
      CALL GRFILE(10+NINPH,' ',' ')
      IF(typop .EQ. merge) THEN
! Identical histos APPEND with id=>id+1000000
      write(6,*) '>>> appending::    ', file2 
         CALL GRIN(   0,9999,0)
      ELSE
! Identical histos ADD directly
      write(6,*) '>>> adding::    ', file2
         CALL GRIN2(   0,9999,0)
      ENDIF
      GOTO 600
         
C ------------dumping histogram  -------------------------------- 
 900  CONTINUE
      write(6,*) '>>> Dumping result into::  ',file3
      NOUTH=7   
!*******************************************
      OPEN(NOUTH,file=file3)
      REWIND(NOUTH)
!*******************************************
      CALL GRFILE(NOUTH,' ','N') 
      CALL GROUT( 0,ICY,' ')       
      CALL GREND(' ')   
C ------------THE END OF HISTO WRITING -------------------------       
      CLOSE(NOUT)              
C     ***********          
      END
