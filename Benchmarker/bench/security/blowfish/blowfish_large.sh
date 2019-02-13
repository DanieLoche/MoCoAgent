#!/bin/sh
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/security/blowfish/bf e /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/security/blowfish/input_large.asc /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large.enc 1234567890abcdeffedcba0987654321
/home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/security/blowfish/bf d /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large.enc /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_large.asc 1234567890abcdeffedcba0987654321
