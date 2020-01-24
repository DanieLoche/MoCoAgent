#!/bin/sh
./bench/jpg/cjpeg -dct int -progressive -opt -outfile ./bench/output/output_small_encode.jpeg ./bench/jpg/input_small.ppm
./bench/jpg/djpeg -dct int -ppm -outfile ./bench/output/output_small_decode.ppm ./bench/jpg/input_small.jpg
