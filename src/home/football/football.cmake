AddTarget(${PROJECT_NAME} app
	PROJECT_GROUP         App
	SOURCE_DIRECTORY      ${CMAKE_CURRENT_LIST_DIR}
	LINK_LIBRARIES
		Boost::headers
		Qt${QT_MAJOR_VERSION}::Core
		Qt${QT_MAJOR_VERSION}::Gui
		Qt${QT_MAJOR_VERSION}::Widgets
	LINK_TARGETS
		logging
		settings
		utilgui
)
