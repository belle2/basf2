      SUBROUTINE RANECU(RVEC,LEN,KSEQ)
      DIMENSION RVEC(*)
	call kkmc_ranmar(RVEC,LEN)
      END

