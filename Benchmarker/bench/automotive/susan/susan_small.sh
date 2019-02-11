#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/susan /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/input_small.pgm /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.smoothing.pgm -s
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/susan /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/input_small.pgm /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.edges.pgm -e
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/susan /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/automotive/susan/input_small.pgm /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.corners.pgm -c
