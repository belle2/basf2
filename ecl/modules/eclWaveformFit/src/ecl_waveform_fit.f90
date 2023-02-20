
! basf2 (Belle II Analysis Software Framework)
! Author: The Belle II Collaboration
!
! See git log for contributors and copyright holders.
! This file is licensed under LGPL-3.0, see LICENSE.md.

MODULE ECL_WAVEFORM_FIT
  USE, INTRINSIC :: ISO_C_BINDING
  USE, INTRINSIC :: ISO_FORTRAN_ENV

  !> Number of data points.
  INTEGER, PARAMETER :: N_POINTS = 31

  !> Vector length.
  INTEGER, PARAMETER :: VECTOR_LENGTH = 2

  !> Number of vectors.
  INTEGER, PARAMETER :: N_VECTORS = 16

  !> Number of data points for vectorized data.
  INTEGER, PARAMETER :: N_POINTS_VECTOR = VECTOR_LENGTH * N_VECTORS

  !> Packed matrix size.
  INTEGER, PARAMETER :: PACKED_MATRIX_SIZE = N_POINTS * (N_POINTS + 1) / 2

  !> Inverse covariance matrix.
  !!
  !! This is stored in a special format in order to assist vectorization
  !! of its multiplication by a vector. The storage
  !! depends on the vector length, it is explained below for a 4x4 matrix and
  !! vector length 2. The matrix is given by
  !!
  !! W(1,1) W(1,2) W(1,3) W(1,4)
  !! W(2,1) W(2,2) W(2,3) W(2,4)
  !! W(3,1) W(3,2) W(3,3) W(3,4)
  !! W(4,1) W(4,2) W(4,3) W(4,4)
  !!
  !! First, the first two rows are stored:
  !!
  !! W(1,1) W(2,1)
  !! W(1,2) W(2,2)
  !! W(1,3) W(2,3)
  !! W(1,4) W(2,4)
  !!
  !! Then, the second two rows are stored:
  !!
  !! W(3,1) W(4,1)
  !! W(3,2) W(4,2)
  !! W(3,3) W(4,3)
  !! W(3,4) W(4,4)
  !!
  !! When multiplying by a vector, each column is multiplied by the same
  !! number:
  !!
  !! W(3,1)*X(1) W(4,1)*X(1)
  !! W(3,2)*X(2) W(4,2)*X(2)
  !! W(3,3)*X(3) W(4,3)*X(3)
  !! W(3,4)*X(4) W(4,4)*X(4)
  !!
  !! After that, all rows are summed. This can be done by SiMD instructions
  !! without any order changes.
  REAL(REAL64) INVERSE_COVARIANCE(N_POINTS_VECTOR * N_POINTS)

CONTAINS

  !> Load inverse covariance matrix from the packed form.
  !! @param[in] PACKED_MATRIX Packed matrix.
  SUBROUTINE ECL_WAVEFORM_FIT_LOAD_INVERSE_COVARIANCE(PACKED_MATRIX) BIND(C)
    USE, INTRINSIC :: ISO_C_BINDING
    IMPLICIT NONE
    REAL(C_FLOAT), INTENT(IN) :: PACKED_MATRIX(PACKED_MATRIX_SIZE)
    INTEGER I, J, PACKED_INDEX, ROW, ROW_MIN, ROW_MAX
    REAL(C_FLOAT) MATRIX(N_POINTS, N_POINTS)
    PACKED_INDEX = 1
    DO I = 1, N_POINTS
      DO J = 1, I
        MATRIX(I, J) = PACKED_MATRIX(PACKED_INDEX)
        MATRIX(J, I) = MATRIX(I, J)
        PACKED_INDEX = PACKED_INDEX + 1
      ENDDO
    ENDDO
    PACKED_INDEX = 1
    ROW_MIN = 1
    DO I = 1, N_VECTORS
      ROW_MAX = ROW_MIN + VECTOR_LENGTH - 1
      IF (ROW_MAX .GT. N_POINTS) THEN
        ROW_MAX = N_POINTS
      ENDIF
      DO J = 1, N_POINTS
        DO ROW = ROW_MIN, ROW_MAX
          INVERSE_COVARIANCE(PACKED_INDEX) = MATRIX(ROW, J)
          PACKED_INDEX = PACKED_INDEX + 1
        ENDDO
        IF (I .EQ. N_VECTORS) THEN
          DO ROW = ROW_MAX + 1, N_POINTS_VECTOR
            INVERSE_COVARIANCE(PACKED_INDEX) = 0
            PACKED_INDEX = PACKED_INDEX + 1
          ENDDO
        ENDIF
      ENDDO
      ROW_MIN = ROW_MAX + 1
    ENDDO
  END SUBROUTINE

  !> Multiply vector by the stored inverse covariance matrix.
  !! @param[out] Y Result vector.
  !! @param[in]  X Vector.
  SUBROUTINE ECL_WAVEFORM_FIT_MULTIPLY_INVERSE_COVARIANCE(Y, X) BIND(C)
    IMPLICIT NONE
    REAL(REAL64), INTENT(OUT) :: Y(N_POINTS_VECTOR)
    REAL(REAL64), INTENT(IN) :: X(N_POINTS_VECTOR)
    REAL(REAL64) V(VECTOR_LENGTH)
    INTEGER I, INDEX_COV, INDEX_Y, J, K
    INDEX_COV = 0
    INDEX_Y = 0
    DO I = 1, N_VECTORS
      !$OMP SIMD
      DO K = 1, VECTOR_LENGTH
        V(K) = INVERSE_COVARIANCE(INDEX_COV + K) * X(1)
      ENDDO
      INDEX_COV = INDEX_COV + VECTOR_LENGTH
!GCC$ unroll 31
      DO J = 2, N_POINTS
        !$OMP SIMD
        DO K = 1, VECTOR_LENGTH
          V(K) = V(K) + INVERSE_COVARIANCE(INDEX_COV + K) * X(J)
        ENDDO
        INDEX_COV = INDEX_COV + VECTOR_LENGTH
      ENDDO
      !$OMP SIMD
      DO K = 1, VECTOR_LENGTH
        Y(INDEX_Y + K) = V(K)
      ENDDO
      INDEX_Y = INDEX_Y + VECTOR_LENGTH
    ENDDO
  END SUBROUTINE

END MODULE
