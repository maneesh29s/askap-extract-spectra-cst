#!/usr/bin/env bash

# cubeletJsonFile=data/selavy_cont_image_JSON_files/selavy_cont_image-cubelet.json

cubeletJsonFileSorted=data/selavy_cont_image_JSON_files/selavy_cont_image-cubelet-sorted.json

# spectralJsonFile=data/selavy_cont_image_JSON_files/selavy_cont_image-spectrum-I-slicer-scaledToHalf.json

spectralJsonFileSorted=data/selavy_cont_image_JSON_files/selavy_cont_image-spectrum-I-slicer-scaledToHalf-sorted.json

outputFile="benchmarks-`TZ=Asia/Kolkata date +%Y-%m-%dT%H:%M`.txt"

# Cubelet Extractor (2D)
echo "Cubelet JSON File being processed: ${cubeletJsonFileSorted}" >> ${outputFile}
# fits to fits - single read for whole data
make build/bench_cubelet_extractor_2d.out OPT="-DSINGLE_READ"
./build/bench_cubelet_extractor_2d.out fits fits ${cubeletJsonFileSorted} >> ${outputFile}
# fits to bp - single read for whole data
./build/bench_cubelet_extractor_2d.out fits bp ${cubeletJsonFileSorted} >> ${outputFile}

# fits to fits - sliced read (source by source)
make build/bench_cubelet_extractor_2d.out OPT="-DSLICED_READ"
./build/bench_cubelet_extractor_2d.out fits fits ${cubeletJsonFileSorted} >> ${outputFile}
# fits to bp - sliced read (source by source)
./build/bench_cubelet_extractor_2d.out fits bp ${cubeletJsonFileSorted} >> ${outputFile}

# Spectral Extractor (3D)
echo "Spectral JSON File being processed: ${spectralJsonFileSorted}" >> ${outputFile}
# fits to fits - sorted-grouped reads
make build/bench_spectrum_extractor_3d.out OPT="-DSORTED_GROUPED_READ"
./build/bench_spectrum_extractor_3d.out fits fits ${spectralJsonFileSorted} >> ${outputFile}
# fits to bp - sorted-grouped reads
./build/bench_spectrum_extractor_3d.out fits bp ${spectralJsonFileSorted} >> ${outputFile}

# fits to fits - sliced read (source by source)
make build/bench_spectrum_extractor_3d.out OPT="-DSLICED_READ"
./build/bench_spectrum_extractor_3d.out fits fits ${spectralJsonFileSorted} >> ${outputFile}
# fits to bp - sliced read (source by source)
./build/bench_spectrum_extractor_3d.out fits bp ${spectralJsonFileSorted} >> ${outputFile}

