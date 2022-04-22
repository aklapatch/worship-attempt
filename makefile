CXX = $(shell fltk-config --cxx)
CXXFLAGS = $(shell fltk-config --cflags --use-images --use-gl)
#CXXFLAGS += -std=c++11
main:
	fltk-config -g --use-images --use-gl --compile worship-attempt.cc 
	./worship-attempt