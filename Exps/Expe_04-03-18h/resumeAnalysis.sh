#!/bin/bash

output=_resumes.csv

#ls | grep resume.txt
echo "Task ; ID ; Activations ; Primary exec time ; MIN ; AVG ; MAX ; Mode SW ; Context SW ; Sys Calls" > $output

for file in `ls | grep resume.txt`
do
	TASK=`more $file | grep "summary for" | awk 'NF{ printf $NF }'`
	TASK=${TASK%?}
	ID=`echo $file | awk -F_ '{print $1_$2}'`

	ACT=`more $file | grep "times" | awk '{print $5}'`
	PRIM=`more $file | grep "Primary Mode" | awk 'FNR==2' | awk '{print $6}'`
	MIN=`more $file | grep "(ms)" | awk '{print $1}'`
	AVG=`more $file | grep "(ms)" | awk '{print $3}'`
	MAX=`more $file | grep "(ms)" | awk '{print $5}'`
	ModSW=`more $file | grep "Mode Switches" | awk 'FNR==2' | awk 'NF{ printf $NF }'`
	CtxSW=`more $file | grep "Context Switches" | awk 'FNR==2' | awk 'NF{ printf $NF }'`
	SysCALL=`more $file | grep "Sys calls" | awk 'FNR==2' | awk 'NF{ printf $NF }'`

	echo "===== $TASK ====="
	echo "Activations=$ACT"
	echo "Prim=$PRIM"
	echo "Min=$MIN"
	echo "Avg=$AVG"
	echo "Max=$MAX"
	echo "Mode Switches=$ModSW"
	echo "Context Switches=$CtxSW"
	echo "Sys Calls=$SysCALL"
	echo

	echo "$TASK ; $ID ; $ACT ; $PRIM ; $MIN ; $AVG ; $MAX ; $ModSW ; $CtxSW ; $SysCALL" >> $output
done
