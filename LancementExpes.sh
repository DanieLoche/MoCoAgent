#!/usr/bin/env bash
# Lance la même experimentation selon les 3 ordonancements Possibles : RM, RR, FIFO;

input=input_chaine.txt
duration=100
Duration=0
load=100
Mode=1 

while [ "$1" != "" ]; do
    case $1 in
        -i | -f | --input )     shift
                                input=$1
                                ;;
        -m | --mode )     shift
                                Mode=$1
                                ;;
        -d | --duration )    	shift
				                duration=$1
                                ;;
        -D | --Duration )    	shift
	                        Duration=$1
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

if (( $Duration > $duration ))
then
	duration=`expr $Duration / 3`
fi

#./runBashTest.sh -i $input -s 1 -d $duration -l $load; FIFO=$? ;

./runBashTest.sh -i $input -m $Mode -s 0 -d $duration -l $load; OTHER=$? ;

#./runBashTest.sh -i $input -s 2 -d $duration -l $load; RR=$? ;

#./runBashTest.sh -i $input -s 7 -d $duration -l $load; RM=$? ;

echo "Result FIFO : $FIFO"
echo "Result OTHER : $OTHER"
echo "Result RR : $RR"
echo "Result RM : $RM"
