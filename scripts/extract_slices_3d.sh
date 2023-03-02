#!/usr/bin/env bash

# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:00,837] - Extracting spectrum for Stokes 1 from image "image.restored.i.NGC5044_3B_band2.SB40905.contcube.conv.fits".
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:00,863] - Extracting spectrum from image.restored.i.NGC5044_3B_band2.SB40905.contcube.conv.fits with shape [12427, 12424, 1, 144] for source ID SB40905_component_24a using slicer [2308, 9383, 0, 0] to [2312, 9387, 0, 143] with stride [1, 1, 1, 1], length [5, 5, 1, 144] and Stokes I
# INFO  analysis.noiseSpectrumExtractor (24, nid00305) [2022-06-19 17:29:05,131] - Extracting noise spectrum from image.restored.i.NGC5044_3B_band2.SB40905.contcube.conv.fits surrounding source ID SB40905_component_24a with slicer [2283, 9358, 0, 0] to [2337, 9412, 0, 143] with stride [1, 1, 1, 1], length [55, 55, 1, 144]
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:12,078] - Extracting spectrum for Stokes 2 from image "image.restored.q.NGC5044_3B_band2.SB40905.contcube.conv.fits".
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:12,106] - Extracting spectrum from image.restored.q.NGC5044_3B_band2.SB40905.contcube.conv.fits with shape [12427, 12424, 1, 144] for source ID SB40905_component_24a using slicer [2308, 9383, 0, 0] to [2312, 9387, 0, 143] with stride [1, 1, 1, 1], length [5, 5, 1, 144] and Stokes Q
# INFO  analysis.noiseSpectrumExtractor (24, nid00305) [2022-06-19 17:29:15,897] - Extracting noise spectrum from image.restored.q.NGC5044_3B_band2.SB40905.contcube.conv.fits surrounding source ID SB40905_component_24a with slicer [2283, 9358, 0, 0] to [2337, 9412, 0, 143] with stride [1, 1, 1, 1], length [55, 55, 1, 144]
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:21,686] - Extracting spectrum for Stokes 3 from image "image.restored.u.NGC5044_3B_band2.SB40905.contcube.conv.fits".
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:21,710] - Extracting spectrum from image.restored.u.NGC5044_3B_band2.SB40905.contcube.conv.fits with shape [12427, 12424, 1, 144] for source ID SB40905_component_24a using slicer [2308, 9383, 0, 0] to [2312, 9387, 0, 143] with stride [1, 1, 1, 1], length [5, 5, 1, 144] and Stokes U
# INFO  analysis.noiseSpectrumExtractor (24, nid00305) [2022-06-19 17:29:28,554] - Extracting noise spectrum from image.restored.u.NGC5044_3B_band2.SB40905.contcube.conv.fits surrounding source ID SB40905_component_24a with slicer [2283, 9358, 0, 0] to [2337, 9412, 0, 143] with stride [1, 1, 1, 1], length [55, 55, 1, 144]
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:41,240] - Extracting spectrum for Stokes 4 from image "image.restored.v.NGC5044_3B_band2.SB40905.contcube.conv.fits".
# INFO  analysis.sourcespectrumextractor (24, nid00305) [2022-06-19 17:29:41,265] - Extracting spectrum from image.restored.v.NGC5044_3B_band2.SB40905.contcube.conv.fits with shape [12427, 12424, 1, 144] for source ID SB40905_component_24a using slicer [2308, 9383, 0, 0] to [2312, 9387, 0, 143] with stride [1, 1, 1, 1], length [5, 5, 1, 144] and Stokes V
# INFO  analysis.noiseSpectrumExtractor (24, nid00305) [2022-06-19 17:29:45,965] - Extracting noise spectrum from image.restored.v.NGC5044_3B_band2.SB40905.contcube.conv.fits surrounding source ID SB40905_component_24a with slicer [2283, 9358, 0, 0] to [2337, 9412, 0, 143] with stride [1, 1, 1, 1], length [55, 55, 1, 144]

if [ $# -ne 1 ]
then
    >&2 echo "Usage: ./scripts/extract_slices.sh <path to log file>";
    exit 1;
fi

fullfile=$1

filename=$(basename -- "$fullfile")
extension="${filename##*.}"

if [ ${extension} != "log" ]
then
    >&2 echo "input must be a log file";
    >&2 echo "Usage: ./scripts/extract_slices.sh <path to log file>";
    exit 1;
fi

inputFile="${fullfile%.*}"

# update as per need
truncatedLogFile=${inputFile}-truncated-spectrum-I.log
outputFile=${inputFile}-processed-spectrum-I.json

# considering all the stokes
# sed "/^INFO.*Extracting spectrum from image\.restored\.[iquv]\.NGC5044_3B_band2\.SB40905\.contcube\.conv\.fits.*$/!d" ${inputFile}.log > ${truncatedLogFile}

# only considering one of the stokes to simplify things
sed "/^INFO.*Extracting spectrum from image\.restored\.i\.NGC5044_3B_band2\.SB40905\.contcube\.conv\.fits.*$/!d" ${inputFile}.log > ${truncatedLogFile}

echo "[" > ${outputFile}

cat ${truncatedLogFile} | awk '       
  {                                                 
    sourceID = $21;                                 
    slicerBegin = $24 $25 $26 $27;                 
    slicerEnd = $29 $30 $31 $32;                   
    stride = $35 $36 $37 substr($38, 1, length($38)-1);                       
    len = $40 $41 $42 $43; 
    stokes = $46;                      
    printf("{ \"sourceID\": \"%s\", \"slicerBegin\": %s, \"slicerEnd\": %s, \"stride\": %s, \"length\": %s, \"stokes\": \"%s\" },\n", sourceID, slicerBegin, slicerEnd, stride, len, stokes);  
  }                                                 
' >> ${outputFile}

command=sed
if [ `uname` = "Darwin" ]
then
  command=gsed
fi

# to remove the last ',' in last line
${command} -i '$ s/\(.*\),/\1/' ${outputFile}

echo "]" >> ${outputFile}