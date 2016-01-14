#!/bin/sh
ssh -n -n -XY root@klm03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
exit
ssh -n -n -XY root@svd01 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@svd03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@svd08 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@arich01 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@ecl07 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"


exit

ssh -n -n -XY root@svd02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@svd03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@cdc01 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc05 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@cdc07 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc08 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc09 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
ssh -n -n -XY root@cdc10 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@top02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@klm02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"


exit

ssh -n -n -XY root@svd05 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on; /sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@svd06 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on; /sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@cdc04 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@cdc06 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@top03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@top04 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@arich02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@ecl02 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@ecl03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@ecl05 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@ecl06 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"

ssh -n -n -XY root@klm03 "http_proxy=http://b2stone.daqnet.kek.jp:8080/; https_proxy=http://b2stone.daqnet.kek.jp:8080/; ftp_proxy=http://b2stone.daqnet.kek.jp:8080/; export http_proxy https_proxy ftp_proxy; yum -y install emacs ; yum -y install openldap-clients ; /sbin/chkconfig ntpd on;
/sbin/chkconfig tftp on; /sbin/chkconfig nfs on; /sbin/service ntpd restart; /sbin/service xinetd restart;"
