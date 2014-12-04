// 3DDiffusion.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "operations.h"
#include "binvox.h"

const pixelType borderColor = -1.0f;
const pixelType intialColor = -0.1f;
const pixelType surfaceColor = 0.0f;
const pixelType interiorColor = 1.0f;

void generateInitalGrid(const wchar_t *path,
	imageType*& imgF, boolMat*& constant)
{
	imgF = reinterpret_cast<imageType*>(new pixelType[dim * dim * dim]);
	::memset(imgF, *reinterpret_cast<const int*>(&intialColor), sizeof(imageType));

	constant = reinterpret_cast<boolMat*>(new boolType[dim * dim * dim]);
	::memset(constant, 0, sizeof(boolMat)); 
	
	DrawCircleXY(*imgF, 127, 127, 128, 63, surfaceColor);
	DrawCircleXY(*imgF, 127, 127, 80, 40, surfaceColor);
	DrawCircleXY(*imgF, 127, 127, 168, 40, surfaceColor);
	DrawCircleXZ(*imgF, 127, 127, 128, 58, surfaceColor);

	(*imgF)[127][127][128] = interiorColor;
	(*imgF)[127][127][80] = interiorColor;
	(*imgF)[127][127][168] = interiorColor;

	DrawCircleXY(*constant, 127, 127, 128, 63, 1U);
	DrawCircleXY(*constant, 127, 127, 80, 40, 1U);
	DrawCircleXY(*constant, 127, 127, 168, 40, 1U);
	DrawCircleXZ(*constant, 127, 127, 128, 58, 1U);

	(*constant)[127][127][128] = 1U;
	(*constant)[127][127][80] = 1U;
	(*constant)[127][127][168] = 1U;
}

int wmain(int argc, wchar_t *argv[])
{
	LARGE_INTEGER perfFreq;
	::QueryPerformanceFrequency(&perfFreq);

	imageType *imgF;
	boolMat *constant;
	generateInitalGrid(argc > 1 ? argv[1] : nullptr, imgF, constant);

	Concurrency::array_view<pixelType, 3> imageView(dim, dim, dim, reinterpret_cast<pixelType*>(imgF));
	Concurrency::array_view<boolType, 3> constantView(dim, dim, dim, reinterpret_cast<boolType*>(constant));
	Concurrency::array_view<const float, 3> filterView(filterSize, filterSize, filterSize, reinterpret_cast<const float*>(averagingFilter));


	HighlightBorder(imageView, borderColor);
	HighlightBorder(constantView, 1U);

	LARGE_INTEGER startTime, endTime;
	::QueryPerformanceCounter(&startTime);

	const unsigned int numIterations = 0U;

	for (unsigned int iter = 0; iter < numIterations; iter++)
	{
		filterImage(imageView, filterView, constantView, -1.0f, 1.0f);
		if (iter % 30 == 0)
			imageView.synchronize();
		std::cout << "Iteration: " << iter << "\n";
	}
	imageView.synchronize();

	const std::wstring dumpPath = L"D:\\Desktop\\prethresh.dmp";

	std::ofstream dump(dumpPath, std::ios_base::binary | std::ios_base::trunc);
	if (!dump.is_open())
	{
		throw L"Unable to open" + dumpPath;
	}
	dump.write(reinterpret_cast<const char*>(imgF), sizeof(imageType));
	dump.close();

	thresholdImage(imageView, surfaceColor - 0.3f, 0.0f, 1.0f);
	imageView.synchronize();

	::QueryPerformanceCounter(&endTime);

	std::cout << "\n" << numIterations << " iterations completed in " << (endTime.QuadPart - startTime.QuadPart) / perfFreq.QuadPart
		<< " seconds\n\n";

	VoxelFile circleFile;
	circleFile.dimensions[0] = circleFile.dimensions[1] = circleFile.dimensions[2] = dim;
	circleFile.scale = 1.0f;
	circleFile.offset[0] = circleFile.offset[1] = circleFile.offset[2] = 0.0f;
	std::vector<VoxelRun> runs = EncodeVoxels(reinterpret_cast<pixelType*>(imgF), dim * dim * dim);
	circleFile.data = &runs[0];

	delete[] reinterpret_cast<boolType*>(constant);
	delete[] reinterpret_cast<pixelType*>(imgF);

	SaveVoxelFile(circleFile, L"D:\\Desktop\\test.binvox");


	return 0;
}

