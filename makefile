CXX = $(shell fltk-config --cxx)
CXXFLAGS = $(shell fltk-config --use-images --use-gl --cxxflags) -Og
LDFLAGS = $(shell fltk-config --use-images --use-gl --ldstaticflags)
main:
	$(CXX) $(CXXFLAGS) -g worship-attempt.cc -o worship-attempt.exe $(LDFLAGS)
	./worship-attempt