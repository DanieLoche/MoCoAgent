# Lance la même experimentation selon les 3 ordonancements Possibles : RM, RR, FIFO;

input=input_chaine.txt
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

sudo stress-ng --ionice-class rt --ionice-level 0 --cache 4 --fault 4 --io 4 --matrix 2 &
#sudo /usr/xenomai/bin/dohell -s 192.168.0.1 -m /tmp 10800 &

sleep 1

./runBashTest.sh -i $input -s 1 -d $duration -l $load -o STRESS; FIFO=$? ;

./runBashTest.sh -i $input -s 0 -d $duration -l $load -o STRESS; OTHER=$? ;

./runBashTest.sh -i $input -s 2 -d $duration -l $load -o STRESS; RR=$? ;

#./runBashTest.sh -i $input -s 7 -d $duration -l $load -o STRESS; RM=$? ;

sleep 1
sudo killall stress-ng
#sudo killall dohell

echo "Result FIFO : $FIFO"
echo "Result OTHER : $OTHER"
echo "Result RR : $RR"
echo "Result RM : $RM"
