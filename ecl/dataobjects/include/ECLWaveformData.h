/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guglielmo De Nardo                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLWAVEFORMDATA_H
#define ECLWAVEFORMDATA_H

#include <cassert>
#include <vector>
#include <TObject.h>

namespace Belle2 {

  /** ECLWaveformData - container for inverse covariant matrix and shape parameters for time and amplitude fit of ecl crystal waveform
   *
   * Objects of this class keep the independent matrix elements
   * of the inverse covariant matrix to perform waveform fit and
   * the parameters defining the waveform shape.
   */

  class ECLWaveformData : public TObject {
  public:

    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < c_nElements);
      m_matrixElement[i] = value;
    }
    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < c_nElements);
      return m_matrixElement[i];
    }

    /** Getter method for all matrix as one dimentional array */

    void getArray(float WF[136]) const  {for (int i = 0; i < 136; i++) { WF[i] = (float) m_matrixElement[i];}   }

    /** Getter method for all matrix as two dimentional array */

    void getMatrix(float M[16][16]) const
    {
      int ar16x16[16][16] = {
        { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120},
        { 1, 2, 4, 7, 11, 16, 22, 29, 37, 46, 56, 67, 79, 92, 106, 121},
        { 3, 4, 5, 8, 12, 17, 23, 30, 38, 47, 57, 68, 80, 93, 107, 122},
        { 6, 7, 8, 9, 13, 18, 24, 31, 39, 48, 58, 69, 81, 94, 108, 123},
        { 10, 11, 12, 13, 14, 19, 25, 32, 40, 49, 59, 70, 82, 95, 109, 124},
        { 15, 16, 17, 18, 19, 20, 26, 33, 41, 50, 60, 71, 83, 96, 110, 125},
        { 21, 22, 23, 24, 25, 26, 27, 34, 42, 51, 61, 72, 84, 97, 111, 126},
        { 28, 29, 30, 31, 32, 33, 34, 35, 43, 52, 62, 73, 85, 98, 112, 127},
        { 36, 37, 38, 39, 40, 41, 42, 43, 44, 53, 63, 74, 86, 99, 113, 128},
        { 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 64, 75, 87, 100, 114, 129},
        { 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 76, 88, 101, 115, 130},
        { 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 89, 102, 116, 131},
        { 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 103, 117, 132},
        { 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 118, 133},
        { 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 134},
        { 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135},
      };

      int l;

      for (int i = 0; i < 16; i++) {

        for (int j = 0; j < 16; j++) {


          l = ar16x16[i][j];
          M[i][j] = (float) m_matrixElement[l];

        }
      }

    }




    /** Setter method for waveform shape parameter */

    void setWaveformPar(size_t i, float value)
    {
      assert(i < c_nParams);
      m_waveformPar[i] = value;
    }


    /** Getter method for waveform shape parameter */
    float getWaveformPar(size_t i) const
    {
      assert(i < c_nParams);
      return m_waveformPar[i];
    }

    /** Getter method for waveform shape parameters as one dimentional array */
    void getWaveformParArray(float P[10]) const
    {
      for (int i = 0; i < 10; i++)
        P[i] = (float) m_waveformPar[i];
    }


    /** print-out function for debugging purpose*/

    void print() const;


    static const size_t c_nElements = 136; /** number of independent matrix elements*/
    static const size_t c_nParams = 10; /** number of parameters defining the waveform shape*/
  private:
    Float_t m_matrixElement[c_nElements];
    Float_t m_waveformPar[c_nParams];

    ClassDef(ECLWaveformData, 1);
  };

  /** Container for constant parameters used in waveform fits */

  class ECLWFAlgoParams : public TObject {
  public:

    Int_t ka;               /** multipliers power of 2 for fg31,fg41 */
    Int_t kb;               /** multipliers power of 2 for fg32 */
    Int_t kc;               /** multipliers power of 2 for fg33,fg43 */
    Int_t y0Startr;         /** start point for pedestal calculation */
    Int_t chiThresh;        /** chi2 threshold for quality bit */
    Int_t k1Chi;            /** multipliers power of 2 for f */
    Int_t k2Chi;            /** multipliers power of 2 for chi2 calculation */
    Int_t hitThresh;        /** hardware threshold(to start digitization) */
    Int_t lowAmpThresh;     /** threshold to calculate time */
    Int_t skipThresh;       /** threshold to send data to collector */
    int getka() const {return (int) ka;}
    int getkb() const {return (int) kb;}
    int getkc() const {return (int) kc;}
    int gety0s()  const {return (int) y0Startr;}
    int getcT() const {return (int) chiThresh;}
    int getk1() const {return (int) k1Chi;}
    int getk2() const {return (int) k2Chi;}
    int gethT() const {return (int) hitThresh;}
    int getlAT() const {return (int) lowAmpThresh;}
    int getsT()  const {return (int) skipThresh;}

    /*
        void getka(int x ){ x=(int) ka;}
        void getkb(int x ){ x=(int) kb;}
        void getkc(int x ){ x=(int) kc;}
        void gety0s(int x ){ x=(int) y0Startr;}
        void getcT(int x){ x=(int) chiThresh;}
        void getk1(int x){ x=(int) k1Chi;}
        void getk2(int x){ x=(int) k2Chi;}
        void gethT(int x){ x=(int) hitThresh;}
        void gethlAT(int x){ x=(int) lowAmpThresh;}
        void getsT(int x){ x=(int) skipThresh;}
    */
    ClassDef(ECLWFAlgoParams, 1)
  };

  /** Container for constant matrix used to generate electronic noise */
  class ECLNoiseData : public TObject {
  public:
    /** Setter method for independent matrix element */
    void setMatrixElement(size_t i, float value)
    {
      assert(i < c_nElements);
      m_matrixElement[i] = value;
    }
    /** Getter method for independent matrix element */
    float getMatrixElement(size_t i) const
    {
      assert(i < c_nElements);
      return m_matrixElement[i];
    }

    /** Getter method for matrix as one dimentional array */

    void getArray(float NoiseData[496]) const  {for (int i = 0; i < 496; i++) { NoiseData[i] = (float) m_matrixElement[i];}   }

    /** Getter method for matrix as two dimentional array */

    void getMatrix(float M[31][31]) const
    {
      int ar31x31[31][31] = {
        { 0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 325, 351, 378, 406, 435, 465},
        { 1, 2, 4, 7, 11, 16, 22, 29, 37, 46, 56, 67, 79, 92, 106, 121, 137, 154, 172, 191, 211, 232, 254, 277, 301, 326, 352, 379, 407, 436, 466},
        { 3, 4, 5, 8, 12, 17, 23, 30, 38, 47, 57, 68, 80, 93, 107, 122, 138, 155, 173, 192, 212, 233, 255, 278, 302, 327, 353, 380, 408, 437, 467},
        { 6, 7, 8, 9, 13, 18, 24, 31, 39, 48, 58, 69, 81, 94, 108, 123, 139, 156, 174, 193, 213, 234, 256, 279, 303, 328, 354, 381, 409, 438, 468},
        { 10, 11, 12, 13, 14, 19, 25, 32, 40, 49, 59, 70, 82, 95, 109, 124, 140, 157, 175, 194, 214, 235, 257, 280, 304, 329, 355, 382, 410, 439, 469},
        { 15, 16, 17, 18, 19, 20, 26, 33, 41, 50, 60, 71, 83, 96, 110, 125, 141, 158, 176, 195, 215, 236, 258, 281, 305, 330, 356, 383, 411, 440, 470},
        { 21, 22, 23, 24, 25, 26, 27, 34, 42, 51, 61, 72, 84, 97, 111, 126, 142, 159, 177, 196, 216, 237, 259, 282, 306, 331, 357, 384, 412, 441, 471},
        { 28, 29, 30, 31, 32, 33, 34, 35, 43, 52, 62, 73, 85, 98, 112, 127, 143, 160, 178, 197, 217, 238, 260, 283, 307, 332, 358, 385, 413, 442, 472},
        { 36, 37, 38, 39, 40, 41, 42, 43, 44, 53, 63, 74, 86, 99, 113, 128, 144, 161, 179, 198, 218, 239, 261, 284, 308, 333, 359, 386, 414, 443, 473},
        { 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 64, 75, 87, 100, 114, 129, 145, 162, 180, 199, 219, 240, 262, 285, 309, 334, 360, 387, 415, 444, 474},
        { 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 76, 88, 101, 115, 130, 146, 163, 181, 200, 220, 241, 263, 286, 310, 335, 361, 388, 416, 445, 475},
        { 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 89, 102, 116, 131, 147, 164, 182, 201, 221, 242, 264, 287, 311, 336, 362, 389, 417, 446, 476},
        { 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 103, 117, 132, 148, 165, 183, 202, 222, 243, 265, 288, 312, 337, 363, 390, 418, 447, 477},
        { 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 118, 133, 149, 166, 184, 203, 223, 244, 266, 289, 313, 338, 364, 391, 419, 448, 478},
        { 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 134, 150, 167, 185, 204, 224, 245, 267, 290, 314, 339, 365, 392, 420, 449, 479},
        { 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 151, 168, 186, 205, 225, 246, 268, 291, 315, 340, 366, 393, 421, 450, 480},
        { 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 169, 187, 206, 226, 247, 269, 292, 316, 341, 367, 394, 422, 451, 481},
        { 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 188, 207, 227, 248, 270, 293, 317, 342, 368, 395, 423, 452, 482},
        { 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 208, 228, 249, 271, 294, 318, 343, 369, 396, 424, 453, 483},
        { 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 229, 250, 272, 295, 319, 344, 370, 397, 425, 454, 484},
        { 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 251, 273, 296, 320, 345, 371, 398, 426, 455, 485},
        { 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 274, 297, 321, 346, 372, 399, 427, 456, 486},
        { 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 298, 322, 347, 373, 400, 428, 457, 487},
        { 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 323, 348, 374, 401, 429, 458, 488},
        { 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 349, 375, 402, 430, 459, 489},
        { 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 350, 376, 403, 431, 460, 490},
        { 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376, 377, 404, 432, 461, 491},
        { 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392, 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404, 405, 433, 462, 492},
        { 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432, 433, 434, 463, 493},
        { 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460, 461, 462, 463, 464, 494},
        { 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488, 489, 490, 491, 492, 493, 494, 495},
      };


      for (int i = 0; i < 31; i++) {

        for (int j = 0; j < 31; j++) {

          int l;
          l = ar31x31[i][j];
          if (j > i) {M[i][j] = 0.;}
          else {M[i][j] = (float) m_matrixElement[l];}

        }
      }

    }
    static const size_t c_nElements = 496; /** number of independent elements */

    Float_t m_matrixElement[c_nElements]; /** electronic noise matrix */

    ClassDef(ECLNoiseData, 1)
  };


  class ECLLookupTable : public TObject {
  public:
    ECLLookupTable() : m_content(8736, 0) {}

    int operator[](int key) const
    { return m_content[ key ]; }
    int& operator[](int key)
    { return m_content[ key ]; }

  private:
    std::vector<int> m_content; /** index to index lookup table */
    ClassDef(ECLLookupTable, 1);
  };

}
#endif
