have_lz4_header=n
have_lz4_lib=n
AC_CHECK_HEADERS([lz4frame.h],[have_lz4_header=y])
AC_CHECK_LIB(lz4,LZ4F_compressFrame,[have_lz4_lib=y])
if test "x${have_lz4_header}${have_lz4_lib}" != xyy
then
  AC_MSG_ERROR([
  -----------------------------------------------------
  The liblz4 build prerequisite was not found. Please
  see the build instructions, install liblz4 and retry.
  -----------------------------------------------------
  ])
fi
