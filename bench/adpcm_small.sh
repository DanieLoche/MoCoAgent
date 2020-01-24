#!/bin/sh
./bench/adpcm/adpcm_rawcaudio < ./bench/adpcm/small.pcm > ./bench/output/output_small.adpcm
./bench/adpcm/adpcm_rawdaudio < ./bench/adpcm/small.adpcm > ./bench/output/output_small.pcm
