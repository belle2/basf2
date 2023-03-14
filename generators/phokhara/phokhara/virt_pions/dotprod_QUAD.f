
      function dotrr_QUAD(p1,p2)
      implicit none      
      real*16 p1(0:3),p2(0:3),dotrr_QUAD
      dotrr_QUAD = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - p1(3)*p2(3)
      end
      
      function dotrc_QUAD(p1,p2)
      implicit none      
      real*16 p1(0:3)
      complex*32  p2(0:3),dotrc_QUAD
      dotrc_QUAD = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - p1(3)*p2(3)
      end

      function dotcc_QUAD(p1,p2)
      implicit none      
      complex*32 dotcc_QUAD,p1(0:3),p2(0:3)
      dotcc_QUAD = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - 
     #     p1(3)*p2(3)
      end





