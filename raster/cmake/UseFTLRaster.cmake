FIND_PACKAGE(FREETYPE REQUIRED)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
INCLUDE_DIRECTORIES(${FTLRASTER_INCLUDE_DIR})
LINK_LIBRARIES(${FTLRASTER_LIBRARY})
INCLUDE(BuildOptionsFTLRaster)
