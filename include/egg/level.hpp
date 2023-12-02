#pragma once

#include "offset_pointer.hpp"
#include "serialization.hpp"
#include "math_types.hpp"
#include "egg/asset.hpp"
#include <cstdint>

struct Packed LevelMeshes
{
	OffsetArray<Matrix> mesh_transforms;
	OffsetArray<Asset::Reference> mesh_files;
};

static void collect_references(std::unordered_set<Asset::Reference>& references, const LevelMeshes& meshes)
{
	collect_references(references, meshes.mesh_files);
}

struct Packed LevelFileHeader
{
	LevelMeshes meshes;
};

static void collect_references(std::unordered_set<Asset::Reference>& references, const LevelFileHeader& level)
{
	collect_references(references, level.meshes);
}