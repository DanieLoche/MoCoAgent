#!/bin/sh
./bench/jpg/cjpeg -dct int -progressive -opt -outfile ./bench/output/output_large_encode.jpeg ./bench/jpg/input_large.ppm
./bench/jpg/djpeg -dct int -ppm -outfile ./bench/output/output_large_decode.ppm ./bench/jpg/input_large.jpg
