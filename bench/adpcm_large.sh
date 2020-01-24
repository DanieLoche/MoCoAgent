#!/bin/sh
./bench/adpcm/adpcm_rawcaudio < ./bench/adpcm/large.pcm > ./bench/output/output_large.adpcm
./bench/adpcm/adpcm_rawdaudio < ./bench/adpcm/large.adpcm > ./bench/output/output_large.pcm
