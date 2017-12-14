function echo_config()
{
echo "nodename                : ${1}"
echo "config                  : RC:cosmic:0001"
echo "eb0.port                : 5101"
echo "eb0.executable          : /home/usr/b2daq/eb/eb0"
echo "eb0.xinetd              : false"
}

function echo_cpr()
{
port=$((${1} + 34001))
echo "stream0[${1}].used         : true"
echo "stream0[${1}].script       : daq/rawdata/examples/RecvStream2.py"
echo "stream0[${1}].used         : true"
echo "stream0[${1}].host         : ${2}"
echo "stream0[${1}].port         : ${port}"
}

index=0
echo_config ROPC501 > ropc501.conf
for i in $(seq 5001 5013)
do
  echo_cpr ${index} CPR${i} >> ropc501.conf
  index=$(($index + 1))
done
daqdbcreate ropc501.conf ecl

index=0
echo_config ROPC504 > ropc504.conf
for i in $(seq 5014 5018)
do
  echo_cpr ${index} CPR${i} >> ropc504.conf
  index=$(($index + 1))
done
daqdbcreate ropc504.conf ecl

