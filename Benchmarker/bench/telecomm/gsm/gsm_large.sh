#!/bin/sh
bin/toast -fps -c home/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/large.au > home/BrixCode/MoCoAgent/Benchmarker/output/output_large.encode.gsm
bin/untoast -fps -c home/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/large.au.run.gsm > home/BrixCode/MoCoAgent/Benchmarker/output/output_large.decode.run
