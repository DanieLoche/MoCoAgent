#!/bin/sh
bin/toast -fps -c home/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/small.au > home/BrixCode/MoCoAgent/Benchmarker/output/output_small.encode.gsm
bin/untoast -fps -c home/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/small.au.run.gsm > home/BrixCode/MoCoAgent/Benchmarker/output/output_small.decode.run
