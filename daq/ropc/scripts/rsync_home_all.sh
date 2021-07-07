#!/bin/sh
ssh -n -n -XY root@svd04 "rsync -avtpog rsync://172.22.19.41/export/home/usr/b2daq /home/usr;rsync -avtpog rsync://172.22.19.41/export/home/usr/yamadas /home/usr"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@svd05 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@svd06 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@cdc04 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@cdc06 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@top03 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@top04 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@arich02 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl02 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl03 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl05 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl06 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@klm03 "rsync -avtpog rsync://172.22.19.41/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;

