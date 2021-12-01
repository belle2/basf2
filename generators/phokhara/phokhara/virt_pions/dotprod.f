
      function dotrr(p1,p2)
      implicit none      
      real*8 p1(0:3),p2(0:3),dotrr
      dotrr = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - p1(3)*p2(3)
      end
      
      function dotrc(p1,p2)
      implicit none      
      real*8 p1(0:3)
      complex*16  p2(0:3),dotrc
      dotrc = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - p1(3)*p2(3)
      end

      function dotcc(p1,p2)
      implicit none      
      complex*16 dotcc,p1(0:3),p2(0:3)
      dotcc = p1(0)*p2(0) - p1(1)*p2(1) - p1(2)*p2(2) - 
     #     p1(3)*p2(3)
      end





