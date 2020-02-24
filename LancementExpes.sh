input=input_chaine.txt
duration=100
Duration=0
load=80

while [ "$1" != "" ]; do
    case $1 in
        -i | -f | --input )     shift
                                input=$1
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

#./runBashTest.sh -i $input -s FIFO -d $duration -l $load; 
#FIFO=$? ; 

./runBashTest.sh -i $input -s RR -d $duration -l $load; 
RR=$? ; 

#./runBashTest.sh -i $input -s RM -d $duration -l $load; 
#RM=$? ;

#echo "Result FIFO : $FIFO"
echo "Result RR : $RR"
#echo "Result RM : $RM"