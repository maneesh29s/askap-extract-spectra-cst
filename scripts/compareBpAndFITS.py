#!/usr/bin/env python
# coding: utf-8
import fitsio
from astropy.io import fits
import adios2
import numpy as np
import os
import math
import sys

if(len(sys.argv) != 3):
	print("Usage: python3 compareBpAndFITS.py <inputFitsDir> <inputBpImage>")
	exit(1)

# inputFitsDir = "test_data_3d/fits-to-fits_multi_read_output_sources"
inputFitsDir = sys.argv[1]

# inputBpImage = "test_data_3d/fits-to-bp_multi_read_output_sources.bp"
inputBpImage = sys.argv[2]

# Arguments passed
fitsImageNames = os.listdir(inputFitsDir)

it = 0
with adios2.open(inputBpImage, "r") as bpFile:
	for imageName in fitsImageNames:
		it += 1

		bpData = np.array(bpFile.read(imageName.split('.')[0]))

		fitsImagePath = os.path.join( inputFitsDir, imageName)
		### Using fitsio library
		fitsData = np.array(fitsio.read(fitsImagePath))

		### Using astropy library
		# hdul = fits.open(fitsImagePath)
		# fitsData = np.ascontiguousarray(hdul[0].data , dtype=hdul[0].data.dtype.name)

		# RESHAPING BP array, because fitsio API follows C order, thus it reverses the FITS image shape
		bpData = np.resize(bpData, fitsData.shape)
				
		# ## DEBUG
		# print(imageName)
		# print(imageName.split('.')[0])
		# print(fitsImagePath)
		# print("Bpdata Shape : \n" , bpData.shape)
		# print("FITSData shape : \n" , fitsData.shape)
		# print("Bpdata : \n\n" , bpData)
		# print("FITSData : \n" , fitsData)

		assert (fitsData.shape == bpData.shape)

		if not np.array_equal(bpData, fitsData):
		    print(imageName, "does not match")

		# ## DEBUG
		# break
		# if(it == 3):
		# 	break