#!/usr/bin/env python
# coding: utf-8
import fitsio
from astropy.io import fits
import adios2
import numpy as np
import os
import math

# Arguments passed
fitsImageNames = os.listdir(
	"test_data/fits-to-fits_single_read_output_sources")

it = 0
with adios2.open("test_data/fits-to-bp_single_read_output_sources.bp", "r") as bpFile:
	for imageName in fitsImageNames:
		bpData = np.array(bpFile.read(imageName.split('.')[0]))

		fitsImagePath = "test_data/fits-to-fits_single_read_output_sources/"+imageName
		### Using fitsio library
		fitsData = np.array(fitsio.read(fitsImagePath))

		### Using astropy library
		# hdul = fits.open(fitsImagePath)
		# fitsData = np.ascontiguousarray(hdul[0].data , dtype=hdul[0].data.dtype.name)

		# RESHAPING BP array, because fitsio API follows C order, thus it reverses the FITS image shape
		bpData = np.resize(bpData, fitsData.shape)
				
		## DEBUG
		# print(imageName)
		# print(imageName.split('.')[0])
		# print(fitsImagePath)
		# print("Bpdata Shape : \n" , bpData.shape)
		# print("FITSData shape : \n" , fitsData.shape)
		# print("Bpdata : \n\n" , bpData[0][0][0:2])
		# print("FITSData : \n" , fitsData[0][0][0:2])

		assert (fitsData.shape == bpData.shape)

		if not np.array_equal(bpData, fitsData):
		    print(imageName, "does not match")

		# break