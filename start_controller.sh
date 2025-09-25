# Author: Pavlo Nykolyn
# starts either one or multiple (sequential) invocations of the controller
# the following environment variables are expected to be defined:
# 1) RELAY_ARRAY_CONFIGURATION
#    the format of the value shall be:
#    <ip-address>;[<port>];<model>
# 2) IDS
#    the format of the value shall be:
#    <id>{ <id>}
#    each identifier shall belong to the interval [1, 8]
# this scripts expects the existence of a logs sub-directory within the current
# working directory
logfile="./logs/relay_controller.txt"

myLogger ()
{
   # the first positional argument should be a prefix, indicating the message type
   # the second positional argument is the message itself
   echo "{$(date +'%Y-%m-%d %H:%M:%S')} ${1} ${2}"
} >> "${logfile}"

parse_res="$(echo "${RELAY_ARRAY_CONFIGURATION}" | awk -v 'FS=;' -f parse_wRCtrl.awk)"
if [ -n "${parse_res}" ]
then
   err=$(echo "${parse_res}" | tr --delete '[[:space:]]' | cut '--delimiter=:' '--fields=2')
   msg=
   case "${err}"
   in
      1) msg='only three arguments shall be supplied' ;;
      2) msg='the format of the IPv4 address is not correct' ;;
      3) msg='either the format or the value of the port is not correct' ;;
      4) msg='the model of the web relay controller is not supported' ;;
   esac
   myLogger '[ERR]' "${msg}"
   exit 1
fi

ip_address=$(echo "${RELAY_ARRAY_CONFIGURATION}" | cut '--delimiter=;' '--fields=1')
port=$(echo "${RELAY_ARRAY_CONFIGURATION}" | cut '--delimiter=;' '--fields=2')
model=$(echo "${RELAY_ARRAY_CONFIGURATION}" | cut '--delimiter=;' '--fields=3')
# creating the configuration option list
option_list="--behaviour=single --ipv4=${ip_address} --model=${model}"
if [ "${model}" = 'NC800' ]
then
   option_list="${option_list} --port=${port}"
fi

for id in ${IDS}
do
   parse_id=$(echo "${id}" | sed -n '/^[1-8]$/p')
   if [ -z "${parse_id}" ]
   then
      myLogger '[ERR]' "${id} does not belong to the interval [1, 8]"
      exit 1
   fi
   myLogger '[INF]' "turning on relay ${id} of ${ip_address} ..."
   ./bin/wRCtrl ${option_list} "--mnemonic-code=t_on_${id}" >> "${logfile}"
   sleep 10s
   myLogger '[INF]' "turning off relay ${id} of ${ip_address} ..."
   ./bin/wRCtrl ${option_list} "--mnemonic-code=t_off_${id}" >> "${logfile}"
done
