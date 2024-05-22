/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#pragma once

#include <stdio.h>

#include <framework/logging/Logger.h>


namespace Belle2 {

  /**
   * kinkVertexFitterKFitError is a container of error codes and a macro function.
   */
  class kinkVertexFitterKFitError {
  public:
    /**
     * ECode is a error code enumerate.
     */
    enum ECode {
      /** No error */
      kNoError                 =  0,

      /** Not fitted yet */
      kNotFittedYet            =  1,

      /** Specified track-id out of range */
      kOutOfRange              = 11,
      /** Unprepared */
      kUnimplemented           = 12,

      /** Track count too small to perform fit */
      kBadTrackSize            = 21,
      /** Wrong correlation matrix size */
      kBadMatrixSize           = 22,
      /** Wrong correlation matrix size (internal error) */
      kBadCorrelationSize      = 23,
      /** Bad initial chi-square (internal error) */
      kBadInitialCHIsq         = 24,

      /** Division by zero (bad track property or internal error) */
      kDivisionByZero          = 31,
      /** Cannot calculate matrix inverse (bad track property or internal error) */
      kCannotGetMatrixInverse  = 32,
      /** Cannot get arcsin (bad track property or internal error) */
      kCannotGetARCSIN         = 33,
    };


  public:
    /**
     * Display a description of error and its location.
     * @param file file name where the error takes place
     * @param line line number in the file
     * @param func function name in the file
     * @param code error code
     */
    static void displayError(const char* file, const int line, const char* func, const enum ECode code)
    {

      std::string str_error = "";
      switch (code) {
        case kNoError:
          str_error = "No error";
          break;
        case kNotFittedYet:
          str_error = "Not fitted yet";
          break;
        case kOutOfRange:
          str_error = "Out of range";
          break;
        case kBadTrackSize:
          str_error = "Bad track count";
          break;
        case kBadMatrixSize:
          str_error = "Bad matrix size";
          break;
        case kBadCorrelationSize:
          str_error = "Bad correlation size";
          break;
        case kBadInitialCHIsq:
          str_error = "Bad initial chisq";
          break;
        case kDivisionByZero:
          str_error = "Division by zero";
          break;
        case kCannotGetMatrixInverse:
          str_error = "Cannot calculate matrix inverse";
          break;
        case kCannotGetARCSIN:
          str_error = "Cannot calculate arcsin";
          break;
        default:
          str_error = "Unknown error";
      }

      B2WARNING(std::string(file) << ":" << line << ":" << std::string(func) << "(): " << str_error);
    }
  };

} // namespace Belle2


