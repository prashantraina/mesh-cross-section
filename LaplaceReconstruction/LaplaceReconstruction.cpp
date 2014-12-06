// LaplaceReconstruction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


struct Vertex
{
	float x;
	float y;
	float z;
	unsigned id;
};

struct HE_Vertex;
struct HE_Edge;
struct HE_Face;

struct HE_Edge
{
	HE_Vertex *vertex;
	HE_Edge *pair;
	HE_Edge *next;
	HE_Face *face;

	inline const HE_Vertex * oppositeVertex() const
	{
		return next->vertex;
	}
};

struct HE_Vertex
{
	Vertex *actualVertex;
	HE_Edge *edge;

	inline std::vector<const HE_Vertex*> neighboringVertices() const
	{
		std::vector<const HE_Vertex*> result;
		HE_Edge *currentEdge = edge;
		do
		{
			result.insert(result.begin(), currentEdge->vertex);
			currentEdge = currentEdge->pair->next;
		} while (currentEdge != edge);

		return result;
	}
};


struct HE_Face
{
	HE_Edge *edge;
	HE_Vertex *vertex;

	inline std::vector<const HE_Edge*> edges() const
	{
		std::vector<const HE_Edge*> result;
		HE_Edge *currentEdge = edge;
		do
		{
			result.push_back(currentEdge);
			currentEdge = currentEdge->next;
		} while (currentEdge != edge);

		return result;

	}

	inline std::vector<const HE_Face*> adjacentFaces() const
	{
		std::vector<const HE_Face*> result;
		HE_Edge *currentEdge = edge;
		do
		{
			result.push_back(currentEdge->pair->face);
			currentEdge = currentEdge->next;
		} while (currentEdge != edge);

		return result;
	}
};


int wmain(int argc, wchar_t* argv[])
{
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;

	std::wstring path = argc > 1 ? argv[1] : L"C:\\Users\\Concordia\\Desktop\\recon_heart.obj";

	FILE *file;
	_wfopen_s(&file, path.c_str(), L"r");
	if (file == NULL){
		printf("Unable to open %ls", path);
		getchar();
		throw;
	}

	char lineHeader[128];

	while (fscanf_s(file, "%s", lineHeader, sizeof(lineHeader)) != EOF)
	{
		if (strcmp(lineHeader, "v") == 0)
		{
			Vertex vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "f") == 0)
		{
			int vertIndex[3], normalIndex[3];
			fscanf_s(file, "%d//%d %d//%d %d//%d\n", &vertIndex[0], &normalIndex[0], &vertIndex[1], &normalIndex[1], &vertIndex[2], &normalIndex[2]);
			indices.push_back(vertIndex[0] - 1);
			indices.push_back(vertIndex[1] - 1);
			indices.push_back(vertIndex[2] - 1);
		}
		else
		{
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}

	fclose(file);


	typedef std::pair<HE_Vertex*, HE_Vertex*> vertex_pair;

	struct vertexPairHash
	{
		inline size_t operator()(const vertex_pair& pair) const
		{
			return std::hash<size_t>()(reinterpret_cast<size_t>(pair.first)) ^
				std::hash<size_t>()(reinterpret_cast<size_t>(pair.second));
		}
	};

	struct vertexPairEquality
	{
		inline bool operator()(const vertex_pair& pair1, const vertex_pair& pair2) const
		{
			return (pair1.first == pair2.first) && (pair1.second == pair2.second);
		}
	};

	std::unordered_map<vertex_pair, HE_Edge*, vertexPairHash, vertexPairEquality> edgeMap;

	const size_t numFaces = indices.size() / 3;
	const size_t numEdges = numFaces * 3;


	std::unique_ptr<HE_Vertex[]> heVertices;
	std::unique_ptr<HE_Face[]> heFaces;
	std::unique_ptr<HE_Edge[]> heEdges;

	heVertices = std::make_unique<HE_Vertex[]>(vertices.size());
	heFaces = std::make_unique<HE_Face[]>(numFaces);
	heEdges = std::make_unique<HE_Edge[]>(numEdges);

	for (ptrdiff_t i = 0; i < vertices.size(); i++)
	{
		heVertices[i].actualVertex = &vertices[i];
		heVertices[i].actualVertex->id = i;
	}


	for (ptrdiff_t indexId = 0; indexId < indices.size(); indexId += 3)
	{
		unsigned index1 = indices[indexId];
		unsigned index2 = indices[indexId + 1];
		unsigned index3 = indices[indexId + 2];

		HE_Face *currentFace = &heFaces.get()[indexId / 3];

		HE_Vertex *verts[] = { &heVertices.get()[index1], &heVertices.get()[index2], &heVertices.get()[index3] };

		{
			HE_Edge *firstEdge = nullptr, *previousEdge = nullptr;

			for (ptrdiff_t i = 0; i < 3; i++)
			{
				HE_Vertex *startVertex = verts[i];
				HE_Vertex *endVertex = verts[(i + 1) % 3];
				HE_Edge *newEdge = &heEdges.get()[indexId + i];

				newEdge->face = currentFace;
				newEdge->vertex = endVertex;
				newEdge->pair = nullptr;

				assert(startVertex != endVertex);

				startVertex->edge = newEdge;

				if (!firstEdge)
				{
					firstEdge = newEdge;
					currentFace->edge = newEdge;
					currentFace->vertex = startVertex;
					assert(startVertex->edge->vertex != currentFace->vertex);
				}
				if (previousEdge)
					previousEdge->next = newEdge;

				previousEdge = newEdge;

				assert(startVertex->edge->vertex != startVertex);

				edgeMap[vertex_pair(startVertex, endVertex)] = newEdge;
			}

			previousEdge->next = firstEdge;

			assert(previousEdge->next != previousEdge);
		}
	}

	for (const auto& kvPair : edgeMap)
	{
		const vertex_pair& key = kvPair.first;
		HE_Edge * const & currentEdge = kvPair.second;

		if (currentEdge->pair)
			continue;

		const vertex_pair oppositeKey = vertex_pair(key.second, key.first);

		if (edgeMap.find(oppositeKey) != edgeMap.end())
		{
			HE_Edge * const & otherEdge = edgeMap[oppositeKey];
			currentEdge->pair = otherEdge;
			otherEdge->pair = currentEdge;
			assert(currentEdge->vertex != otherEdge->vertex);
		}
	}

	//Verify that there are no boundary edges

	for (const auto& kvPair : edgeMap)
	{
		const vertex_pair& key = kvPair.first;
		HE_Edge * const & currentEdge = kvPair.second;
		if (currentEdge->pair)
			continue;

		std::cout << "\n\nThis mesh contains boundary edges, which are not supported by the half-edge data structure\n\n";
		std::cin.get();
		std::exit(0);
		return 0;
	}

	std::unique_ptr<Vertex[]> newVertices;

	for (int iter = 0; iter < 100; iter++)
	{
		newVertices = std::make_unique<Vertex[]>(vertices.size());

		for (int i = 0; i < vertices.size(); i++)
		{
			const HE_Vertex& oldVertex = heVertices[i];
			Vertex& newVertex = newVertices[i];
			newVertex.id = oldVertex.actualVertex->id;

			float sumX, sumY, sumZ;
			sumX = sumY = sumZ = 0;
			int numNeighbors = 0;

			for (auto neighbor : oldVertex.neighboringVertices())
			{
				numNeighbors++;
				sumX += neighbor->actualVertex->x;
				sumY += neighbor->actualVertex->y;
				sumZ += neighbor->actualVertex->z;
			}

			if (numNeighbors > 0)
			{
				newVertex.x = sumX / numNeighbors;
				newVertex.y = sumY / numNeighbors;
				newVertex.z = sumZ / numNeighbors;
			}
			else
			{
				newVertex = *oldVertex.actualVertex;
			}
		}

		::memcpy(&vertices[0], newVertices.get(), vertices.size() * sizeof(Vertex));
	}

	std::ostringstream strOut;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		const Vertex& vert = newVertices[i];
		strOut << "v " << vert.x << " " << vert.y << " " << vert.z << "\n";
	}

	for (size_t i = 0; i < indices.size(); i += 3)
	{
		strOut << "f " << (indices[i] + 1) << " " << (indices[i + 1] + 1) << " " << (indices[i + 2] + 1) << "\n";
	}

	std::wstring outPath = L"C:\\Users\\Concordia\\Desktop\\smooth.obj";

	std::ofstream fOut(outPath, std::ios::trunc);
	if (!fOut.is_open())
	{
		std::cerr << (L"Unable to open file: " + outPath).c_str();
	}
	else
	{
		fOut << strOut.str();
		fOut.close();
	}

	return 0;
}

