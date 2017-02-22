CXXFLAGS := -std=c++14 -Iinclude/

TARGET := camera_transform
SRC := camera_transform.cc

all : $(TARGET)
	

$(TARGET) : $(SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)
