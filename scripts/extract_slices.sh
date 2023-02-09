#!/usr/bin/env bash

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

sed "/^INFO.*Extracting cubelet from meanMap\.image\.i\.NGC5044_3B_band2\.SB40905\.cont\.taylor\.0\.restored\.conv\.fits.*$/!d" ${inputFile}.log > ${inputFile}-truncated.log
 

outputFile=${inputFile}-processed.json

echo "[" > ${outputFile}

cat ${inputFile}-truncated.log | awk '       
  {                                                 
    sourceID = $15;                                 
    slicerBegin = $18 $19 $20 $21;                 
    slicerEnd = $23 $24 $25 $26;                   
    stride = $29 $30 $31 substr($32, 1, length($32)-1);                       
    len = $34 $35 $36 $37;                       
    printf("{ \"sourceID\": %s, \"slicerBegin\": %s, \"slicerEnd\": %s, \"stride\": %s, \"length\": %s },\n", sourceID, slicerBegin, slicerEnd, stride, len);  
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