include_directories ("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")

add_executable (1807.02079.TDep 1807.02079.TDep.cpp)
add_executable (1807.02079.Fits 1807.02079.Fits.cpp)

target_link_libraries (1807.02079.TDep ThermalFIST)
target_link_libraries (1807.02079.Fits ThermalFIST)

set_property(TARGET 1807.02079.TDep PROPERTY FOLDER "1807.02079")
set_property(TARGET 1807.02079.Fits PROPERTY FOLDER "1807.02079")

install (TARGETS 1807.02079.TDep
         RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)
install (TARGETS 1807.02079.Fits
         RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)		 
		 
