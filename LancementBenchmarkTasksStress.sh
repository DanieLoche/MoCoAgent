#!/usr/bin/env bash

input=input_benchmarkTasksStress.in
duration=100
Duration=0
load=100

while [ "$1" != "" ]; do
    case $1 in
        -i | -f | --input )     shift
                                input=$1
                                ;;
        -d | --duration )    	shift
				               duration=$1
                                ;;
    	-l | --load )           shift
                                load=$1
                                ;;
        -h | --help )           echo "usage: LancementExpes.sh [[-i input ] [-d duration | -D DurationTotal] [-l load] ] | [-h]"
                                exit
                                ;;
        * )                     echo "usage: LancementExpes.sh [[-i input ] [-d duration | -D DurationTotal] [-l load] ] | [-h]"
                                exit 1
    esac
    shift
done

#creation des stressors de caches et I/O
#sudo stress-ng --cache 4 --io 4 &
sudo stress-ng --ionice-class rt --ionice-level 0 --cache 4 --fault 4 --io 4 --matrix 2 &
#sudo /usr/xenomai/bin/dohell -s 192.168.0.1 -m /tmp 10800 &



./runBashTestTasks.sh -i $input -s 1 -d $duration -l $load -o S
result=$? ;
sudo rm -f ./bench/output/*


#Libération des stressors
sudo killall stress-ng
#killall dohell
echo "Result : $result"
