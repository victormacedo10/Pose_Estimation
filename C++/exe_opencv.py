import os
import sys

os.system("g++ $(pkg-config --libs --cflags opencv) -o " 
	      + sys.argv[1] + " " + sys.argv[1] + ".cpp" + " -lpthread")
