/*
 * Copyright (C) 2007-2013 Frank Mertens.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef FMAKE_BUILDPLAN_H
#define FMAKE_BUILDPLAN_H

#include <fkit/PrefixTree.h>
#include "Module.h"
#include "ToolChain.h"

namespace fkit {
class FileStatus;
class Config;
} // namespace fkit

namespace fmake
{

using namespace fkit;

class BuildPlan;

typedef List< Ref<BuildPlan> > BuildPlanList;

class BuildPlan: public Object
{
public:
	enum Option {
		Static        = 1 << 0,
		Application   = 1 << 1,
		Library       = 1 << 2,
		Tools         = 1 << 3,
		Tests         = 1 << 4,
		Package       = 1 << 5,
		Debug         = 1 << 6,
		Release       = 1 << 7,
		Simulate      = 1 << 8,
		Blindfold     = 1 << 9,
		Bootstrap     = 1 << 10,
		BuildTests    = 1 << 11,
		Verbose       = 1 << 12,
		OptimizeSpeed = 1 << 13,
		OptimizeSize  = 1 << 14,
		OptimizeDebug = 1 << 15,
		Unspecified   = -1,
		GlobalOptions = Debug|
		                Release|
		                Simulate|
		                Blindfold|
		                Bootstrap|
		                BuildTests|
		                Verbose|
		                OptimizeSpeed|
		                OptimizeSize|
		                OptimizeDebug
	};

	static Ref<BuildPlan> create(int argc, char **argv);

	inline ToolChain *toolChain() const { return toolChain_; }
	inline String projectPath() const { return projectPath_; }
	inline String modulePath() const { return modulePath_; }
	inline Config *recipe() const { return recipe_; }

	inline String name() const { return name_; }
	inline String version() const { return version_; }
	inline int options() const { return options_; }
	inline int speedOptimizationLevel() const { return speedOptimizationLevel_; }
	inline int sizeOptimizationLevel() const { return sizeOptimizationLevel_; }

	inline StringList *includePaths() const { return includePaths_; }
	inline StringList *libraryPaths() const { return libraryPaths_; }
	inline StringList *libraries() const { return libraries_; }
	inline StringList *sources() const { return sources_; }
	inline StringList *previousSources() const { return previousSources_; }
	inline ModuleList *modules() const { return modules_; }

	inline BuildPlanList *prequisites() const { return prequisites_; }

	int make();

	String sourcePath(String source) const;
	String modulePath(String object) const;
	String installPath(String relativeInstallPath) const;

	String beautifyCommand(String command);
	bool runBuild(String command);

	bool mkdir(String path);
	bool rmdir(String path);
	bool symlink(String path, String newPath);
	bool unlink(String path);
	bool install(String sourcePath, String destPath);

	Ref<FileStatus> fileStatus(String path);

private:
	Ref<BuildPlan> create(String projectPath);

	typedef PrefixTree<char, BuildPlan *> BuildMap;

	BuildPlan(int argc, char **argv);
	BuildPlan(String projectPath, BuildPlan *parentPlan);
	void initFlags();

	void readRecipe(BuildPlan *parentPlan = 0);

	void prepare();
	bool analyse();
	bool build();
	int testRun();
	bool install();
	bool uninstall();
	void clean();

	Ref<ToolChain> toolChain_;
	String projectPath_;
	String modulePath_;
	Ref<Config> recipe_;

	String name_;
	String version_;
	int options_;
	int speedOptimizationLevel_;
	int sizeOptimizationLevel_;

	Ref<StringList> includePaths_;
	Ref<StringList> libraryPaths_;
	Ref<StringList> libraries_;
	Ref<StringList> sources_;
	Ref<StringList> previousSources_;
	Ref<ModuleList> modules_;

	Ref<BuildPlanList> prequisites_;

	Ref<BuildMap> buildMap_;
	String sourcePrefix_;
	String installPrefix_;

	bool prepareComplete_;
	bool analyseComplete_;
	bool buildComplete_;
	bool testRunComplete_;
	bool installComplete_;
	bool uninstallComplete_;
	bool cleanComplete_;

	bool analyseResult_;
	bool buildResult_;
	int testRunResult_;
};

} // namespace fmake

#endif // FMAKE_BUILDPLAN_H