/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUXTOKI_PALETTELOADER_H
#define FLUXTOKI_PALETTELOADER_H

#include <flux/toki/Palette>

namespace flux {

template<class> class Singleton;

namespace toki {

class PaletteLoader: public Object
{
public:
    Ref<Palette> load(String path) const;

private:
    friend class Singleton<PaletteLoader>;
    PaletteLoader();

    Ref<MetaProtocol> protocol_;
};

const PaletteLoader *paletteLoader();

}} // namespace flux::toki

#endif // FLUXTOKI_PALETTELOADER_H
