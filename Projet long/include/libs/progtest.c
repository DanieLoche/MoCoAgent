#include <stdio.h>
#include <iostream>

#include <chrono>
using namespace std::chrono;

extern "C" {
	int basicmath_small   (int argc, char *argv[]);
	int basicmath_large   (int argc, char *argv[]);
	int bitcount_func     (int argc, char *argv[]);
	int qsort_small       (int argc, char *argv[]);
	int qsort_large       (int argc, char *argv[]);
	int susan             (int argc, char *argv[]);
	int djpeg_func        (int argc, char *argv[]);
	int cjpeg_func        (int argc, char *argv[]);
	int typeset_func      (int argc, char *argv[]);
	int dijkstra_small    (int argc, char *argv[]);
	int dijkstra_large    (int argc, char *argv[]);
	int patricia          (int argc, char *argv[]);
	int stringsearch_small(int argc, char *argv[]);
	int stringsearch_large(int argc, char *argv[]);
	int blowfish          (int argc, char *argv[]);
	int rijndael          (int argc, char *argv[]);
	int sha               (int argc, char *argv[]);
	int rawdaudio         (int argc, char *argv[]);
	int rawcaudio         (int argc, char *argv[]);
	int crc               (int argc, char *argv[]);
	int fft               (int argc, char *argv[]);
	int gsm_func          (int argc, char *argv[]);
}

int main(int argc, char *argv[])
{
	printf("Debut test\n");
	int ret = 0;
	auto start = high_resolution_clock::now();
	ret += basicmath_small( argc, argv);
	auto mid = high_resolution_clock::now();
	ret += basicmath_large( argc, argv);
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
	auto end = high_resolution_clock::now();
	printf("Fin test : %d.\n", ret);
	auto duration = duration_cast<microseconds>(end - start);
	auto midTime = duration_cast<microseconds>(mid - start);
	std::cout << "Total duration = " << duration.count() << "us. Miduration = " << midTime.count() <<"us." << std::endl;
	return ret;

}
