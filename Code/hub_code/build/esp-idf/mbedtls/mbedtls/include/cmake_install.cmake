# Install script for directory: /Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "/Users/kylemartin/Documents/senioryear/fall/EC444/esp-idf/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

