./runBashTest.sh input_chaine.txt FIFO ; 
echo Result FIFO : $? ; 

./runBashTest.sh input_chaine.txt RR ; 
echo Result RR : $? ; 

./runBashTest.sh input_chaine.txt RM ; 
echo Result RM : $?

