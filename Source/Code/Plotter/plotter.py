#!/usr/bin/python

from matplotlib import pylab as plt
import mpl_toolkits.mplot3d.axes3d as p3
import sys

print "starting"

def loadFile(filepath):

	fp = open(filepath);

	timestamps = []

	yaw = []
	pitch = []
	roll= []

	x = []
	y = []
	z = []

	for line in fp:
		vals = line.split('\t')
		timestamps.append(float(vals[0]))
		yaw.append(float(vals[1]))
		pitch.append(float(vals[2]))
		roll.append(float(vals[3]))
		x.append(float(vals[4]))
		y.append(float(vals[5]))
		z.append(float(vals[6]))

	return timestamps, yaw, pitch, roll, x, y, z

if len(sys.argv) < 2:
	print "insufficient arguments..."
else:
	filepath = sys.argv[1]

	timestamps, yaw, pitch, roll, x, y, z = loadFile(filepath)

	plt.plot(timestamps, x, 'ro')
	plt.plot(timestamps, y, 'bo')
	plt.plot(timestamps, z, 'go')

	plt.xlabel("Time (ms)")
	plt.ylabel("Position (mm)")

	plt.legend(["X", "Y", "Z"])

	plt.title("Centroid Position")

	plt.show()

	plt.plot(timestamps, yaw, 'ro')
	plt.plot(timestamps, pitch, 'bo')
	plt.plot(timestamps, roll, 'go')

	plt.xlabel("Time (ms)")
	plt.ylabel("Angle (rad)")

	plt.legend(["yaw", "pitch", "roll"])

	plt.title("Flight Attitude")

	plt.show()

	fig = plt.figure()
	ax = p3.Axes3D(fig)
	ax.scatter3D(x, y, z, 'bo')
	ax.set_xlabel('X (mm)')
	ax.set_ylabel('Y (mm)')
	ax.set_zlabel('Z (mm)')
	fig.add_axes(ax)
	plt.title("3d Trajectory")
	plt.show()




