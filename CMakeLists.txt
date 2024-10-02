cmake_minimum_required(VERSION 3.18.4)
project(pngimp)
include_directories(${PROJECT_SOURCE_DIR}/pngimp)

enable_testing()
add_executable(image_open ${PROJECT_SOURCE_DIR}/tests/image_open.cpp ${PROJECT_SOURCE_DIR}/pngimp/pngimp.cpp)

add_test(
	NAME test_basn2c08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basn2c08.png tests/basn2c08.raw
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basn2c08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basn2c08.png;tests/basn2c08.raw"
)

add_test(
	NAME test_basi2c08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basi2c08.png tests/basn2c08.raw
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basi2c08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basi2c08.png;tests/basn2c08.raw"
)

add_test(
	NAME test_basn6a08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basn6a08.png tests/basn6a08.raw
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basn6a08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basn6a08.png;tests/basn6a08.raw"
)

add_test(
	NAME test_basn2c16
	COMMAND $<TARGET_FILE:image_open> pngsuite/basn2c16.png
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basn2c16
	PROPERTIES
		REQUIRED_FILES "pngsuite/basn2c16.png"
		WILL_FAIL true
		FAIL_REGULAR_EXPRESSION "UnsupportedFormat thrown"
)

add_test(
	NAME test_basi0g08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basi0g08.png
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basi0g08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basi0g08.png"
		WILL_FAIL true
		FAIL_REGULAR_EXPRESSION "UnsupportedFormat thrown"
)

add_test(
	NAME test_basi3p08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basi3p08.png
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basi3p08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basi3p08.png"
		WILL_FAIL true
		FAIL_REGULAR_EXPRESSION "UnsupportedFormat thrown"
)

add_test(
	NAME test_basi4a08
	COMMAND $<TARGET_FILE:image_open> pngsuite/basi4a08.png
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)
set_tests_properties(
	test_basi4a08
	PROPERTIES
		REQUIRED_FILES "pngsuite/basi4a08.png"
		WILL_FAIL true
		FAIL_REGULAR_EXPRESSION "UnsupportedFormat thrown"
)
