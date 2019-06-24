#!/bin/sh
./bench/gsm/toast -fps -c ./bench/gsm/large.au > ./bench/output/output_large.encode.gsm
./bench/gsm/untoast -fps -c ./bench/gsm/large.au.run.gsm > ./bench/output/output_large.decode.run
