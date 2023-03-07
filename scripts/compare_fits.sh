#!/usr/bin/env bash

for path in test_data_3d/fits-to-fits_multi_read_output_sources/*
do
    imageName=`basename $path`
    fitsdiff -n 100 test_data_3d/fits-to-fits_multi_read_output_sources/$imageName test_data_3d/fits-to-fits_sorted_grouped_read_output_sources/$imageName
done

# for path in test_data_2d/fits-to-fits_multi_read_output_sources/*
# do
#     imageName=`basename $path`
#     fitsdiff -n 100 test_data_2d/fits-to-fits_multi_read_output_sources/$imageName test_data_2d/fits-to-fits_single_read_output_sources/$imageName
# done