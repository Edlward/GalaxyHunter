add_custom_target(objects_database ALL COMMAND unxz -k -c ${CMAKE_CURRENT_SOURCE_DIR}/objects.db.xz > ${CMAKE_CURRENT_BINARY_DIR}/objects.db SOURCES objects.db.xz)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/objects.db DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES dslr-qt-shooter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
add_subdirectory(icons)
