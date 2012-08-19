/*
 * CommandLine.hpp -- command line interface
 *
 * Copyright (c) 2007-2012, Frank Mertens
 *
 * This file is part of the a free software library. You can redistribute
 * it and/or modify it under the terms of FTL's 2-clause BSD license.
 *
 * See the LICENSE.txt file for details at the top-level of FTL's sources.
 */
#ifndef FTL_COMMANDLINE_HPP
#define FTL_COMMANDLINE_HPP

#include "atoms"
#include "String.hpp"
#include "Variant.hpp"
#include "List.hpp"
#include "SyntaxDefinition.hpp"
#include "CommandOption.hpp"

namespace ftl
{

FTL_EXCEPTION(CommandLineException, Exception);

class CommandLine: public SyntaxDefinition
{
public:
	inline static Ref<CommandLine, Owner> newInstance() { return new CommandLine(); }

	Ref<CommandOption> define(char shortName, String longName, Variant defaultValue = false, String description = "");
	String entity(String newValue = "");
	String synopsis(String newValue = "");
	String summary(String newValue = "");
	String details(String newValue = "");

	Ref<StringList, Owner> read(int argc, char **argv);

	typedef List< Ref<CommandOption, Owner> > OptionList;
	Ref<OptionList> usedOptions() const;

	String helpText() const;
	String execPath() const;
	String execName() const;
	String execDir() const;

private:
	CommandLine();

	Ref<StringList, Owner> read(String line);
	Ref<CommandOption> optionByShortName(char name) const;
	Ref<CommandOption> optionByLongName(String name) const;

	void readOption(String line, Ref<Token> token);
	void readValue(Ref<CommandOption> option, String line, Ref<Token> token);
	void verifyTypes();

	int longNameRule_;
	int shortNameRule_;
	int valueRule_;
	int optionRule_;

	Ref<OptionList, Owner> definedOptions_;
	Ref<OptionList, Owner> usedOptions_;
	String entity_;
	String synopsis_;
	String summary_;
	String details_;

	String execPath_;
	String execName_;
	String execDir_;

	int position_;
};

} // namespace ftl

#endif // FTL_COMMANDLINE_HPP
