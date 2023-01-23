# INFO analysis.cubeletextractor (24, nid00305) [2022-06-19 17:25:53,991] - Extracting cubelet from meanMap.image.i.NGC5044_3B_band2.SB40905.cont.taylor.0.restored.conv.fits surrounding source ID 24 with slicer [3321, 10408, 0, 0] to [3360, 10433, 0, 0] with stride [1, 1, 1, 1], length [40, 26, 1, 1]

inputFile=data/selavy_cont_image.i.NGC5044_3B_band2.SB40905.cont.taylor.0.restored.conv.fits_Full_14342405

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
    printf(" {  \"sourceID\": \"%s\", \n    \
                \"slicerBegin\": %s, \n   \
                \"slicerEnd\": %s, \n   \
                \"stride\": %s, \n   \
                \"length\": %s \n },  \n", sourceID, slicerBegin, slicerEnd, stride, len);  
  }                                                 
' >> ${outputFile}

gsed -i '$ s/,//' ${outputFile}

echo "]" >> ${outputFile}