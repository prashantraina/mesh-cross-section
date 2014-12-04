#include "stdafx.h"
#pragma once

static const unsigned dim = 256U;
typedef float pixelType;
static const size_t filterSize = 3U;
static const unsigned short maxPowerOf2 = 8U;
typedef float filter[filterSize][filterSize][filterSize];
template <unsigned short power>
using imageLevelType = pixelType[1 << power][1 << power][1 << power];
typedef imageLevelType<maxPowerOf2> imageType;
typedef unsigned int boolType;
template <unsigned short power>
using boolLevelType = boolType[1 << power][1 << power][1 << power];
typedef boolLevelType<maxPowerOf2> boolMat;

static const filter averagingFilter = {
	0, 0, 0,
	0, 1, 0,
	0, 0, 0,

	0, 1, 0,
	1, 0, 1,
	0, 1, 0,

	0, 0, 0,
	0, 1, 0,
	0, 0, 0
};

template<int rank, typename value_type, typename bool_type>
void filterImage(Concurrency::array_view<value_type, rank>& image,
	const Concurrency::array_view<const float, rank>& filter,
	const Concurrency::array_view<bool_type, rank>& constantPoints,
	value_type clampLow,
	value_type clampHigh)
{
	static_assert(std::is_arithmetic<value_type>::value, "filterImage: the type parameter value_type should be a numeric type");
	static_assert(std::is_integral<bool_type>::value, "filterImage: the type parameter bool_type should be an integral type");

	int filterSize = filter.extent[0];
	assert(filterSize > 1 && filterSize % 2 == 1);

	int numFilterElems = filterSize;

	for (int i = 1; i < rank; i++)
	{
		assert(filter.extent[i] == filterSize);
		numFilterElems *= filterSize;
	}

	for (int i = 0; i < rank; i++)
		assert(image.extent[i] == constantPoints.extent[i]);

	Concurrency::parallel_for_each(image.extent,
		[=](Concurrency::index<rank> idx)
		restrict(amp)
	{
		if (constantPoints[idx] != 0)
			return;

		for (int r = 0; r < rank; r++)
		{
			if (idx[r] < (filterSize / 2) || idx[r] >= (image.extent[r] - (filterSize / 2)))
				return;
		}

		float sum = 0;
		float sumCoeffs = 0;

		for (int i = 0; i < numFilterElems; i++)
		{
			Concurrency::index<rank> filterIndex;
			Concurrency::index<rank> sampleIndex;
			int tmp = i;
			for (int r = 0; r < rank; r++)
			{
				filterIndex[r] = tmp % filterSize;
				sampleIndex[r] = idx[r] + filterIndex[r] - (filterSize / 2);
				tmp /= filterSize;
			}

			float coeff = filter[filterIndex];
			float sample = image[sampleIndex];
			sum += sample * coeff;
			sumCoeffs += coeff;
		}

		image[idx] = (value_type)Concurrency::direct3d::clamp(sum / sumCoeffs, (float)clampLow, (float)clampHigh);
	});
}

//http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
template<typename image_type, typename pixel_type>
void DrawCircleXY(image_type& imgF, unsigned centreX, unsigned centreY, unsigned centreZ, unsigned radius, pixel_type color)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;

	while (x >= y)
	{
		if ((x + centreX < dim) && (x + centreX >= 0))
		{
			if ((y + centreY < dim) && (y + centreY >= 0))
				imgF[x + centreX][y + centreY][centreZ] = color;
			if ((-y + centreY < dim) && (-y + centreY >= 0))
				imgF[x + centreX][-y + centreY][centreZ] = color;
		}
		if ((y + centreX < dim) && (y + centreX >= 0))
		{
			if ((x + centreY < dim) && (x + centreY >= 0))
				imgF[y + centreX][x + centreY][centreZ] = color;
			if ((-x + centreY < dim) && (-x + centreY >= 0))
				imgF[y + centreX][-x + centreY][centreZ] = color;
		}
		if ((-x + centreX < dim) && (-x + centreX >= 0))
		{
			if ((y + centreY < dim) && (y + centreY >= 0))
				imgF[-x + centreX][y + centreY][centreZ] = color;
			if ((-y + centreY < dim) && (-y + centreY >= 0))
				imgF[-x + centreX][-y + centreY][centreZ] = color;
		}
		if ((-y + centreX < dim) && (-y + centreX >= 0))
		{
			if ((x + centreY < dim) && (x + centreY >= 0))
				imgF[-y + centreX][x + centreY][centreZ] = color;
			if ((-x + centreY < dim) && (-x + centreY >= 0))
				imgF[-y + centreX][-x + centreY][centreZ] = color;
		}
		y++;
		if (radiusError<0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x + 1);
		}
	}
}

//http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
template<typename image_type, typename pixel_type>
void DrawCircleXZ(image_type& imgF, unsigned centreX, unsigned centreY, unsigned centreZ, unsigned radius, pixel_type color)
{
	int x = radius;
	int y = 0;
	int radiusError = 1 - x;

	while (x >= y)
	{
		if ((x + centreX < dim) && (x + centreX >= 0))
		{
			if ((y + centreZ < dim) && (y + centreZ >= 0))
				imgF[x + centreX][centreY][y + centreZ] = color;
			if ((-y + centreZ < dim) && (-y + centreZ >= 0))
				imgF[x + centreX][centreY][-y + centreZ] = color;
		}
		if ((y + centreX < dim) && (y + centreX >= 0))
		{
			if ((x + centreZ < dim) && (x + centreZ >= 0))
				imgF[y + centreX][centreY][x + centreZ] = color;
			if ((-x + centreZ < dim) && (-x + centreZ >= 0))
				imgF[y + centreX][centreY][-x + centreZ] = color;
		}
		if ((-x + centreX < dim) && (-x + centreX >= 0))
		{
			if ((y + centreZ < dim) && (y + centreZ >= 0))
				imgF[-x + centreX][centreY][y + centreZ] = color;
			if ((-y + centreZ < dim) && (-y + centreZ >= 0))
				imgF[-x + centreX][centreY][-y + centreZ] = color;
		}
		if ((-y + centreX < dim) && (-y + centreX >= 0))
		{
			if ((x + centreZ < dim) && (x + centreZ >= 0))
				imgF[-y + centreX][centreY][x + centreZ] = color;
			if ((-x + centreZ < dim) && (-x + centreZ >= 0))
				imgF[-y + centreX][centreY][-x + centreZ] = color;
		}
		y++;
		if (radiusError<0)
		{
			radiusError += 2 * y + 1;
		}
		else
		{
			x--;
			radiusError += 2 * (y - x + 1);
		}
	}
}

template<int rank, typename value_type>
void HighlightBorder(Concurrency::array_view<value_type, rank>& image, value_type highlightColor)
{
	Concurrency::parallel_for_each(image.extent,
		[=](Concurrency::index<rank> idx)
		restrict(amp)
	{
		for (int i = 0; i < rank; i++)
		{
			if (idx[i] == 0 || idx[i] == image.extent[i] - 1)
			{
				image[idx] = highlightColor;
				break;
			}
		}
	});
	image.synchronize();
}

//http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
//http://cobrabytes.squeakyduck.co.uk/forum/index.php?topic=1150.0
template<typename image_type, typename pixel_type>
void DrawLine3D(image_type& imgF, 
	int startX, int startY, int startZ,
	int endX, int endY, int endZ,
	pixel_type color)
{
	bool swapXY = abs(endY - startY) > abs(endX - startX);
	
	if (swapXY)
	{
		std::swap(startX, startY);
		std::swap(endX, endY);
	}

	bool swapXZ = abs(endZ - startZ) > abs(endX - startX);

	if (swapXZ)
	{
		std::swap(startX, startZ);
		std::swap(endX, endZ);
	}

	int deltaX = abs(endX - startX);
	int deltaY = abs(endY - startY);
	int deltaZ = abs(endZ - startZ);

	int driftXY = deltaX / 2;
	int driftXZ = deltaX / 2;

	int stepX = (endX >= startX) ? 1 : -1;
	int stepY = (endY >= startY) ? 1 : -1;
	int stepZ = (endZ >= startZ) ? 1 : -1;

	int y = startY;
	int z = startZ;

	for (int x = startX; x <= endX; x += stepX)
	{
		int cx = x, cy = y, cz = z;

		if (swapXZ)
			std::swap(cx, cz);
		if (swapXY)
			std::swap(cx, cy);

		imgF[cx][cy][cz] = color;

		driftXY -= deltaY;
		driftXZ -= deltaZ;

		if (driftXY < 0)
		{
			y += stepY;
			driftXY += deltaX;
		}

		if (driftXZ < 0)
		{
			z += stepZ;
			driftXZ += deltaX;
		}
	}
}

template<int rank, typename value_type>
void thresholdImage(Concurrency::array_view<value_type, rank>& image,
	value_type threshold,
	value_type lowValue,
	value_type highValue)
{
	Concurrency::parallel_for_each(image.extent,
		[=](Concurrency::index<rank> idx)
		restrict(amp)
	{
		if (image[idx] >= threshold)
			image[idx] = highValue;
		else
			image[idx] = lowValue;
	});
}