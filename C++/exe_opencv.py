import os
import sys

os.system("sudo modprobe bcm2835-v4l2")
os.system("sudo g++ $(pkg-config --libs --cflags opencv) -o " 
	      + sys.argv[1] + " " + sys.argv[1] + ".cpp" + " -lpthread")
os.system("sudo ./" + sys.argv[1])
