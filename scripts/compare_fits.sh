#!/usr/bin/env bash

for path in test_data/fits-to-fits_single_read_output_sources/*
do
    imageName=`basename $path`
    fitsdiff -n 100 test_data/fits-to-fits_multi_read_output_sources/$imageName test_data/fits-to-fits_single_read_output_sources/$imageName
done