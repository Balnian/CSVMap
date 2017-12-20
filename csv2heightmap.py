import csv
import math
import png
import argparse
import os

# Print iterations progress
def printProgressBar (iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█'):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = '\r')
    # Print New Line on Complete
    if iteration == total: 
        print()

#Command line arguments for csv2heightmap
parser = argparse.ArgumentParser(description='Convert CSV point data to a PNG grayscale heightmap')
parser.add_argument('CSV_file', metavar = 'file', type=argparse.FileType('r'), help='CSV file to convert')
parser.add_argument('-o','--out', dest= 'outfile', type=argparse.FileType('wb'), help='output file for the heightmap' )
args = parser.parse_args()

csvfile = args.CSV_file
outfile = open(os.path.splitext(csvfile.name)[0]+".png", 'wb') if args.outfile is None else args.outfile

#Read CSV and create 2d array of height values
arr = []
currYval=0
y=-1
reader = csv.DictReader(csvfile)
#rows = list(reader)
#totalrows = len(rows)
#printProgressBar(0, totalrows, prefix = 'Progress:', suffix = 'Complete', length = 50)
#for i, row in enumerate(rows):
for row in reader:
    #printProgressBar(i+1, totalrows, prefix = 'Progress:', suffix = 'Complete', length = 50)
    if currYval != row["Y"]:
        currYval = row["Y"]
        arr.append([])
        y+=1
    arr[y].append(float(row["Z"]))

csvfile.close()


#Transform Z height info into a 0-255 value
minimum = min([min(z) for z in arr])
maxi = max([max(z) for z in arr])

stepsize = (maxi-minimum)/65535
#print(str(stepsize))
#print(str(minimum))
#print(str(maxi))
arr = [ [math.floor((z-minimum)/stepsize) for z in row] for row in arr]
#count =0
#for row in arr:
 #   for val in row:
  #      count += val
#print(str(count))

#Write PNG
w = png.Writer(len(arr[0]),len(arr), bitdepth=16, greyscale=True)
w.write(outfile,arr)
outfile.close()

