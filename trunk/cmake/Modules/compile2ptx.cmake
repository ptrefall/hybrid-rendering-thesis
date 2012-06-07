
macro(COMPILE_CU2PTX)
	message(STATUS "Compile optix files...")
	set(PTX_EXT ".ptx")
	#set(RUN_IF_NEWER_EXEC ${CMAKE_SOURCE_DIR}/bin/run_if_newer.exe)
	set(RUN_IF_NEWER_EXEC ${CMAKE_SOURCE_DIR}/bin/run_if_newer.exe)
	foreach(CU ${ARGV}	)
		message(STATUS ${CU})
		set(PTX ${CU}${PTX_EXT})
		add_custom_command(	TARGET ${NAME}
							PRE_BUILD 
							COMMAND ${RUN_IF_NEWER_EXEC} ${CU} ${PTX} "nvcc.exe" ${CU} ARGS --machine 32 --ptx --include-path ${CUDA_INCLUDE_PATH} --include-path ${OPTIX_INCLUDE_PATH} --output-file ${PTX}
							COMMENT "Compiling Optix script ${CU} to ${PTX}")
	endforeach()
endmacro()