#!/bin/sh
bin/rawcaudio < /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/adpcm/data/small.pcm > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.adpcm
bin/rawdaudio < /home/danlo/BrixCode/MoCoAgent/Benchmarker/bench/telecomm/adpcm/data/small.adpcm > /home/danlo/BrixCode/MoCoAgent/Benchmarker/output/output_small.pcm
