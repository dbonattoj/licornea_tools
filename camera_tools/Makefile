CXXFLAGS := -std=c++14 -I../external/include/

LIB_SRC := $(wildcard lib/*.cc)

all : export_mpeg import_mpeg transform visualize

export_mpeg : export_mpeg.cc $(LIB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

import_mpeg : import_mpeg.cc $(LIB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

transform : transform.cc $(LIB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

visualize : visualize.cc $(LIB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f export_mpeg import_mpeg transform visualize

