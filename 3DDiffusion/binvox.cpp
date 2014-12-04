#include "stdafx.h"
#include "binvox.h"

void SaveVoxelFile(const VoxelFile& voxelFile, const std::wstring path)
{
	std::ofstream stream(path, std::ios_base::binary | std::ios_base::trunc);
	if (!stream.is_open())
	{
		throw L"Unable to open" + path;
	}
	stream << "#binvox 1\n";
	stream << "dim " << voxelFile.dimensions[0] << " " << voxelFile.dimensions[1] << " " << voxelFile.dimensions[2] << "\n";
	stream << "translate " << voxelFile.offset[0] << " " << voxelFile.offset[1] << " " << voxelFile.offset[2] << "\n";
	stream << "scale " << voxelFile.scale << "\n";
	stream << "data\n";
	size_t totalVoxels = voxelFile.dimensions[0] * voxelFile.dimensions[1] * voxelFile.dimensions[2];
	size_t count = 0;
	VoxelRun *run = voxelFile.data;
	while (count < totalVoxels)
	{
		stream.write(reinterpret_cast<char*>(run), sizeof(VoxelRun));
		count += run->runLength;
		run++;
	}
	stream.close();
}