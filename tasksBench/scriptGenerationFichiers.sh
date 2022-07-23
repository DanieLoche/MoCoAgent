#! /bin/bash

names=("adpcmCaudio_L"
"adpcmCaudio_S"
"adpcmDaudio_L"
"adpcmDaudio_S"
"basicmath_L"
"basicmath_S"
"bitcount_L"
"bitcount_S"
"blowfishD_L"
"blowfishD_S"
"blowfishE_L"
"blowfishE_S"
"cjpeg_L"
"cjpeg_S"
"CRC32_L"
"CRC32_S"
"dijkstra_L"
"dijkstra_S"
"djpeg_L"
"djpeg_S"
"fft_inv_L"
"fft_inv_S"
"fft_L"
"fft_S"
"gsmToast_L"
"gsmToast_S"
"gsmUToast_L"
"gsmUToast_S"
"patricia_L"
"patricia_S"
"qsort_L"
"qsort_S"
"rijndaelD_L"
"rijndaelD_S"
"rijndaelE_L"
"rijndaelE_S"
"sha_L"
"sha_S"
"stringsearch_L"
"stringsearch_S"
"susan_L_corners"
"susan_L_edges"
"susan_L_smooth"
"susan_S_corners"
"susan_S_edges"
"susan_S_smooth"
"load_L"
"load_S"
"typeset_L"
"typeset_S")

inputFile=./baseFile_funcs.csv
DO=func         # default behaviour

while [ "$1" != "" ]; do
    case $1 in
        -i | --input)   shift
                        inputFile=$1
                        DO=func
                        ;;
        -m | --mode)     shift
		        inputFile=./baseFile.csv
                        DO=exec
                        ;;
        -h | --help )   echo "usage: scriptGenerationFichiers.sh [[-o outputmode = (0,1)] "
                        exit
    esac
    shift
done

cpt=1

if test $DO == "func"
then
	for n in ${names[*]}
	do
	    cp $inputFile ./${n}.tmp
	    numLigne=0
	    while read -r firstWord garbage restLine # Premier nom de la ligne
	    do
		if test $numLigne == 0 ; then
		    echo "ID  name 	wcet  period isHRT aff  prec func                       arguments" > ./${n}.in
		    numLigne=`expr $numLigne + 1`
		elif test $n == $firstWord ; then
		        echo $cpt $firstWord $restLine		>> ./${n}.in
		        numLigne=`expr $numLigne + 1`
		elif test $n == $garbage -a $firstWord == "//" ; then
			restLine=`echo ${restLine//\"/}`
			echo $firstWord $restLine			>> ./${n}.in
		        numLigne=`expr $numLigne + 1`
		fi
	    done < ${n}.tmp
	    rm ./${n}.tmp
        cpt=`expr $cpt + 1`
	done



else  # DO="exec"
	for n in ${names[*]}
	do
	    cp ./baseFile.csv ./${n}.tmp
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
