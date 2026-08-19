cmake_minimum_required(VERSION 3.7)

project( M6502Lib )

if(MSVC)
	add_compile_options(/MP)				
	add_compile_options(/W4 /wd4201 /WX)	
else()
	add_compile_options(-W -Wall -Werror) 
endif()

set  (M6502_SOURCES
    "main-files/public/m6502.h"
	"main-files/private/m6502.cpp"
    "main-files/private/main_6502.cpp")
		
source_group("src" FILES ${M6502_SOURCES})
		
add_library( M6502Lib ${M6502_SOURCES} )

target_include_directories ( M6502Lib PUBLIC "${PROJECT_SOURCE_DIR}/main-files/public")
target_include_directories ( M6502Lib PRIVATE "${PROJECT_SOURCE_DIR}/main-files/private")

