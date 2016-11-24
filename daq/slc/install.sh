CUR=${PWD}
B2SLC=${PWD}
SLC=${HOME}/slc
TOP=${SLC}/externals
SRC=${TOP}/src

mkdir -p ${SRC}
cd ${SLC}
ln -sf ${B2SLC}/base
ln -sf ${B2SLC}/system
ln -sf ${B2SLC}/nsm
ln -sf ${B2SLC}/copper
ln -sf ${B2SLC}/runcontrol
ln -sf ${B2SLC}/database
ln -sf ${B2SLC}/psql
ln -sf ${B2SLC}/readout
ln -sf ${B2SLC}/dqm
ln -sf ${B2SLC}/hvcontrol
ln -sf ${B2SLC}/apps
ln -sf ${B2SLC}/tools
cp ${B2SLC}/Makefile ${B2SLC}/Makefile.src ${B2SLC}/setenv.sh .
cp -r ${B2SLC}/extra .
cp -r ${B2SLC}/data .

if [ ! -e ${TOP}/pgsql ]
then

mkdir -p ${TOP}/pgsql
cd ${SRC}
wget https://ftp.postgresql.org/pub/source/v9.3.1/postgresql-9.3.1.tar.gz
tar zxf postgresql-9.3.1.tar.gz
cd ${SRC}/postgresql-9.3.1
./configure --prefix=${TOP}/pgsql
make
make install
cd ${TOP}

fi

cd ${SLC}
source setenv.sh
make
cd ${CUR}

