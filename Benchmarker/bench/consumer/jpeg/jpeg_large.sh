#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/cjpeg -dct int -progressive -opt -outfile /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large_encode.jpeg /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/input_large.ppm
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/djpeg -dct int -ppm -outfile /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large_decode.ppm /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/input_large.jpg
