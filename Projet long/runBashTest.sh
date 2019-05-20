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
if(( $# >= 1 )) && test -f $file
then
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
        sudo sar -o ./Experimentations/Expe_16-05/IODatas${name}_1_600_80_${schedPolicy} -P 0,1,2,3 1 600 > /dev/null 2>&1 & 
        ./MoCoAgent.out true  600 80 ./inputFile Chain2${name}_1_600_80_${schedPolicy}.csv2 $schedPolicy
        rm ./bench/output/*

        sudo sar -o ./Experimentations/Expe_16-05/IODatas${name}_0_600_80_${schedPolicy} -P 0,1,2,3 1 600 > /dev/null 2>&1 & 
        ./MoCoAgent.out false 600 80 ./inputFile Chain2${name}_0_600_80_${schedPolicy}.csv2 $schedPolicy    
        rm ./bench/output/*
        rm -f ./inputFile
    done
    exit 0
else
    exit 1
fi
