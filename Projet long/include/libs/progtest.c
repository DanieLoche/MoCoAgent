#include <stdio.h>

extern "C" {
int basicmath_small(int argc, char *argv[]);
}

int main(int argc, char *argv[])
{
	printf("Debut test\n");
	int ret = 0;
	ret += basicmath_small( argc, argv);
//	ret += basicmath_large( argc, argv);
//	ret += bitcount_func( argc, argv);
//	ret += qsort_small( argc, argv) ;
//	ret += qsort_large( argc, argv) ;
//	ret += susan( argc, argv);
//	ret += djpeg_func( argc, argv);
//	ret += cjpeg_func( argc, argv);
//	ret += dijkstra_small( argc, argv) ;
//	ret += dijkstra_large( argc, argv) ;
//	ret += patricia( argc, argv);
//	ret += stringsearch_small( argc, argv);
//	ret += stringsearch_large( argc, argv);
//	ret += blowfish( argc, argv);
//	ret += rijndael( argc, argv);
//	ret += sha( argc, argv);
//	ret += rawdaudio( argc, argv);
//	ret += rawcaudio( argc, argv);
//	ret += crc( argc, argv);
//	ret += fft( argc, argv) ;
//	ret += typeset_func( argc, argv);
//	ret += gsm_func ( argc, argv);

	printf("Fin test : %d.\n", ret);
	return ret;

}
