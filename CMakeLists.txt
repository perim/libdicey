CMAKE_MINIMUM_REQUIRED(VERSION 3.12)
PROJECT(dicey VERSION 0.1)

# Enable the below for testing
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -O0 -Wall")

set(DICE_LIBS stdc++ m)
set(DICE_SRC dice.cpp dice.h dmath.h dmath.cpp)
enable_testing()

ADD_EXECUTABLE(chunkytest tests/chunkytest.cpp ${DICE_SRC} chunky.cpp chunky.h)
TARGET_INCLUDE_DIRECTORIES(chunkytest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(chunkytest ${DICE_LIBS})
ADD_TEST(chunkytest chunkytest)

ADD_EXECUTABLE(test1 tests/test1.cpp ${DICE_SRC})
TARGET_INCLUDE_DIRECTORIES(test1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(test1 ${DICE_LIBS})

ADD_EXECUTABLE(test_arpgstats tests/test_arpgstats.cpp arpgstats.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(test_arpgstats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(test_arpgstats ${DICE_LIBS})

ADD_EXECUTABLE(test_direction tests/test_direction.cpp direction.h)
TARGET_INCLUDE_DIRECTORIES(test_direction PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(test_direction ${DICE_LIBS})

ADD_EXECUTABLE(test_fixp tests/test_fixp.cpp fixp.h)
TARGET_INCLUDE_DIRECTORIES(test_fixp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(test_fixp ${DICE_LIBS})

ADD_EXECUTABLE(perf_roll tests/perf_roll.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_roll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_roll ${DICE_LIBS})

ADD_EXECUTABLE(perf_derive tests/perf_derive.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_derive PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_derive ${DICE_LIBS})

ADD_EXECUTABLE(perf_dmath tests/perf_dmath.cpp dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_dmath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_dmath ${DICE_LIBS})

ADD_EXECUTABLE(perf_table_rolls tests/perf_table_rolls.cpp dice.cpp dice.h dmath.h dmath.cpp)
TARGET_INCLUDE_DIRECTORIES(perf_table_rolls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_table_rolls ${DICE_LIBS})

ADD_EXECUTABLE(test_table_rolls tests/test_table_rolls.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(test_table_rolls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(test_table_rolls ${DICE_LIBS})

ADD_EXECUTABLE(perf_unique_rolls tests/perf_unique_rolls.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_unique_rolls PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_unique_rolls ${DICE_LIBS})

ADD_EXECUTABLE(perf_box tests/perf_box.cpp dice.cpp dice.h)
TARGET_INCLUDE_DIRECTORIES(perf_box PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_box ${DICE_LIBS})

ADD_EXECUTABLE(perf_prd tests/perf_prd.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_prd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_prd ${DICE_LIBS})

ADD_EXECUTABLE(visualization tests/visualization.cpp dice.cpp dice.h dmath.h dmath.cpp)
TARGET_INCLUDE_DIRECTORIES(visualization PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/external)
TARGET_LINK_LIBRARIES(visualization ${DICE_LIBS})

ADD_EXECUTABLE(perf_pow2_roll tests/perf_pow2_roll.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_pow2_roll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_pow2_roll ${DICE_LIBS})

ADD_EXECUTABLE(perf_quadratic_roll tests/perf_quadratic_roll.cpp dice.cpp dice.h dmath.h)
TARGET_INCLUDE_DIRECTORIES(perf_quadratic_roll PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(perf_quadratic_roll ${DICE_LIBS})

ADD_EXECUTABLE(stats tests/stats.cpp dmath.h dice.cpp dice.h)
TARGET_INCLUDE_DIRECTORIES(stats PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(stats ${DICE_LIBS})

ADD_LIBRARY(dicey ${DICE_SRC})
TARGET_INCLUDE_DIRECTORIES(dicey PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
TARGET_LINK_LIBRARIES(dicey ${DICE_LIBS})

ADD_TEST(test1 test1)
ADD_TEST(test_direction test_direction)
ADD_TEST(test_fixp test_fixp)
ADD_TEST(perf_roll perf_roll)
ADD_TEST(perf_derive perf_derive)
ADD_TEST(perf_table_rolls perf_table_rolls)
ADD_TEST(perf_unique_rolls perf_unique_rolls)
ADD_TEST(perf_box perf_box)
ADD_TEST(perf_pow2_roll perf_pow2_roll)
ADD_TEST(perf_quadratic_roll perf_quadratic_roll)
ADD_TEST(stats stats)
ADD_TEST(perf_prd perf_prd)
ADD_TEST(visualization visualization)
ADD_TEST(test_arpgstats test_arpgstats)
