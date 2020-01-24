#!/bin/sh
./bench/blowfish/bf e ./bench/blowfish/input_large.asc ./bench/output/output_large.enc 1234567890abcdeffedcba0987654321
./bench/blowfish/bf d ./bench/output/output_large.enc ./bench/output/output_large.asc 1234567890abcdeffedcba0987654321
