CXXFLAGS := -std=c++14 -I../external/include/

all : transform visualize
	

transform : transform.cc lib/camera.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

visualize : visualize.cc lib/camera.cc
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f transform visualize

