include Makefile.config

obj-dir = build

gn-deps = manager dsp musicid_stream
gn-ver = 3.10.5

gn-cpp = $(gn)/wrappers/gnsdk_cplusplus

gn-includes = -I$(gn-cpp)/src_wrapper \
              -I$(gn)/include         \
              -I$(gn)/include/$(platform)

gn-libs = $(patsubst %,libgnsdk_%.so.$(gn-ver),$(gn-deps))
gn-defs = $(patsubst %,           \
                     -DGNSDK_%=1, \
                     $(shell echo $(gn-deps) | tr a-z A-Z))

gn-src-dir = $(gn-cpp)/src_wrapper
gn-srcs = $(wildcard $(gn-src-dir)/*.cpp)
gn-objs = $(patsubst $(gn-src-dir)/%.cpp,$(obj-dir)/%.o,$(gn-srcs))


all: gn.so


gn.so: $(gn-objs) $(obj-dir)/main.o $(gn-libs)
	@echo Building $@
	       # search for gnsdk lib in current dir
	$(CXX) -Wl,-rpath,'$$ORIGIN'        \
	       $(gn-objs) $(obj-dir)/main.o \
	       -shared $(gn-libs)           \
	       -o gn.so


$(obj-dir)/main.o: src/main.cpp
	@echo Building $@
	$(CXX) -std=c++11 -fPIC                        \
	       $(shell python3 -m pybind11 --includes) \
	       $(gn-defs) $(gn-includes)               \
	       -c $< -o $@


libgnsdk_%.so.$(gn-ver):
	@echo Copying $@ to current dir
	cp $(gn)/lib/$(platform)/$@ .


$(obj-dir)/%.o: $(gn-src-dir)/%.cpp | $(obj-dir)
	@echo Building $@
	$(CXX) -std=c++11 -fPIC                                 \
	       $(gn-suppress-warns) $(gn-defs) $(gn-includes) \
	       -c $< -o $@


$(obj-dir):
	mkdir $@


clean:
	@echo Removing: build/ gn.so $(gn-libs)
	rm -rf build
	rm -f gn.so
	rm -f $(gn-libs)


$(V).SILENT:
