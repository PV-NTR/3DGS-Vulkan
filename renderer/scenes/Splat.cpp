#include "Splat.hpp"

namespace X {

std::unique_ptr<Splat> Splat::MakeUnique(std::string fileName)
{
	return std::unique_ptr<Splat>(new Splat(fileName));
}

std::shared_ptr<Splat> Splat::MakeShared(std::string fileName)
{
	return std::shared_ptr<Splat>(new Splat(fileName));
}

Splat::Splat(std::string fileName) : Object(Type::Splat)
{
	// TODO: load data from file
}

} // namespace X