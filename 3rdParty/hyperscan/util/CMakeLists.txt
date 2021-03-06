# utility libs

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CXX_FLAGS}")
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/tools/ExpressionParser.cpp
    PROPERTIES
    COMPILE_FLAGS "${RAGEL_C_FLAGS}")

IF (WITH_RAGEL STREQUAL "YES")
ragelmaker(ExpressionParser.rl)
ENDIF (WITH_RAGEL STREQUAL "YES")

set(expressionutil_SRCS
    expressions.cpp
    expressions.h
    ExpressionParser.h
    ExpressionParser.cpp
    )
add_library(expressionutil STATIC ${expressionutil_SRCS})
IF (WITH_RAGEL STREQUAL "YES")
add_dependencies(expressionutil ragel_ExpressionParser)
ENDIF (WITH_RAGEL STREQUAL "YES")

SET(corpusomatic_SRCS
    ng_corpus_editor.h
    ng_corpus_editor.cpp
    ng_corpus_generator.h
    ng_corpus_generator.cpp
    ng_corpus_properties.h
    ng_corpus_properties.cpp
    ng_find_matches.h
    ng_find_matches.cpp
)
add_library(corpusomatic STATIC ${corpusomatic_SRCS})

