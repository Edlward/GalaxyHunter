add_custom_target(docker_image_ubuntu docker build -t galaxy_hunter_ubuntu_build Ubuntu-15.04 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
add_custom_target(docker_create_container_ubuntu COMMAND docker create -v ${CMAKE_SOURCE_DIR}:/GalaxyHunter:ro -v ${CMAKE_CURRENT_BINARY_DIR}:/GalaxyHunter-dist --name galaxy_hunter_ubuntu galaxy_hunter_ubuntu_build DEPENDS docker_image_ubuntu)
add_custom_target(docker_build_ubuntu COMMAND docker start -a -i galaxy_hunter_ubuntu)
add_custom_target(docker_clean_ubuntu docker rm galaxy_hunter_ubuntu )