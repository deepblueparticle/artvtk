# basic source code CMakeLists.txt

WARNING - ADDING A FILE TO THIS DIRECTORY WILL NOT TRIGGER
# A REBUILD (since it doesn't change the CMakeLists.txt file).

# install all *.fcl files in this directory
file(GLOB rt_files *.py)
install( FILES ${rt_files}
DESTINATION ${product}/${version}/pipelines )

# Also install in the build area
foreach(aFile ${rt_files})
get_filename_component( basename ${aFile} NAME )
configure_file( ${aFile} ${CMAKE_BINARY_DIR}/${product}/pipelines/${basename} COPYONLY )
endforeach(aFile)