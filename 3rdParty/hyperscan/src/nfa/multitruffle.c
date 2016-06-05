/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "ue2common.h"

#include "multitruffle.h"
#include "util/bitutils.h"
#include "util/simd_utils.h"
#include "util/simd_utils_ssse3.h"

#include "multiaccel_common.h"

#if !defined(__AVX2__)

#define MATCH_ALGO long_
#include "multiaccel_long.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#define MATCH_ALGO longgrab_
#include "multiaccel_longgrab.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#define MATCH_ALGO shift_
#include "multiaccel_shift.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#define MATCH_ALGO shiftgrab_
#include "multiaccel_shiftgrab.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#define MULTIACCEL_DOUBLE

#define MATCH_ALGO doubleshift_
#include "multiaccel_doubleshift.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#define MATCH_ALGO doubleshiftgrab_
#include "multiaccel_doubleshiftgrab.h"
#include "multitruffle_sse.h"
#undef MATCH_ALGO

#undef MULTIACCEL_DOUBLE

#else

#define MATCH_ALGO long_
#include "multiaccel_long.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#define MATCH_ALGO longgrab_
#include "multiaccel_longgrab.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#define MATCH_ALGO shift_
#include "multiaccel_shift.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#define MATCH_ALGO shiftgrab_
#include "multiaccel_shiftgrab.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#define MULTIACCEL_DOUBLE

#define MATCH_ALGO doubleshift_
#include "multiaccel_doubleshift.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#define MATCH_ALGO doubleshiftgrab_
#include "multiaccel_doubleshiftgrab.h"
#include "multitruffle_avx2.h"
#undef MATCH_ALGO

#undef MULTIACCEL_DOUBLE

#endif
