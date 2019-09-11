#! /bin/bash

names=("basicmath_S" "basicmath_L" "bitcount_S" "bitcount_L" "qsort_S" "qsort_L" "susan_S_smooth" "susan_S_edges" "susan_S_corners" "susan_L_smooth" "susan_L_edges" "susan_L_corners" "cjpeg_S" "cjpeg_L" "djpeg_S" "djpeg_L" "typeset_S" "typeset_L" "dijkstra_S" "dijkstra_L" "patricia_S" "patricia_L" "stringsearch_L" "stringsearch_S" "blowfishE_S" "blowfishD_S" "blowfishE_L" "blowfishD_L" "sha_S" "sha_L" "adpcmCaudio_S" "adpcmCaudio_L" "adpcmDaudio_S" "adpcmDaudio_L" "CRC32_S" "CRC32_L" "fft_S" "fft_L" "fft_inv_S" "fft_inv_L" "gsmToast_S" "gsmToast_L" "gsmUToast_S" "gsmUToast_L")   

DO=func
inputFile=./baseFile_funcs.txt

while [ "$1" != "" ]; do
    case $1 in
        -i | --input)   shift
                        inputFile=$1
                        DO=func
                        ;;
        -m | --mode)     shift
		        inputFile=./baseFile.txt
                        DO=exec
                        ;;
        -h | --help )   echo "usage: scriptGenerationFichiers.sh [[-o outputmode = (0,1)] "
                        exit
    esac
    shift
done

if test $DO == "func"
then
	for n in ${names[*]}
	do
	    cp $inputFile ./${n}.tmp
	    numLigne=0
	    while read -r firstWord restLine # Premier nom de la ligne
	    do
		if test $numLigne == 0
		then
		    echo "ID  name 		     meanET  wcet    isHRT aff  prec path                       arguments"
		    numLigne=`expr $numLigne + 1`
		else
		    if test $n == $firstWord
		    then
		        echo $numLigne $firstWord $restLine
		        numLigne=`expr $numLigne + 1`
		    fi
		fi
	    done < ${n}.tmp > ./${n}.in
	    rm ./${n}.tmp
	done
else
	for n in ${names[*]}
	do
	    cp ./baseFile.txt ./${n}.tmp
	    numLigne=0
	    while read -r firstWord restLine # Premier nom de la ligne
	    do
		if test $numLigne == 0
		then
		    echo "ID  name 		     meanET  wcet    isHRT aff  prec path                       arguments"
		    numLigne=`expr $numLigne + 1`
		else
		    if test $n == $firstWord
		    then
		        echo $numLigne $firstWord $restLine
		        numLigne=`expr $numLigne + 1`
		    fi
		fi
	    done < ${n}.tmp > ./${n}.in
	    rm ./${n}.tmp
	done

fi
