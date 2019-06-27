#!/usr/bin/env bash

# Il faut :
# Un fichier chain.txt pour chaque chaine de tâche à benchmarker.
# Pour chaque fichier, lancer ./MoCoAgent.out - 20min [chaine] [chaineBenchOutput]

# De là, on a :
# La répartition du temps d'execution des chaines en isolation,
# et donc le WCET des tâches de la chaine + WCET de la chaîne.

# on fixe donc ces temps dans le fichier général d'informations.
# ---------------------------------------- #

# Ensuite vient la phase de benchmarks :

commentaire="//"
file=$1
schedPolicy=$2
if(( $# >= 3 )) && test -f $file
then
    duration=$3
else 
    duration=500
fi
if(( $# >= 4 )) && test -f $file
then
    load=$4
else 
    load=80
fi

if(( $# >= 1 )) && test -f $file
then
    dirName=./Experimentations/Expe_`date +%e-%m-%Hh`
    mkdir $dirName
    for toExecute in {2..6} # exécuter toutes les chaines
    do
        #echo $toExecute
        numLigne=1
        while read -r line
        do
            if test $numLigne -eq $toExecute
            then
                echo $line
                name=`echo $line | awk '{print $5}'`
            else
                echo "${commentaire}$line"
            fi
            numLigne=`expr $numLigne + 1`
        done < $file > inputFile
        #echo "Name is "$name
        sudo sar -o ${dirName}/IODatas${name}_1_${duration}_${load}_${schedPolicy} -P 0-3 1 $duration > /dev/null 2>&1 & 
        ./MoCoAgent.out true  $duration $load ./inputFile ${dirName}/${name}_1_${duration}_${load}_${schedPolicy} $schedPolicy
        rm ./bench/output/*

        #sudo sar -o ${dirName}/IODatas${name}_0_${duration}_${load}_${schedPolicy} -P 0-3 1 $duration > /dev/null 2>&1 & 
        #./MoCoAgent.out false $duration $load ./inputFile ${dirName}/${name}_0_${duration}_${load}_${schedPolicy} $schedPolicy    
        #rm ./bench/output/*
        #rm -f ./inputFile
    done
    # Convertir les fichiers SAR en .csv... la totale..!
    for file in ${dirName}/IODatas*
    do
        sadf -dh -- -P 0-3 ${file} > tmp1        
        sadf -dh -- -P 0-3 -rB -m CPU,TEMP ${file} | cut -d\; -f1-3 --complement > tmp2
        headerBefore="# hostname;interval;timestamp; CPU0;CPU0%user;CPU0%nice;CPU0%system;CPU0%iowait;CPU0%steal;CPU0%idle; CPU1;CPU1%user;CPU1%nice;CPU1%system;CPU1%iowait;CPU1%steal;CPU1%idle; CPU2;CPU2%user;CPU2%nice;CPU2%system;CPU2%iowait;CPU2%steal;CPU2%idle; CPU3;CPU3%user;CPU3%nice;CPU3%system;CPU3%iowait;CPU3%steal;CPU3%idle; pgpgin-s;pgpgout-s;fault-s;majflt-s;pgfree-s;pgscank-s;pgscand-s;pgsteal-s; %vmeff;kbmemfree;kbavail;kbmemused;%memused;kbbuffers;kbcached;kbcommit;%commit;kbactive;kbinact;kbdirty; CPU0;CPU0MHz;CPU1;CPU1MHz;CPU2;CPU2MHz;CPU3;CPU3MHz; CPU0TEMP;CPU0DEVICE;CPU0degC;CPU0%temp; CPU1TEMP;CPU1DEVICE;CPU1degC;CPU1%temp; CPU2TEMP;CPU2DEVICE;CPU2degC;CPU2%temp; CPU3TEMP;CPU3DEVICE;CPU3degC;CPU3%temp; CPU4TEMP;CPU4DEVICE;CPU4degC;CPU4%temp; CPU5TEMP;CPU5DEVICE;CPU5degC;CPU5%temp"
    # On dégage toutes les colonnes inutiles
        linesToCut="2,4,6,9,11,13,16,18,20,23,25,27,30,35,37-40,52,54,56,58,60,61,63-65,67-69,71-73,75-83"
        newfileName=`echo $file | cut -c 10-`
        newfileName=IOD_$newfileName
        paste -d ";" tmp1 tmp2 | cut -d\; -f${linesToCut} --complement > ${newfileName}.csv
        header=`echo $headerBefore | cut -d\; -f${linesToCut} --complement`
#"# hostname;timestamp; CPU0%user;CPU0%system;CPU0%iowait;CPU0%idle; CPU1%user;CPU1%system;CPU1%iowait;CPU1%idle; CPU2%user;CPU2%system;CPU2%iowait;CPU2%idle; CPU3%user;CPU3%system;CPU3%iowait;CPU3%idle; pgpgin/s;pgpgout/s;fault/s;pgfree/s;kbmemfree;kbavail;kbmemused;%memused;kbbuffers;kbcached;kbcommit;%commit;kbactive;kbinact;kbdirty;CPU0MHz;CPU1MHz;CPU2MHz;CPU3MHz;CPU0degC;CPU1degC;CPU2degC;CPU3degC"
    # Remplace le header selon les colonnes dégagées
        sed -i "1s/.*/$header/" ${newfileName}.csv
    done
    rm tmp1 tmp2
    exit 0
else
    exit 1
fi
