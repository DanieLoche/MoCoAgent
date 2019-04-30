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

# 
