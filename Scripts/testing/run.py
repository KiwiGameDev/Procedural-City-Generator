
# import the necessary packages
from re import S
from cv2 import COLOR_BGR2HSV, COLOR_HSV2BGR, bitwise_not
from skimage.metrics import structural_similarity
import argparse
import imutils
import cv2
import numpy as np
import csv
import os
from typing import NamedTuple

class RESULT(NamedTuple):
	fileName : str
	ssimScore: float
	avePercentage: float
	pixelCountString: list
	pixelPercentString: list
	id: int
	difference: float
	finalScore: float

def run(path_to_imageA, path_to_imageB, arr , indx):

	if os.path.exists(path_to_imageA) == False or os.path.exists(path_to_imageB) == False:
		return


	image_a_split = path_to_imageA.split('images2/true/')
	print(image_a_split[1])

	image_b_split = path_to_imageB.split('images2/unreal/')
	print(image_b_split[1])
	final_label = f'{image_a_split[1]} vs {image_b_split[1]}'
	print(final_label)
#		myFile.writerow([final_label])

	

	# construct the argument parse and parse the arguments
	#ap = argparse.ArgumentParser()
	#ap.add_argument("-f", "--first", required=True,
		#help="first input image")
	#ap.add_argument("-s", "--second", required=True,
		#help="second")
	#args = vars(ap.parse_args())

	# load the two input images
	#imageA = cv2.imread(args["first"])
	#imageB = cv2.imread(args["second"])
	imageA = cv2.imread(path_to_imageA)
	imageB = cv2.imread(path_to_imageB) #//norm
#	imageB = cv2.imread('images/white.png') #worst
#	imageB = imageA #best
#	white_image = cv2.imread('images/white.png')

	#get the unique colors of both images and store in an array

	#flatten 3d array to 2d
	bgrA = imageA.reshape(-1, imageA.shape[-1])
	unique_bgrA = np.unique(bgrA, axis=0)
	#print(unique_bgrA)

	bgrB = imageB.reshape(-1, imageB.shape[-1])
	unique_bgrB = np.unique(bgrB, axis=0)
	#print(unique_bgrB)

	#intesersection
	setA = set([tuple(x) for x in unique_bgrA])
	setB = set([tuple(x) for x in unique_bgrB])
	intersection = np.array([x for x in setA & setB])
	#print(intersection)
	#print(len(intersection))

	index = 0
	length = len(intersection)-1

	for i in range(length):
		#print (index, intersection[i])
		maskCurr = cv2.inRange(imageA, intersection[i], intersection[i])
		maskNext = cv2.inRange(imageA, intersection[i+1], intersection[i+1])
		index += 1
		if i == 0:
			finalmask = maskCurr + maskNext
		else:
				finalmask = maskCurr + maskNext + finalmask
		

	index = 0	
	for i in range(length):
		maskCurrB = cv2.inRange(imageB, intersection[i], intersection[i])
		maskNextB = cv2.inRange(imageB, intersection[i+1], intersection[i+1])
		index += 1
		if i == 0:
			finalmaskB = maskCurrB + maskNextB
		else:
				finalmaskB = maskCurrB + maskNextB + finalmaskB
		
	if(length == -1):
		white = cv2.imread('images/white.png')
		finalmask = cv2.bitwise_not(white)
		finalmaskB = finalmask

	#mask combine the mask with the original image
	if(length != -1):
		final_image_A = cv2.bitwise_and(imageA, imageA, mask=finalmask)
	else:
		final_image_A = imageA
#	cv2.imshow("FinalOutputA", final_image_A) #important

	if(length != -1):
		final_image_B = cv2.bitwise_and(imageB, imageB, mask=finalmaskB)
	else:
		final_image_B = imageB
#	cv2.imshow("FinalOutputB", final_image_B) #important

#	cv2.imshow("maskedA", finalmask) #important
#	cv2.imshow("maskedB", finalmaskB) #important

	#count the masked pixels
	finalMaskACount = np.count_nonzero(finalmask == 0) #final 
	finalMaskBCount = np.count_nonzero(finalmaskB == 0) #final

	

	print('FinalMaskBCount: '+ str(finalMaskBCount) + '    FinalMaskACount:   ' + str(finalMaskACount))

	# convert the images to grayscale
	grayA = cv2.cvtColor(final_image_A, cv2.COLOR_BGR2GRAY)
	grayB = cv2.cvtColor(final_image_B, cv2.COLOR_BGR2GRAY)

	# compute the Structural Similarity Index (SSIM) between the two
	# images, ensuring that the difference image is returned

	#(score, diff) = compare_ssim(grayA, grayB, full=True)
	(score, diff) = structural_similarity(grayA, grayB, full=True)
	diff = (diff * 255).astype("uint8")
	formatted_score = float("{:0.4f}".format(score))
	#print(f"SSIM: {formatted_score}")
	
#		myFile.writerow(["SSIM: ", formatted_score])


	# threshold the difference image, followed by finding contours to
	# obtain the regions of the two input images that differ
	thresh = cv2.threshold(diff, 0, 255,
		cv2.THRESH_BINARY_INV | cv2.THRESH_OTSU)[1]
	cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)
	cnts = cnts[0] if imutils.is_cv2() else cnts[1]

	#pixel counting
	#abs(true - unreal) / all_pixels #1280*720

	highrise = np.array([0,186,255])
	lowrise = np.array([0,255,255])
	land = np.array([123,6,136])
	foliage = np.array([0,255,0])
	poles = np.array([255,0,0])
	road = np.array([255,0,255])
	black = np.array([0,0,0])

	total_pixel_count = 1280 * 720

	highrise_pixels_A = np.count_nonzero(np.all(final_image_A==highrise, axis=2))

	lowrise_pixels_A = np.count_nonzero(np.all(final_image_A==lowrise, axis=2))

	land_pixels_A = np.count_nonzero(np.all(final_image_A==land, axis=2))

	foliage_pixels_A = np.count_nonzero(np.all(final_image_A==foliage, axis=2))

	poles_pixels_A = np.count_nonzero(np.all(final_image_A==poles, axis=2))

	road_pixels_A = np.count_nonzero(np.all(final_image_A==road, axis=2))

	#final_image_A_mask_pixelCount = np.count_nonzero(np.all(finalmask==black, axis=2))

	total_percent = 0
	total_num_count = 0
	total_list = list()
	total_percent_list = list()
	final_score = 0

	highrise_pixels_B = np.count_nonzero(np.all(final_image_B==highrise, axis=2))
	temp = (1- (np.absolute(highrise_pixels_A - highrise_pixels_B)) / total_pixel_count) * 100 
	if highrise_pixels_B != 0:
		pixels = f"HighRise: {highrise_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(f"Highrise: {highrise_pixels_B}px - {temp : .4f}%")
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
#			myFile.writerow([pixels,percent])


	lowrise_pixels_B = np.count_nonzero(np.all(final_image_B==lowrise, axis=2))
	temp =  (1- (np.absolute(lowrise_pixels_A - lowrise_pixels_B)) / total_pixel_count) * 100
	if lowrise_pixels_B != 0:
		pixels = f"LowRise: {lowrise_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(pixels + " " + percent)
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
#			myFile.writerow([pixels,percent])

	land_pixels_B = np.count_nonzero(np.all(final_image_B==land, axis=2))
	temp = (1- (np.absolute(land_pixels_A - land_pixels_B)) / total_pixel_count) * 100
	if land_pixels_B != 0:
		pixels = f"Land/Sidewalk: {land_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(pixels + " " + percent)
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
		#total_list.append(pixels + " " + percent + '\n')
#			myFile.writerow([pixels,percent])

	foliage_pixels_B = np.count_nonzero(np.all(final_image_B==foliage, axis=2))
	temp = (1- (np.absolute(foliage_pixels_A - foliage_pixels_B)) / total_pixel_count) * 100
	if foliage_pixels_B != 0:
		pixels = f"Foliage/Details: {foliage_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(pixels + " " + percent)
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
#			myFile.writerow([pixels,percent])

	poles_pixels_B = np.count_nonzero(np.all(final_image_B==poles, axis=2))
	temp = (1- (np.absolute(poles_pixels_A - poles_pixels_B)) / total_pixel_count) * 100
	if poles_pixels_B != 0:
		pixels = f"Poles: {poles_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(pixels + " " + percent)
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
#			myFile.writerow([pixels,percent])

	road_pixels_B = np.count_nonzero(np.all(final_image_B==road, axis=2))
	temp = (1- (np.absolute(road_pixels_A - road_pixels_B)) / total_pixel_count) * 100
	if road_pixels_B != 0:
		pixels = f"Road: {road_pixels_B}px"
		percent = f"Percentage: {temp: .4f}%"
		#print(pixels + " " + percent)
		total_num_count += 1
		total_percent += temp
		total_list.append(pixels)
		total_percent_list.append(percent)
#			myFile.writerow([pixels,percent])

	if total_num_count == 0:
			ave_percentage = 0
			diff_percentage = (((finalMaskACount/total_pixel_count) + (finalMaskBCount/total_pixel_count)))*100
			final_score = ave_percentage - diff_percentage
	else:
		ave_percentage = total_percent / total_num_count
		#print('Intersection Percent: ' + str(ave_percentage))
		#diff_percentage = (((finalMaskBCount/ total_pixel_count) + (finalMaskACount/ total_pixel_count))) *100
		diff_percentage = (((finalMaskACount/total_pixel_count) + (finalMaskBCount/total_pixel_count)))*100
		print('Diff percentage: ' + str(diff_percentage))
		final_score = ave_percentage - diff_percentage
		print('Final score: ' + str(final_score))
	#get difference between the intersection average and difference
		

	add_me = RESULT(final_label, score, ave_percentage, total_list, total_percent_list, indx, finalMaskBCount, final_score)
	arr.append(add_me)

	
#	cv2.waitKey(0) #important

def run_multiple(start,end,skewed):
	last_num = end+1
	for i in range(start,last_num):
		

		path_to_A = 'images2/true/m_' + str(i) + '.png'
		path_to_A_split = 'm_' + str(i)

		path_to_B_text = 'images2/unreal/' + path_to_A_split + '/' + 'Seeds' + '.txt'

		seeds_array = list()
		with open(path_to_B_text) as file:
			for line in file:
				seed_string = 'Seed:' + line.rstrip()
				seeds_array.append(seed_string)
		
		
		resulting_tuple = list()
		for j in range (1, 440): #11, 231
			
			if skewed[0] == True:
				path_to_skewed = 'skewed/'
			else:
				path_to_skewed = 'unskewed/'
			
			path_to_B = 'images2/unreal/' + path_to_A_split + '/' + path_to_skewed + str(j) + '.png'
			run(path_to_A, path_to_B, resulting_tuple, j)
		
		resulting_tuple = sorted(resulting_tuple, key=lambda r: r[7], reverse=True)
		#sorted(resulting_tuple, key=lambda RESULT: RESULT[1])
		#print('=================')

		ave_highest = list()
		ave_lowest = list()

		h = getattr(resulting_tuple[0], 'finalScore')
		l = getattr(resulting_tuple[len(resulting_tuple) - 1],  'finalScore')
		ave_highest.append(h)
		ave_lowest.append(l)

		#get difference between the intersection average and difference
		
		#check if csv file exists, if it does, delete
		csv_skewed_filename = 'data_skewed.csv'
		csv_unskewed_filename = 'data_unskewed.csv'
		path_to_csv_skewed = 'images2/unreal/' + path_to_A_split + '/' + csv_skewed_filename
		path_to_csv_unskewed = 'images2/unreal/' + path_to_A_split + '/' + csv_unskewed_filename

		file = path_to_csv_skewed
		if(os.path.exists(file) and os.path.isfile(file) and skewed[0] == True):
			os.remove(file)
			#print("file deleted")
		
		
		file = path_to_csv_unskewed
		if(os.path.exists(file) and os.path.isfile(file) and skewed[0] == False):
			os.remove(file)
			#print("file deleted")

		#make files
		#f = open(path_to_csv_skewed, 'w')
		#f.close

		#f = open(path_to_csv_unskewed, 'w')
		#f.close

		if skewed[0] == True:
			fileWriter(path_to_csv_skewed, resulting_tuple, seeds_array, ave_highest, ave_lowest, skewed)
		else:
			fileWriter(path_to_csv_unskewed, resulting_tuple, seeds_array, ave_highest, ave_lowest, skewed)
		

		
			
def fileWriter(path_to_data, resulting_tuple, seeds_array, ave_highest, ave_lowest, skewed):
	#open file
		with open(path_to_data, 'a', newline='') as file: 
			myFile = csv.writer(file)
			for i in range(len(resulting_tuple)):
					header = getattr(resulting_tuple[i], 'fileName')
					myFile.writerow([header])

					score = getattr(resulting_tuple[i], 'ssimScore')
					formatted_score = float("{:0.4f}".format(score))
					score_str = "SSIM: " + str(formatted_score)
					myFile.writerow([score_str])	
					
					seed_index = 0
					temp = float((getattr(resulting_tuple[i], 'id'))-1)
					if skewed[0] == True:
						if temp % 2 == 0 and temp != 0:
							increase = temp/2
						elif temp%2 != 0 and temp > 1:
							increase = (temp-1) / 2
						else:
							increase = 0
						seed_index = (increase)
						myFile.writerow([seeds_array[int(seed_index)]])
					else:
						if temp % 2 == 0 and temp != 0:
							increase = i/2
						elif temp%2 != 0 and temp > 1:
							increase = (temp-1) / 2
						else:
							increase = 0
						seed_index = 125 + (increase)
						myFile.writerow([seeds_array[int(seed_index)]])

					l = getattr(resulting_tuple[i], 'pixelCountString')
					p = getattr(resulting_tuple[i], 'pixelPercentString')
					#myFile.writerow(l)
					for j in range(len(l)):
						s = l[j]
						d = p[j]
						myFile.writerow([s, d])
						#print(l[j])
						
							
					ave = getattr(resulting_tuple[i], 'avePercentage')
					formatted_ave = float("{:0.4f}".format(ave))
					ave_str = "Average: " + str(formatted_ave) + "%"
					myFile.writerow([ave_str])	

					diff = getattr(resulting_tuple[i], 'difference')
					#formatted_diff = float("{:0.4f}".format(diff))
					formatted_diff = diff
					diff_str = "Difference: " + str(formatted_diff) + "%"
					myFile.writerow([diff_str])	

					fin = getattr(resulting_tuple[i], 'finalScore')
					#formatted_fin = float("{:0.4f}".format(fin))
					#print('FINAL: ' + str(fin))
					formatted_fin = fin
					fin_str = "Final: " + str(formatted_fin) + "%"
					myFile.writerow([fin_str])	



					myFile.writerow(['\n'])		
					#print(i)
			
			ave_highest_percent = 0
			ave_lowest_percent = 0	
			total_percent = len(ave_highest)	
			for i in range(total_percent):
				ave_highest_percent += ave_highest[i]
				ave_lowest_percent += ave_lowest[i]

			ave_highest_percent/=total_percent	
			ave_lowest_percent/=total_percent
			ave_highest_percent = float("{:0.4f}".format(ave_highest_percent))
			ave_lowest_percent = float("{:0.4f}".format(ave_lowest_percent))
			fin_str_high = 'All Top Highest Percent Ave: ' + str(ave_highest_percent) + '%'
			fin_str_low = 'All Lowest Percent Ave: ' + str(ave_lowest_percent) + '%'
			myFile.writerow([fin_str_high, fin_str_low])






def run_verify(start,end,skewed):
	last_num = end+1
	for i in range(start,last_num):
		

		path_to_A = 'images2/true/m_' + str(i) + '.png'
		path_to_A_split = 'm_' + str(i)

		path_to_B_text = 'images2/unreal/' + path_to_A_split + '/' + 'Seeds' + '.txt'

		seeds_array = list()
		with open(path_to_B_text) as file:
			for line in file:
				seed_string = 'Seed:' + line.rstrip()
				seeds_array.append(seed_string)
		
		if len(seeds_array) < 440:
			print("seeds not found")
			return
		
		for j in range (1, 440): #11 #231
			
			if skewed[0] == True:
				path_to_skewed = 'skewed/'
			else:
				path_to_skewed = 'unskewed/'
			
			path_to_B = 'images2/unreal/' + path_to_A_split + '/' + path_to_skewed + str(j) + '.png'

			if(not os.path.exists(path_to_B) and not os.path.isfile(path_to_B)):
				print(path_to_B + ' does not exist')
				return
		
	print('Verification success')		
		
		
		
		

skewed = list()
#skewed.append(True)
#run_verify(3,3, skewed)
#run_multiple(3,3, skewed) #this will test the all the skewed
skewed[0] = False
#run_verify(6,6, skewed)
#run_multiple(6,6, skewed) #this will test all the unskewed


