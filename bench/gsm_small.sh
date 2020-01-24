#!/bin/sh
./bench/gsm/toast -fps -c ./bench/gsm/small.au > ./bench/output/output_small.encode.gsm
./bench/gsm/untoast -fps -c ./bench/gsm/small.au.run.gsm > ./bench/output/output_small.decode.run
