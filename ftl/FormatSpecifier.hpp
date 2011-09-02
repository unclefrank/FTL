/*
 * FormatSpecifier.hpp -- syntax of "%%"-placeholders inside 'Format' strings
 *
 * Copyright (c) 2007-2011, Frank Mertens
 *
 * See ../COPYING for the license.
 */
#ifndef FTL_FORMATSPECIFIER_HPP
#define FTL_FORMATSPECIFIER_HPP

#include "Syntax.hpp"
#include "Singleton.hpp"

namespace ftl
{

class FormatSpecifier: public Syntax<ByteArray>::Definition, public Singleton<FormatSpecifier>
{
public:
	bool find(Ref<ByteArray> text, int* i0, int* i1, int* w, int* wi, int* wf, int* base, bool* exp, char* blank);
	
private:
	friend class Singleton<FormatSpecifier>;
	
	FormatSpecifier();
	
	int width_;
	int integerWidth_;
	int fractionWidth_;
	int base_;
	int exp_;
	int blank_;
	int format_;
};

inline Ref<FormatSpecifier> formatSpecifier() { return FormatSpecifier::instance(); }

} // namespace ftl

#endif // FTL_FORMATSPECIFIER_HPP
