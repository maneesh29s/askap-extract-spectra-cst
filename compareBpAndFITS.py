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
		# if (it != 1):
		# 	it += 1
		# 	continue

		print(imageName)
		print(imageName.split('.')[0])
		bpData = np.array(bpFile.read(imageName.split('.')[0]))

		fitsImagePath = "test_data/fits-to-fits_single_read_output_sources/"+imageName
		print(fitsImagePath)

		### Using fitsio library
		fitsData = np.array(fitsio.read(fitsImagePath))
		
		### Using astropy library
		# hdul = fits.open(fitsImagePath)
		# fitsData = np.ascontiguousarray(hdul[0].data , dtype=hdul[0].data.dtype.name)

		# bpData = np.resize(bpData, fitsData.shape)

		print("Bpdata Shape : \n" , bpData.shape)
		print("FITSData shape : \n" , fitsData.shape)

		print("Bpdata : \n" , bpData[0][0][0:2])
		print()
		print("FITSData : \n" , fitsData[0][0][0:2])

		# assert (fitsData.shape == bpData.shape)

		# if (np.array_equal(bpData, fitsData)):
		#     print(imageName, " matches")

		# for i in range(5):
		#     for j in range(14):
		#         np.format_float_scientific(bpData[0][0][i][j])
		#         print(bpData[0][0][i][j], "  ", fitsData[0][0][i][j])
		#         assert (math.isclose(
		#             bpData[0][0][i][j], fitsData[0][0][i][j],rel_tol=1e-6))

		break


# with adios2.open(bpFilePath, "r") as bpFile:
#     with fits.open(fitsFilePath, "r") as fitsFile:
#         i = 0
#         for fstep in bpFile:
#             # inspect variables in current step
#             step_vars = fstep.available_variables()

#             # print variables information
#             for name, info in step_vars.items():
#                 print("variable_name: " + name)
#                 for key, value in info.items():
#                     print("\t" + key + ": " + value)
#                 print("\n")

#             # track current step
#             step = fstep.current_step()
#             if (step == 0):
#                 size_in = fstep.read("size")

#             print("Fstep ", i, " Done")
#             i += 1
