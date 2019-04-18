#!/bin/bash

list_dos=`ls | grep "recup"`

for dossier in $list_dos
do
	mv ./$dossier/* /home/danlo/Bureau/recovery/
done

