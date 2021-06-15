AddTarget(
	NAME PlayerParser
	TYPE app_console
	SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/PlayerParser/src"
	INCLUDE_DIRS
		"${CMAKE_CURRENT_LIST_DIR}/../ext"
		"${CMAKE_CURRENT_LIST_DIR}/../ext/fmt/include"
	LINK_TARGETS
		fmt
	COMPILER_OPTIONS
		[ MSVC /WX /Wall ]
	STATIC_RUNTIME
)
