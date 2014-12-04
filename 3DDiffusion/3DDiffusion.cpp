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
	
	FILE *file;
	_wfopen_s(&file, path, L"r");
	if (file == NULL){
		printf("Unable to open %ls", path);
		getchar();
		throw;
	}

	struct coord
	{
		int x;
		int y;
		int z;
	};

	struct coordf
	{
		float x;
		float y;
		float z;
	};

	struct line
	{
		coord start;
		coord end;
	};

	std::vector<coord> vertices;
	std::vector<line> lines;
	std::vector<coord> points;

	char lineHeader[128];

	while (fscanf_s(file, "%s", lineHeader, sizeof(lineHeader)) != EOF)
	{
		if (strcmp(lineHeader, "v") == 0)
		{
			coordf vertexf;
			fscanf_s(file, "%f %f %f\r\n", &vertexf.x, &vertexf.y, &vertexf.z);
			vertexf.x = (vertexf.x + 1.0f) * ((dim - 1) / 2.0f);
			vertexf.y = (vertexf.y + 1.0f) * ((dim - 1) / 2.0f);
			vertexf.z = (vertexf.z + 1.0f) * ((dim - 1) / 2.0f);
			coord vertex = { (int)vertexf.x, (int)vertexf.y, (int)vertexf.z };
			vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "l") == 0)
		{
			int start, end;
			fscanf_s(file, "%d %d\r\n", &start, &end);
			line seg;
			seg.start = vertices[start - 1];
			seg.end = vertices[end - 1];
			lines.push_back(seg);
		}
		else if (strcmp(lineHeader, "p") == 0)
		{
			int index;
			fscanf_s(file, "%d\r\n", &index);
			coord point = vertices[index - 1];
			points.push_back(point);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	for (const line& seg : lines)
	{
		DrawLine3D(*imgF, seg.start.x, seg.start.y, seg.start.z, seg.end.x, seg.end.y, seg.end.z, surfaceColor);
		DrawLine3D(*constant, seg.start.x, seg.start.y, seg.start.z, seg.end.x, seg.end.y, seg.end.z, 1U);
	}

	for (const coord& point : points)
	{
		(*imgF)[point.x][point.y][point.z] = interiorColor;
		(*constant)[point.x][point.y][point.z] = 1U;
	}

	//DrawLine3D(*imgF, 117, 117, 127, 117, 137, 147, surfaceColor);
}

extern "C" {
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}


int wmain(int argc, wchar_t *argv[])
{
	LARGE_INTEGER perfFreq;
	::QueryPerformanceFrequency(&perfFreq);

	imageType *imgF;
	boolMat *constant;
	generateInitalGrid(argc > 1 ? argv[1] : L"D:\\Desktop\\test.obj", imgF, constant);

	Concurrency::array_view<pixelType, 3> imageView(dim, dim, dim, reinterpret_cast<pixelType*>(imgF));
	Concurrency::array_view<boolType, 3> constantView(dim, dim, dim, reinterpret_cast<boolType*>(constant));
	Concurrency::array_view<const float, 3> filterView(filterSize, filterSize, filterSize, reinterpret_cast<const float*>(averagingFilter));


	HighlightBorder(imageView, borderColor);
	HighlightBorder(constantView, 1U);

	LARGE_INTEGER startTime, endTime;
	::QueryPerformanceCounter(&startTime);

	const unsigned int numIterations = 5000U;

	for (unsigned int iter = 0; iter < numIterations; iter++)
	{
		filterImage(imageView, filterView, constantView, -1.0f, 1.0f);
		if (iter % 20 == 0)
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

