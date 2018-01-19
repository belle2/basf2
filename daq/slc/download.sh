CUR=${PWD}
TOP=${HOME}/externals
SRC=${TOP}/src

mkdir -p ${SRC}

ncore=`cat /proc/cpuinfo | grep processor|tail -n1|awk '{print $3}'`

#if [ ! -e ${SRC}/python_src.tgz ]
#then
#
#wget -O ${SRC}/python_src.tgz 'https://www.python.org/ftp/python/3.5.2/Python-3.5.2.tgz'
#
#fi

#if [ ! -e ${SRC}/boost_src.tgz ]
#then
#
#wget -O ${SRC}/boost_src.tgz 'https://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.gz/download' --no-check-certificate
#
#fi

if [ ! -e ${SRC}/pgsql_src.tgz ]
then

wget -O ${SRC}/pgsql_src.tgz 'https://ftp.postgresql.org/pub/source/v9.3.1/postgresql-9.3.1.tar.gz'

fi



