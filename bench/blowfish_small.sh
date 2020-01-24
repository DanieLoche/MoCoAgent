#!/bin/sh
./bench/blowfish/bf e ./bench/blowfish/input_small.asc ./bench/output/output_small.enc 1234567890abcdeffedcba0987654321
./bench/blowfish/bf d ./bench/output/output_small.enc ./bench/output/output_small.asc 1234567890abcdeffedcba0987654321
