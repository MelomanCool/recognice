import subprocess
import os.path
from glob import glob

from make_config import GNSDK_HOME


def make(user_sources):
    GNSDK_CPP_HOME = GNSDK_HOME + "/wrappers/gnsdk_cplusplus"

    SOURCES = (glob(GNSDK_CPP_HOME + "/src_wrapper/*.cpp")
               + glob(GNSDK_CPP_HOME + "/src_helpers/*.cpp"))

    INCLUDES = """-I{gncpp}/src_wrapper \
                  -I{gncpp}/src_helpers \
                  -I{gncpp}/src_wrapper/linux \
                  -I{gncpp}/src_helpers/linux \
                  -I{gn}/include \
                  -I{gn}/include/linux_x86-64""".format(gn=GNSDK_HOME,
                                                        gncpp=GNSDK_CPP_HOME)

    sdkdepends = ["musicid_stream",
                  "dsp",
                  "manager"]

    LIBS = list(map(lambda x: "libgnsdk_{}.so.3.10.5".format(x),
                        sdkdepends))
    LIBS_S = " ".join(LIBS)
    LIB_DEFINES = " ".join(map(lambda x: "-DGNSDK_{}=1".format(x.upper()),
                              sdkdepends))

    subprocess.check_call(["mkdir", "--parents", "_output"])

    for s in SOURCES + user_sources:
        sss = os.path.basename(s)
        subprocess.check_call(
             """g++ -w \
                    -g \
                    -m64 \
                    -fPIC \
                    -funsigned-char \
                    -D_DEBUG \
                    {LIB_DEFINES} \
                    -D_THREAD_SAFE \
                    -D_REENTRANT \
                    -U_FORTIFY_SOURCE \
                    -D_FORTIFY_SOURCE=0 \
                    -std=c++11 \
                    {INCLUDES} \
                    $(python3 -m pybind11 --includes) \
                    -c {s} \
                    -o _output/{sss}.o""".format(s=s,
                                                 sss=sss,
                                                 INCLUDES=INCLUDES,
                                                 LIB_DEFINES=LIB_DEFINES),
            shell=True)

    for lib in LIBS:
        subprocess.check_call("cp -fp {gn}/lib/linux_x86-64/{l} .".format(l=lib, gn=GNSDK_HOME).split())

    subprocess.check_call(
         """g++ -w \
                -m64 \
                -Wl,-rpath,'$ORIGIN' \
                -ogn.so \
                _output/*.cpp.o \
                -shared \
                {LIBS_S} \
                -lpthread \
                -ldl \
                -lrt \
                -lm""".format(LIBS_S=LIBS_S),
        shell=True)


if __name__ == "__main__":
    make(user_sources=["src/main.cpp"])
