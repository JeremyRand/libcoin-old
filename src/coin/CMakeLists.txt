
# FIXME: For OS X, need flag for Framework or dylib
IF(DYNAMIC_LIBCOIN)
    ADD_DEFINITIONS(-DCOIN_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DCOIN_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME coin)

SET(HEADER_PATH ${PROJECT_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
    ${HEADER_PATH}/serialize.h
    ${HEADER_PATH}/uint256.h
    ${HEADER_PATH}/util.h
    
    ${HEADER_PATH}/Asset.h
    ${HEADER_PATH}/Address.h
    ${HEADER_PATH}/BigNum.h
    ${HEADER_PATH}/Block.h
    ${HEADER_PATH}/Key.h
    ${HEADER_PATH}/KeyStore.h
    ${HEADER_PATH}/Script.h
    ${HEADER_PATH}/Transaction.h
    ${LIBCOIN_CONFIG_HEADER}
)

#ADD_LIBRARY(${LIB_NAME}
#    ${LIBCOIN_USER_DEFINED_DYNAMIC_OR_STATIC}
#    ${LIB_PUBLIC_HEADERS}
SET(TARGET_SRC
    util.cpp
    
    Address.cpp
    Asset.cpp
    Block.cpp
    KeyStore.cpp
    Transaction.cpp
    Script.cpp
    ${LIBCOIN_VERSIONINFO_RC}
)
SET(TARGET_LIBRARIES )


SET(TARGET_EXTERNAL_LIBRARIES
    ${CMAKE_THREAD_LIBS_INIT}    
    ${MATH_LIBRARY} 
    ${SSL_LIBRARY} 
    ${CRYPTO_LIBRARY} 
    ${BOOST_LIBRARIES} 
    ${DL_LIBRARY}
)

SETUP_LIBRARY(${LIB_NAME})
