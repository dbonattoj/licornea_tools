CXXFLAGS := -std=c++14 -Iinclude/

TARGET := extrinsic_convert
SRC := extrinsic_convert.cc

all : $(TARGET)
	

$(TARGET) : $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
