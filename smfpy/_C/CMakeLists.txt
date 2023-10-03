add_subdirectory(client_app)

set(INSTALL_DIR "${PROJECT_BINARY_DIR}")
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/client_app" DESTINATION "${INSTALL_DIR}")