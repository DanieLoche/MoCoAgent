#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/cjpeg -dct int -progressive -opt -outfile /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small_encode.jpeg /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/input_small.ppm
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/jpeg-6a/djpeg -dct int -ppm -outfile /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small_decode.ppm /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/consumer/jpeg/input_small.jpg
