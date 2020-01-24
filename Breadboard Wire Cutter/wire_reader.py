'''
Wire Reader v1.0
Created by Ben Boggs: bennyboggs@gmail.com

Command line inputs are 2 svgs: one of the wires only, one of the breadboard only.

python3 wire_reader.py wires_only.svg breadboard_only.svg

This program was designed to work with fritzing: http://fritzing.org/home/
From fritzing, go to View>Hide All Layers, then View>Wires Layer to show only wires,
or View>Breadboard Layer to show only the breadboard.
Export both the wires-only view and the breadboard-only view (with grid lines off) as
SVG files.

The terminal output is a list of wire lengths in breadboard units (dist between adjacent holes)
'''

import sys
import numpy as np

# helper function to parse numbers
def getnum(file, ind, c = "'"):
	i = ind
	while file[i] != c:
		i+=1
	i+=1
	j = i
	while file[j] != c:
		j+=1
	return [(float)(file[i:j]), j+1]


if (len(sys.argv) < 3):
	print("Not enough files found! Please provide 2 SVG files as command line arguments.")


# determine wire lengths
f = open(sys.argv[1], "r")
f = f.read()
lengths = []

for i in range(len(f)-1):
	if f[i] + f[i+1] == "x1":
		x1, i = getnum(f, i)
		y1, i = getnum(f, i)
		x2, i = getnum(f, i)
		y2, i = getnum(f, i)
		lengths.append(np.sqrt((x2-x1)**2 + (y2-y1)**2))

# fritzing wires have outlines which result in duplicate coordinates
lengths = [lengths[2*i] for i in range(int(len(lengths)/2))]

# determine breadboard unit length
bb = open(sys.argv[2], "r")
bb = bb.read()

i = 0
x = []
y = []
for q in range(2):
	while bb[i] +bb[i+1] != "cx":
		i += 1
		if i == len(bb):
			print("Invalid file! Ending...")
			break
	x1, i = getnum(bb, i, "\"")
	y1, i = getnum(bb, i, "\"") 
	x.append(x1)
	y.append(y1)
	if q == 1 and x1 != x[0] and y1 != y[0]:
		x[0] = x1
		y[0] = y1
		del x[1]
		del y[1]
		q -= 1

if x[0] == x[1]:
	bb_unit = abs(y[1]-y[0])
else:
	bb_unit = abs(x[1]-x[0])

lengths = [np.round(lengths[i]/bb_unit, 4) for i in range(len(lengths))]
s = ""
for i in range(len(lengths)-1):
	s += str(lengths[i]) + " "
print(s + str(lengths[-1]))


		

