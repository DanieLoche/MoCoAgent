input=input_benchmarkTasks.in
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

./runBashTestTasks.sh -i $input -s 1 -d $duration -l $load ;
result=$? ;
#rm ./bench/output/*
echo "Result : $result"
