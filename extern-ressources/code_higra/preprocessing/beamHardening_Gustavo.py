#!/usr/bin/env python
"""Original file written and shared by Gustavo Pinzon-Forero

Contact at "Gustavo PINZON FORERO" <gustavo.pinzon-forero@esrf.fr>; 
"""

# ========================
# Import modules
# ========================
import spam.DIC
import spam.label
import tifffile
import spam.helpers
import numpy, os
import multiprocessing
import progressbar
import matplotlib.pyplot as plt

# ========================
# Functions
# ========================

global fixBeamHardening
def fixBeamHardening(pixOrVal, coord, PV, PS, mS, mV, bS, bV):
    
    distance = numpy.sqrt( (coord[0] - comBeam[0])**2 + (coord[1] - comBeam[1])**2 + (coord[2] - comBeam[2])**2)
    gS = mS*distance + bS
    gV = mV*distance + bV
    
    pixModifVal = int(((PS - PV)/(gS - gV))*(pixOrVal - gV) + PV)
    if pixModifVal < 0: pixModifVal=0
    if pixModifVal > 65535: pixModifVal=65535
    
    return pixModifVal


global computepeaks
def computepeaks(pos):
    coord = grid[pos]
    # Check if the point is inside the Full image
    if imFull[int(coord[0]), int(coord[1]),int(coord[2])]:
        # We are inside compute the peaks
        subsetGrey = imGrey[int(coord[0])-cubeSize:int(coord[0])+cubeSize,
                            int(coord[1])-cubeSize:int(coord[1])+cubeSize,
                            int(coord[2])-cubeSize:int(coord[2])+cubeSize]
        subsetBin = imFull[int(coord[0])-cubeSize:int(coord[0])+cubeSize,
                           int(coord[1])-cubeSize:int(coord[1])+cubeSize,
                           int(coord[2])-cubeSize:int(coord[2])+cubeSize]
        peaks = spam.helpers.histogramTools.findHistogramPeaks(subsetGrey*subsetBin, valley1 = 40000)
        distCOM = numpy.sqrt( (coord[0] - comBeam[0])**2 + (coord[1] - comBeam[1])**2 + (coord[2] - comBeam[2])**2)
        radius = numpy.sqrt((coord[1] - comFull[1])**2 + (coord[2] - comFull[2])**2) 
        
        return peaks[0], peaks[1], distCOM, radius, 1
    else:
        return 0, 0, 0, 0, 0

global correctImage
def correctImage(index):
    zStart = int(imFull.shape[0] / numProc)*index
    zStop  = int(imFull.shape[0] / numProc)*(index+1)
    if zStop > imFull.shape[0]: zStop=int(imFull.shape[0])
    imGreySub = imGrey[zStart:zStop,:,:]
    imCorr = numpy.zeros(( (zStop-zStart) , imFull.shape[1], imFull.shape[2]))
    for i in range(imCorr.shape[0]):
        for j in range(imCorr.shape[1]):
            for k in range(imCorr.shape[2]):
                imCorr[i,j,k] = fixBeamHardening(imGreySub[i,j,k], [i+zStart,j,k], 16383, 49151, coefSolid[0], coefVoid[0], coefSolid[1], coefVoid[1])

    return imCorr, index

# ========================
# Working folders
# ========================
codeFolder = os.getcwd()
test = 'EFRGP01'
numProc = 12
greyFolder = '../../RawData/Bin1/'
resFolder  = '../../Results/InitialSegmentation/'
# ========================
# Code
# ========================
print('\n\t Reading images')
# Read full image
os.chdir(codeFolder)
os.chdir(resFolder)
# Full binary image obtained through a rough tresholding, filling holes and manually erasing the caps. All done in Fiji.
imFull = tifffile.imread('EFRGP01_00_Full.tif').astype(bool) 
# Greyscale image
os.chdir(codeFolder)
os.chdir(greyFolder)
imGrey = tifffile.imread(test+'_00.tif')
# Run the code once with generateMask = True to create the mask file. Then set it to False and run again.
generateMask = False
if generateMask:
    # Generate the original greyscale image with the full image
    imGreyMask = numpy.where(imFull == True, imGrey, 0)
    os.chdir(codeFolder)
    os.chdir(resFolder)
    tifffile.imsave(test+'_00_GreyBeam.tif', imGreyMask.astype(numpy.uint16))
    exit()
    # At this point, open the GreyBeam.tif image create before and threshold it with a region between 0.7 to 0.75 relative to the normalised histogram peaks. Save it as test+'_00_BoolBeam.tif'
# Read the beam hardening boolean image that you just created
os.chdir(codeFolder)
os.chdir(resFolder)
imBeamCentre = tifffile.imread(test+'_00_BoolBeam.tif').astype(bool)
print('\n\t Computing COM')
COMBeam = spam.label.centresOfMass(imBeamCentre)
COMFull = spam.label.centresOfMass(imFull)
comFull = COMFull[1]
comBeam = COMBeam[1]
print('\n\t Creating grid')
cubeSize = 50
sz = 1
grid = spam.DIC.grid.makeGrid(imFull.shape, cubeSize)[0]
print('\n\t Computing peaks')
peakVoid = []
peakSolid = []
radiusZ = []
distanceCOM = []
widgets = [progressbar.FormatLabel(''), ' ', progressbar.Bar(), ' ', progressbar.AdaptiveETA()]
pbar = progressbar.ProgressBar(widgets=widgets, maxval=len(grid))
pbar.start()
finishedNodes = 0
# Run multiprocessing
with multiprocessing.Pool(processes=numProc) as pool:
    for returns in pool.imap_unordered(computepeaks, range(len(grid))):
        finishedNodes += 1
        pbar.update(finishedNodes)
        if returns[4] == 1:
            peakVoid.append(returns[0])
            peakSolid.append(returns[1])
            distanceCOM.append(returns[2])
            radiusZ.append(returns[3])
    pool.close()
    pool.join()
pbar.finish()
peakVoid = numpy.asarray(peakVoid)
peakSolid = numpy.asarray(peakSolid)
radiusZ = numpy.asarray(radiusZ)
distanceCOM = numpy.asarray(distanceCOM)
print('\n\t Performing the fitting #1' )
coefVoid = numpy.polyfit(distanceCOM,peakVoid,1)
poly1d_fnVoid = numpy.poly1d(coefVoid) 
coefSolid = numpy.polyfit(distanceCOM,peakSolid,1)
poly1d_fnSolid = numpy.poly1d(coefSolid) 
print('Fitting #1: \n')
print('VoidPeak = {} * distance + {}'.format(coefVoid[0], coefVoid[1]))
print('SolidPeak = {} * distance + {}'.format(coefSolid[0], coefSolid[1]))
plt.figure(1)
plt.scatter(distanceCOM, peakVoid, s=sz)
plt.scatter(distanceCOM, peakSolid, s=sz)
plt.plot(distanceCOM, poly1d_fnVoid(distanceCOM), '--k') 
plt.plot(distanceCOM, poly1d_fnSolid(distanceCOM), '--r') 
plt.savefig('fig_BeamHardeningRaw.pdf')
plt.close()
print('\n\t Performing the fitting #2' )
# Perform the fitting with only some of the data - exclude outliers. Select the values from the graphs
voidMax = 26160
solidMax = 55780
peakVoidN = peakVoid[numpy.where(peakVoid < voidMax)[0]]
peakSolidN = peakSolid[numpy.where(peakVoid < voidMax)[0]]
distanceCOMN = distanceCOM[numpy.where(peakVoid < voidMax)[0]]
peakSolidNN = peakSolidN[numpy.where(peakSolidN < solidMax)[0]]
peakVoidNN = peakVoidN[numpy.where(peakSolidN < solidMax)[0]]
distanceCOMNN = distanceCOMN[numpy.where(peakSolidN < solidMax)[0]]
coefVoid = numpy.polyfit(distanceCOMNN,peakVoidNN,1)
poly1d_fnVoid = numpy.poly1d(coefVoid) 
coefSolid = numpy.polyfit(distanceCOMNN,peakSolidNN,1)
poly1d_fnSolid = numpy.poly1d(coefSolid) 
print('Fitting #2: \n')
print('VoidPeak = {} * distance + {}'.format(coefVoid[0], coefVoid[1]))
print('SolidPeak = {} * distance + {}'.format(coefSolid[0], coefSolid[1]))

plt.figure(3)
plt.scatter(distanceCOMNN, peakVoidNN, s=sz, label='Void: {}*dist+{}'.format(coefVoid[0], coefVoid[1]))
plt.scatter(distanceCOMNN, peakSolidNN, s=sz, label='Solid: {}*dist+{}'.format(coefSolid[0], coefSolid[1]))
plt.legend()
plt.plot(distanceCOMNN, poly1d_fnVoid(distanceCOMNN), '--k') 
plt.plot(distanceCOMNN, poly1d_fnSolid(distanceCOMNN), '--r') 
plt.xlabel('Distance [px]')
plt.ylabel('Grey value [-]')
plt.xlim([0,800])
plt.ylim([0,65000])
plt.savefig('fig_BeamHardeningFinal.pdf')
plt.close()

print('\n\t Fixing beam hardening' )
widgets = [progressbar.FormatLabel(''), ' ', progressbar.Bar(), ' ', progressbar.AdaptiveETA()]
pbar = progressbar.ProgressBar(widgets=widgets, maxval=numProc)
pbar.start()
finishedNodes = 0

# Run multiprocessing
imCorrected = numpy.zeros(imFull.shape)
with multiprocessing.Pool(processes=numProc) as pool:
    for returns in pool.imap_unordered(correctImage, range(numProc)):
        finishedNodes += 1
        pbar.update(finishedNodes)
        imCorrected[int(imFull.shape[0] / numProc)*returns[1]:int(imFull.shape[0] / numProc)*(returns[1]+1), :,:] = returns[0]
    pool.close()
    pool.join()
pbar.finish()
os.chdir(codeFolder)
os.chdir(resFolder)
tifffile.imsave(test+'_00.tif', imCorrected.astype(numpy.uint16))

        
