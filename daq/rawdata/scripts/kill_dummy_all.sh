#!/bin/sh
#
# script to run dummy data producer on remote hotst for HLT test 
# 2016.1.4 S. Yamada
# 

ssh svd01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd03 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd04 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd05 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd06 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd07 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh svd08 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

ssh cdc01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc03 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc04 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc05 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc06 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc07 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc08 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc09 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh cdc10 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

ssh top01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh top02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh top03 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh top04 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

ssh arich01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh arich02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

ssh ecl01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl03 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl04 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl05 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl06 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh ecl07 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

ssh klm01 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh klm02 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";
ssh klm03 "echo \$HOSTNAME;pgrep dummy_data_src | xargs kill -KILL";

