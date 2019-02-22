#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/bin/toast -fps -c /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/large.au > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large.encode.gsm
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/bin/untoast -fps -c /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/gsm/data/large.au.run.gsm > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large.decode.run
