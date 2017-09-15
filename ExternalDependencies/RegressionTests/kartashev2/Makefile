CCC=/set/mars_patch/dist/intel-S2/bin//CC -m64
CCC=g++ -m64

all:
	$(CCC) -c -g foo.cc bar.cc
	ld -r -o combined.o foo.o bar.o
	# dwarfdump combined.o
