#!/bin/sh
#ssh -n -n -XY root@svd04 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y  install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

#ssh -n -n -XY root@svd05  " passwd yamadas;"
#ssh -n -n -XY root@svd06  " passwd yamadas;"
#ssh -n -n -XY root@cdc04  " passwd yamadas;"
#ssh -n -n -XY root@cdc06  " passwd yamadas;"
#ssh -n -n -XY root@top03  " passwd yamadas;"
#ssh -n -n -XY root@top04  " passwd yamadas;"
#ssh -n -n -XY root@arich02  " passwd yamadas;"
#ssh -n -n -XY root@ecl02  " passwd yamadas;"
#ssh -n -n -XY root@ecl03  " passwd yamadas;"
#ssh -n -n -XY root@ecl05  " passwd yamadas;"
#ssh -n -n -XY root@ecl06  " passwd yamadas;"
#ssh -n -n -XY root@klm03  " passwd yamadas;"


#ssh -n -n -XY root@svd01 "mkdir /home/usr; useradd -m -d /home/usr/yamadas -u 14252 yamadas; useradd -m -d /home/usr/b2daq -u 3001 b2daq;"
#ssh -n -n -XY root@svd03 "mkdir /home/usr; useradd -m -d /home/usr/yamadas -u 14252 yamadas; useradd -m -d /home/usr/b2daq -u 3001 b2daq;"
#ssh -n -n -XY root@svd08 "mkdir /home/usr; useradd -m -d /home/usr/yamadas -u 14252 yamadas; useradd -m -d /home/usr/b2daq -u 3001 b2daq;"
#ssh -n -n -XY root@arich01 "mkdir /home/usr; useradd -m -d /home/usr/yamadas -u 14252 yamadas; useradd -m -d /home/usr/b2daq -u 3001 b2daq;"
#ssh -n -n -XY root@ecl07 "mkdir /home/usr; useradd -m -d /home/usr/yamadas -u 14252 yamadas; useradd -m -d /home/usr/b2daq -u 3001 b2daq;"

ssh -n -n -XY root@svd01 "mkdir /home/usr/yamadas; mkdir /home/usr/b2daq; cd /home/usr; chmod a+rx *;"
ssh -n -n -XY root@svd03 "mkdir /home/usr/yamadas; mkdir /home/usr/b2daq; cd /home/usr; chmod a+rx *;"
ssh -n -n -XY root@svd08 "mkdir /home/usr/yamadas; mkdir /home/usr/b2daq; cd /home/usr; chmod a+rx *;"
ssh -n -n -XY root@arich01 "mkdir /home/usr/yamadas; mkdir /home/usr/b2daq; cd /home/usr; chmod a+rx *;"
ssh -n -n -XY root@ecl07 "mkdir /home/usr/yamadas; mkdir /home/usr/b2daq; cd /home/usr; chmod a+rx *;"
