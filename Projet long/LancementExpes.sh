./runBashTest.sh input_chaine.txt FIFO ; 
FIFO="Result FIFO : $?" ; 

./runBashTest.sh input_chaine.txt RR ; 
RR="Result RR : $?" ; 

./runBashTest.sh input_chaine.txt RM ; 
RM="Result RM : $?"

