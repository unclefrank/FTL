/*
 * Pattern.cpp -- regular expressions
 *
 * Copyright (c) 2007-2012, Frank Mertens
 *
 * This file is part of the a free software library. You can redistribute
 * it and/or modify it under the terms of FTL's 2-clause BSD license.
 *
 * See the LICENSE.txt file for details at the top-level of FTL's sources.
 */

#include "Singleton.hpp"
#ifndef NDEBUG
#include "SyntaxDebugger.hpp"
#endif
#include "Format.hpp"
#include "Pattern.hpp"

namespace ftl
{

PatternException::PatternException(const String& error, int pos)
	: error_(error),
	  pos_(pos)
{
	message_ = Format("%%: %%") << pos << error;
}

PatternException::~PatternException() throw()
{}

const char* PatternException::what() const throw()
{
	return message_;
}

class PatternCompiler: public Syntax<ByteArray>::Definition, public Singleton<PatternCompiler>
{
private:
	friend class Singleton<PatternCompiler>;
	friend class Pattern;

	PatternCompiler();
	void compile(Ref<ByteArray> text, Ref<Pattern> pattern);
	NODE compileChoice(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern);
	NODE compileSequence(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern);
	char readChar(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern);
	NODE compileRangeMinMax(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern);
	NODE compileRangeExplicit(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern);
	NODE compileRepeat(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern, NODE previous);

	int gap_;
	int any_;
	int boi_;
	int eoi_;
	int char_;
	int rangeMinMax_;
	int rangeExplicit_;
	int minRepeat_;
	int maxRepeat_;
	int repeat_;
	int sequence_;
	int group_;
	int choice_;
};

PatternCompiler::PatternCompiler()
{
	any_ = DEFINE("Any", CHAR('#'));
	gap_ = DEFINE("Gap", CHAR('*'));
	boi_ = DEFINE("Boi", CHAR('^'));
	eoi_ = DEFINE("Eoi", CHAR('$'));

	char_ =
		DEFINE("Char",
			CHOICE(
				EXCEPT("#*\\[](){}|^$"),
				GLUE(
					CHAR('\\'),
					HINT("Illegal escape sequence"),
					CHOICE(
						RANGE(
							"#*\\[](){}|^$"
							"fnrt"
							"\"/"
						),
						GLUE(
							CHAR('x'),
							REPEAT(2, 2,
								CHOICE(
									RANGE('0', '9'),
									RANGE('a', 'f'),
									RANGE('A', 'F')
								)
							)
						)
					),
					DONE()
				)
			)
		);

	rangeMinMax_ =
		DEFINE("RangeMinMax",
			GLUE(
				CHAR('['),
				CHOICE(
					GLUE(
						REPEAT(0, 1, CHAR('^')),
						CHOICE(
							GLUE(
								STRING(".."),
								REF("Char")
							),
							GLUE(
								REF("Char"),
								STRING(".."),
								REF("Char")
							),
							GLUE(
								REF("Char"),
								STRING("..")
							)
						)
					),
					STRING("..")
				),
				CHAR(']')
			)
		);

	rangeExplicit_ =
		DEFINE("RangeExplicit",
			GLUE(
				CHAR('['),
				REPEAT(0, 1, CHAR('^')),
				REPEAT(1, REF("Char")),
				CHAR(']')
			)
		);

	DEFINE_VOID("Number",
		REPEAT(1, 20,
			RANGE('0', '9')
		)
	);

	minRepeat_ = DEFINE("MinRepeat", INLINE("Number"));
	maxRepeat_ = DEFINE("MaxRepeat", INLINE("Number"));

	repeat_ =
		DEFINE("Repeat",
			GLUE(
				CHOICE(
					PREVIOUS("Char"),
					PREVIOUS("Any"),
					PREVIOUS("RangeMinMax"),
					PREVIOUS("RangeExplicit"),
					PREVIOUS("Group")
				),
				CHAR('{'),
				REPEAT(0, 1,
					GLUE(
						REPEAT(0, 1, REF("MinRepeat")),
						CHAR(','),
						REPEAT(0, 1, REF("MaxRepeat"))
					)
				),
				REPEAT(0, 1, RANGE("~?")),
				CHAR('}')
			)
		);

	sequence_ =
		DEFINE("Sequence",
			REPEAT(
				CHOICE(
					REF("Repeat"),
					REF("Char"),
					REF("Any"),
					REF("Gap"),
					REF("RangeMinMax"),
					REF("RangeExplicit"),
					REF("Boi"),
					REF("Eoi"),
					REF("Group")
				)
			)
		);

	group_ =
		DEFINE("Group",
			GLUE(
				CHAR('('),
				REF("Choice"),
				CHAR(')')
			)
		);

	choice_ =
		DEFINE("Choice",
			GLUE(
				REF("Sequence"),
				REPEAT(
					GLUE(
						CHAR('|'),
						REF("Sequence")
					)
				)
			)
		);

	ENTRY("Choice");
	LINK();
}

void PatternCompiler::compile(Ref<ByteArray> text, Ref<Pattern> pattern)
{
	Ref<State, Owner> state = newState();
	int i0 = 0, i1 = 0;
	Ref<Token, Owner> token = match(text, i0, &i1, state);
	if ((!token) || (i1 < text->length())) {
		String reason = "Syntax error";
		int pos = i1;
		if (state->hint()) {
			reason = state->hint();
			pos = state->hintOffset();
		}
		throw PatternException(reason, pos);
	}
	NODE entry = compileChoice(text, token, pattern);
	pattern->DEFINE("Expression", entry);
	pattern->ENTRY("Expression");
	pattern->LINK();
}

PatternCompiler::NODE PatternCompiler::compileChoice(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern)
{
	if (token->countChildren() == 1)
		return compileSequence(text, token->firstChild(), pattern);
	NODE node = new ChoiceNode;
	for (Ref<Token> child = token->firstChild(); child; child = child->nextSibling())
		node->appendChild(compileSequence(text, child, pattern));
	return pattern->debug(node, "Choice");
}

PatternCompiler::NODE PatternCompiler::compileSequence(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern)
{
	NODE node = new GlueNode;
	for (Ref<Token> child = token->firstChild(); child; child = child->nextSibling()) {
		if (child->rule() == char_) node->appendChild(pattern->CHAR(readChar(text, child, pattern)));
		else if (child->rule() == any_) node->appendChild(pattern->ANY());
		else if (child->rule() == gap_) node->appendChild(pattern->GREEDY_REPEAT(pattern->ANY()));
		else if (child->rule() == rangeMinMax_) node->appendChild(compileRangeMinMax(text, child, pattern));
		else if (child->rule() == rangeExplicit_) node->appendChild(compileRangeExplicit(text, child, pattern));
		else if (child->rule() == repeat_) node->appendChild(compileRepeat(text, child, pattern, node->lastChild()));
		else if (child->rule() == boi_) node->appendChild(pattern->BOI());
		else if (child->rule() == eoi_) node->appendChild(pattern->EOI());
		else if (child->rule() == group_) node->appendChild(compileChoice(text, child->firstChild(), pattern));
	}
	return pattern->debug(node, "Glue");
}

char PatternCompiler::readChar(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern)
{
	return (token->i1() - token->i0() > 1) ?
		text->copy(token)->expandInsitu()->at(0) :
		text->at(token->i0());
}

PatternCompiler::NODE PatternCompiler::compileRangeMinMax(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern)
{
	int n = token->countChildren();
	bool invert = (text->at(token->i0() + 1) == '^');
	if (n == 2) {
		Ref<Token> min = token->firstChild();
		Ref<Token> max = min->nextSibling();
		char a = readChar(text, min, pattern);
		char b = readChar(text, max, pattern);
		return  invert ? pattern->EXCEPT(a, b) : pattern->RANGE(a, b);
	}
	else if (n == 1) {
		Ref<Token> child = token->firstChild();
		char ch = readChar(text, child, pattern);
		return invert ?
			( (child->i0() - token->i0() <= 2) ? pattern->BELOW(ch)            : pattern->GREATER(ch)        ) :
			( (child->i0() - token->i0() <= 2) ? pattern->GREATER_OR_EQUAL(ch) : pattern->BELOW_OR_EQUAL(ch) );
	}
	return pattern->ANY();
}

PatternCompiler::NODE PatternCompiler::compileRangeExplicit(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern)
{
	Ref<Token> child = token->firstChild();
	bool invert = (text->at(token->i0() + 1) == '^');
	int n = token->countChildren();
	String s(n);
	for (int i = 0; i < n; ++i) {
		s->set(i, readChar(text, child, pattern));
		child = child->nextSibling();
	}
	return invert ? pattern->EXCEPT(s->data()) : pattern->RANGE(s->data());
}

PatternCompiler::NODE PatternCompiler::compileRepeat(Ref<ByteArray> text, Ref<Token> token, Ref<Pattern> pattern, NODE previous)
{
	Ref<Token> child = token->firstChild(), min, max;
	while (child) {
		if (child->rule() == minRepeat_) min = child;
		else if (child->rule() == maxRepeat_) max = child;
		child = child->nextSibling();
	}
	int minRepeat = min ? text->copy(min)->toInt() : 0;
	int maxRepeat = max ? text->copy(max)->toInt() : intMax;
	int modifier = text->at(token->i1() - 2);
	previous->unlink();
	if (modifier == '?')
		return pattern->LAZY_REPEAT(minRepeat, previous);
	else if (modifier == '~')
		return pattern->REPEAT(minRepeat, maxRepeat, previous);
	return pattern->GREEDY_REPEAT(minRepeat, maxRepeat, previous);
}

Pattern::Pattern(const char* text)
#ifndef NDEBUG
	: Syntax<ByteArray>::Definition(new Debugger)
#endif
{
	PatternCompiler::instance()->compile(String(text), this);
}

Pattern::Pattern(const String& text)
#ifndef NDEBUG
	: Syntax<ByteArray>::Definition(new Debugger)
#endif
{
	PatternCompiler::instance()->compile(text, this);
}

} // namespace ftl
