#include "Generator.h"
#include "Registry.h"

namespace fluxdoc
{

Registry::Registry()
	: generatorByName_(GeneratorByName::create())
{}

void Registry::registerGenerator(Generator *generator)
{
	generatorByName_->insert(generator->name(), generator);
}

Generator* Registry::generatorByName(String name) const
{
	Ref<Generator> generator;
	generatorByName_->lookup(name, &generator);
	return generator;
}

Generator* Registry::generatorByIndex(int index) const
{
	return generatorByName_->valueAt(index);
}

int Registry::generatorCount() const
{
	return generatorByName_->size();
}

} // namespace fluxdoc
