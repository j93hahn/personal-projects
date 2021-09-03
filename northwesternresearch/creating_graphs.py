#!/usr/bin/env python
# coding: utf-8

#importing all of the necessary files and modules 
from astropy.io import fits
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from astropy.table import Table
from astropy.visualization import ImageNormalize, ZScaleInterval
import pandas as pd

#importing the actual data
dfN = pd.read_csv('SAGUARO Master File - N Images (2).csv', skiprows = [1])
dfS = pd.read_csv('SAGUARO Master File - S Images.csv', skiprows = [1])
df = pd.concat([dfS, dfN])
df.index = pd.RangeIndex(len(df.index))

#compartmentalizing the different types of bad images 
r = df['Name'][df['Rippling'] == 'True ']
a = df['Name'][df['Artifacts '] == 'True ']
ds = df['Name'][df['Dark Spots'] == 'True ']
fce = df['Name'][df['Flat Correction Error'] == 'True ']
gi = df['Name'][df['Good Image '] == 'True ']

rippling = np.array(r)
artifacts = np.array(a)
dark_spots = np.array(ds)
flat_correction_error = np.array(fce)
good_images = np.array(gi)

#lists used to accumulate data about the images and drawing conclusions
ra = dict((str(val), []) for val in range(1, 6))
dec = dict((str(val), []) for val in range(1, 6)) 
ncombine = dict((str(val), []) for val in range(1, 6)) 
image_files = dict((str(val), []) for val in range(5))

class Plotting:
    def __init__(self, y):
        self.y = y
        self.min_list = []
        self.max_list = []
        for each_list in y:
            self.min_list.append(np.min(each_list))
            self.max_list.append(np.max(each_list))
        self.a_min = np.min(self.min_list)
        self.a_max = np.max(self.max_list)

def bad_graphing(x, y, z):
    colors = ['blue', 'orange', 'green', 'red']
    labels = ['artifacts', 'rippling', 'dark spots', 'flat correction error'] 
    styles = ['/', '*', '|', '+']
    
    fig, axs = plt.subplots(3, figsize = (8, 28))    
    object1 = Plotting(y)
    
    axs[0].hist(x, bins = 20, histtype = 'step', color = colors, align = 'mid', label = labels, linewidth = 2, fill = True, alpha = 0.4)
    axs[0].legend(loc = 'upper right')
    
    axs[0].xaxis.set_major_locator(ticker.MaxNLocator(integer = True))
    axs[0].xaxis.set_major_formatter(ticker.FormatStrFormatter('%.1f'))
    axs[0].set_title('The Range of Right Ascensions of "Bad" Images', fontsize = 14) 
    axs[0].set_xlabel("Right Ascension from 0° to 360°", fontsize = 12, labelpad = 10)
    
    axs[1].hist(y, bins = np.arange(object1.a_min - 1, object1.a_max + 1, 0.5), histtype = 'step', stacked = True,color = colors, align = 'mid', label = labels, linewidth = 2, fill = True, alpha = 0.4, hatch = "//")
    axs[1].legend(loc = 'upper right')
    axs[1].set_title('The Range of Declinations of "Bad" Images', fontsize = 14)
    axs[1].set_xlabel("Declination from -4° to 4°", fontsize = 12, labelpad = 5)

    axs[2].hist(z, bins = 'auto', histtype = 'step', stacked = True, color = colors, align = 'mid', label = labels, linewidth = 2, fill = True, alpha = 0.4)
    axs[2].legend(loc = 'upper right')
    axs[2].title.set_text('Number of Images Compiled for each Image')
    axs[2].set_visible(False)
    
    for i in range(3):
        axs[i].set_ylabel("Number of Images", fontsize = 12)
    
    plt.show()

def bad_plotting(lists):
    r = []
    d = []
    n = []
    for index, each_list in enumerate(lists):
        for i in range(len(each_list)):
            #this added_image only works on Mac
            added_image = '/Users/joshuaahn/Desktop/SAGUARO Data/' + str(each_list[i]) + '_wcs.fits.fz'
            #this added_image only works on Windows
            #added_image = 'C:\\Users\\joshu\Desktop\SAGUARO Data\\' + str(each_list[i]) + '_wcs.fits.fz'
            image_files[str(index)].append(added_image)
            header = fits.getheader(image_files[str(index)][i], ext = 1)
            
            hour_angle = float(header['HA'][0] + "1") * (int(header['HA'][1:3]) + int(header['HA'][4:6])/60 + int(header['HA'][7:])/3600)
            new_hour_angle = round(hour_angle, 2)
            
            ra[str(index + 1)].append(header['CRVAL1'])
            dec[str(index + 1)].append(header['CRVAL2'])
            ncombine[str(index + 1)].append(header['NCOMBINE'])
            
        r.append(ra[str(index + 1)]) 
        d.append(dec[str(index + 1)])
        n.append(ncombine[str(index + 1)])
    bad_graphing(r, d, n)

bad_plotting((artifacts, rippling, dark_spots, flat_correction_error))

def good_graphing(x, y):
    #x = RA of good images, y = DEC of good images
    left, width = 0.1, 0.65
    bottom, height = 0.1, 0.65
    spacing = 0.005
    
    rect_scatter = [left, bottom, width, height]
    rect_histx = [left, bottom + height + spacing, width, 0.2]
    rect_histy = [left + width + spacing, bottom, 0.2, height]
    
    plt.figure(figsize = (8, 8))
    ax_scatter = plt.axes(rect_scatter)
    ax_Histx = plt.axes(rect_histx)
    ax_Histy = plt.axes(rect_histy)
    
    ax_scatter.scatter(x,y,color='k',alpha=0.1,label='data label')
    ax_scatter.vlines(np.median(x),-1.5,1.5,color='k',linestyle='-')
    ax_scatter.hlines(np.median(y),-1.5,1.5,color='k',linestyle='-')
    
    ax_Histx.hist(x, color='k', bins=200, alpha=0.5, histtype='step', label='std = %.3f'%(np.std(x)), density = False)
    ax_Histy.hist(y, color='k', bins=200, orientation='horizontal', alpha=0.5, histtype='step', label='std = %.3f'%(np.std(y)), density = False)
    ax_Histx.set_xlim(ax_scatter.get_xlim())
    ax_Histy.set_ylim(ax_scatter.get_ylim())

    plt.show()

def good_plotting(good_images_list):
    r = []
    d = []
    for each_image in good_images_list:
        #this added_image only works on Mac
        added_image = '/Users/joshuaahn/Desktop/SAGUARO Data/' + str(each_image) + '_wcs.fits.fz'
        #this added_image only works on Windows
        #added_image = 'C:\\Users\\joshu\Desktop\SAGUARO Data\\' + str(each_image) + '_wcs.fits.fz'
        header = fits.getheader(added_image, ext = 1)
        
        hour_angle = float(header['HA'][0] + "1") * (int(header['HA'][1:3]) + int(header['HA'][4:6])/60 + int(header['HA'][7:])/3600)
        new_hour_angle = round(hour_angle, 2)
        
        r.append(header['CRVAL1'])
        d.append(header['CRVAL2'])
    
    good_graphing(r, d)

good_plotting(good_images)

#S00190, S00164 are great images to demonstrate a "bad image"
plt.figure(figsize = (14, 14))

plt.imshow(fits.getdata('/Users/joshuaahn/Desktop/SAGUARO Data/' + 'S00164_wcs.fits.fz'), norm = ImageNormalize(fits.getdata('/Users/joshuaahn/Desktop/' + 'SAGUARO Data/S00164_wcs.fits.fz'), interval = ZScaleInterval()), cmap = 'gray')


def percentage_function(x, y):
    bad_n = x['Name'][x['Good Image '] == 'False ']
    bad_s = y['Name'][y['Good Image '] == 'False ']
    good_n = x['Name'][x['Good Image '] == 'True ']
    good_s = y['Name'][y['Good Image '] == 'True ']
    all_n = x['Name']
    all_s = y['Name']
    print("The percentage of bad S images is " + str(len(bad_s)/len(all_s)) + ".")
    print("The percentage of bad N images is " + str(len(bad_n)/len(all_n)) + ".")
    print("The percentage of good S images is " + str(len(good_s)/len(all_s)) + ".")
    print("The percentage of good N images is " + str(len(good_n)/len(all_n)) + ".")

percentage_function(dfN, dfS)

more_info = df['More Information ']
def access_more_info(name):
    index = np.where(df['Name'] == name)[0][0]
    print(more_info[index])
    
access_more_info("N03040")
