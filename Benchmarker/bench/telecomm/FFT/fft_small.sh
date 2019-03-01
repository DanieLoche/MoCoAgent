#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/FFT/fft 4 4096 > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.txt
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/FFT/fft 4 8192 -i > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.inv.txt
