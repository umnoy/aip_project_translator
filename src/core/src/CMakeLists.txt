add_subdirectory(tests)
find_library(ONNXRUNTIME_LIBRARY
    NAMES onnxruntime libonnxruntime
    PATHS ${ONNXRUNTIME_LIB_DIR}
    NO_DEFAULT_PATH
)
if(NOT ONNXRUNTIME_LIBRARY)
    message(FATAL_ERROR "ONNX Runtime library not found in ${ONNXRUNTIME_LIB_DIR}")
endif()