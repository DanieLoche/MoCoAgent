#!/bin/bash

function resumeFolder {
    FOLDER=$1
    VERBOSE=$2
    
    sudo chown danlo:danlo $FOLDER
    cd $FOLDER 
    sudo chown danlo:danlo *
    
    output=${FOLDER%%+(/)}"_RESUMES.csv"
    
    #ls | grep resume.txt
    echo "Task ; ID ; Logged ; Missed ; P. exec time ; MIN ; AVG ; MAX ; Mode SW ; Context SW ; Sys Calls" > $output

    for file in `ls | grep resume.txt`
    do
        fileExpe=${file//resume.txt/expe.csv}
        TASK=`more $file | grep "summary for task" | awk '{print $5}'`
        #awk 'NF{ printf $NF }'`
	    TASK=${TASK%?}
	    
        if [ ! -z "$TASK" ] # check if TASK is empty
        then
	        ID=`echo $file | awk -F_ '{print $1"_"$2}'`

	        ACT=`more $file | grep "task times" | awk '{print $5}'`
	        MISS=`more $file | grep "task times" | awk '{print $1}'`
	        PRIM=`more $file | grep "Primary Mode" | awk 'FNR==2' | awk '{print $6}'`
	        MIN=`more $file | grep "task profile" | awk '{print $1}'`
	        AVG=`cat $fileExpe | awk -F';' '{ total+=$8; count++ } END {printf "%.6f", total/(count*1e6) }'`
	        MAX=`more $file | grep "task profile" | awk '{print $5}'`
	        ModSW=`more $file | grep "Mode Switches" | awk 'FNR==2' | awk 'NF{ printf $NF }'`
	        CtxSW=`more $file | grep "Context Switches" | awk 'FNR==2' | awk 'NF{ printf $NF }'`
	        SysCALL=`more $file | grep "Sys calls" | awk 'FNR==2' | awk 'NF{ printf $NF }'`
	    
	        if [ ! -z "$VERBOSE" ] 
	        then
	            echo "===== $TASK ====="
	            echo "Activations=$ACT"
	            echo "Miss=$MISS"
	            echo "Prim=$PRIM"
	            echo "Min=$MIN"
	            echo "Avg=$AVG"
	            echo "Max=$MAX"
	            echo "Mode Switches=$ModSW"
	            echo "Context Switches=$CtxSW"
	            echo "Sys Calls=$SysCALL"
	            echo
	        fi

	        echo "$TASK ; $ID ; $ACT ; $MISS ; $PRIM ; $MIN ; $AVG ; $MAX ; $ModSW ; $CtxSW ; $SysCALL" >> $output
        fi
    done

    echo "Resume Generated for expe $1."
    echo "Resume file is: $output"
    cd ..
}

shopt -s extglob
while [ "$1" != "" ]; do
    
    if [ ! -d $1 ]
    then 
        if [ "$1" == "-v" ] ; then
            verbose="TRUE"
            echo "==============="
            echo "VERBOSE ENABLED"
            echo "==============="
        else
            echo "WARNING - Missing valid experiment folder location : argument was $1"
        fi
    else 
        FOLDER=$1 
        for folder in $FOLDER ; do
            resumeFolder $folder $verbose
        done
    fi
    
    shift
    
done

exit
