/*
 * Copyright (C) 2007-2014 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "System.h"
#include "Random.h"

namespace flux
{

Random::Random(int seed)
	: x_(seed)
{
	if (seed < 0) {
		while (x_ <= 1)
			x_ = int(System::now() * 1e9);
	}
	else {
		x_ = seed + 2;
		if (x_ <= 1) x_ += 2; // overflow check
	}
}

} // namespace flux