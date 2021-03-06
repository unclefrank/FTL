/*
 * Copyright (C) 2007-2015 Frank Mertens.
 *
 * Use of this source is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef FLUX_COLORNAMES_H
#define FLUX_COLORNAMES_H

#include <flux/Color>

namespace flux {

template<class Char, class Value> class PrefixTree;

template<class SubClass> class Singleton;

/** \brief Lookup table for SVG standard color names
  */
class ColorNames: public Object
{
public:
    bool lookup(const char *name, Color *color) const;

private:
    friend class Singleton<ColorNames>;

    ColorNames();

    typedef PrefixTree<char, Color> ColorByName;
    Ref<ColorByName> colorByName_;
};

const ColorNames *colorNames();

} // namespace flux

#endif // FLUX_COLORNAMES_H
