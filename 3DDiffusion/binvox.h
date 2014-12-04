#include "stdafx.h"
#pragma once

struct VoxelRun
{
	unsigned char value;
	unsigned char runLength;
};

struct VoxelFile
{
	unsigned short dimensions[3];
	float offset[3];
	float scale;
	VoxelRun *data;
};

template <typename value_type>
std::vector<VoxelRun> EncodeVoxels(value_type *voxels, size_t count)
{
	VoxelRun currentRun;
	bool first = true;
	std::vector<VoxelRun> runs;

	for (ptrdiff_t i = 0; i < count; i++)
	{
		const value_type& currentVoxel = *(voxels + i);

		if (first || currentVoxel != currentRun.value ||
			currentRun.runLength == 255U)
		{
			if (!first)
			{
				runs.push_back(currentRun);
			}

			currentRun.value = static_cast<unsigned char>(currentVoxel);
			currentRun.runLength = 1U;
			first = false;
		}
		else
		{
			currentRun.runLength++;
		}
	}
	runs.push_back(currentRun);

	return std::move(runs);
}

void SaveVoxelFile(const VoxelFile& voxelFile, const std::wstring path);