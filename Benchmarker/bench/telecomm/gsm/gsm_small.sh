#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/bin/toast -fps -c /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/small.au > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.encode.gsm
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/bin/untoast -fps -c /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/small.au.run.gsm > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.decode.run
