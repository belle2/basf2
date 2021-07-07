#!/bin/sh
ssh -n -n -XY root@svd04 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@svd05 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@svd06 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@cdc04 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@cdc06 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@top03 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@top04 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@arich02 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl02 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl03 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl05 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@ecl06 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;
ssh -n -n -XY root@klm03 "rsync -a rsync://172.22.19.61/export/tftpboot /"
echo "HIT ANY KEY"; read TEMP;

