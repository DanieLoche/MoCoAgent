#!/bin/sh
home/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/cjpeg -dct int -progressive -opt -outfile home/BrixCode/MoCoAgent/Benchmarker/output/output_large_encode.jpeg input_large.ppm
home/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/djpeg -dct int -ppm -outfile home/BrixCode/MoCoAgent/Benchmarker/output/output_large_decode.ppm input_large.jpg
