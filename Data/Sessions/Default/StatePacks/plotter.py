from matplotlib import pylab as plt
import mpl_toolkits.mplot3d.axes3d as p3

filepath = "./test1.txt"

def loadFile(filename):

	fp = open(filepath);

	yaw = []
	pitch = []
	roll= []

	x = []
	y = []
	z = []

	for line in fp:
		vals = line.split('\t')
		yaw.append(float(vals[0]))
		pitch.append(float(vals[1]))
		roll.append(float(vals[2]))
		x.append(float(vals[3]))
		y.append(float(vals[4]))
		z.append(float(vals[5]))

	return yaw, pitch, roll, x, y, z


def run():

	yaw, pitch, roll, x, y, z = loadFile(filepath)

	plt.plot(x, 'ro')
	plt.plot(y, 'bo')
	plt.plot(z, 'go')

	plt.xlabel("Frame")
	plt.ylabel("Position (mm)")

	plt.legend(["X", "Y", "Z"])

	plt.title("Centroid Position")

	plt.show()

	plt.plot(yaw, 'ro')
	plt.plot(pitch, 'bo')
	plt.plot(roll, 'go')

	plt.xlabel("Frame")
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




