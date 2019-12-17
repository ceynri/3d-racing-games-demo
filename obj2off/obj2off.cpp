
//OPENMESH
#define _USE_MATH_DEFINES
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

#ifdef _DEBUG
#pragma comment(lib, "OpenMeshCored.lib")
#pragma comment(lib, "OpenMeshToolsd.lib")
#else
#pragma comment(lib, "OpenMeshCore.lib")
#pragma comment(lib, "OpenMeshTools.lib")
#endif // _DEBUG


typedef OpenMesh::TriMesh_ArrayKernelT<> MyTriMesh;

using namespace OpenMesh;
using namespace std;

int main(int argc, char** argv)
{
	MyTriMesh mesh;
	IO::Options ropt;
	ropt += IO::Options::Binary;
	ropt += IO::Options::Swap;

	char filename[256];
	for (int i = 0; i<1; i++) {
		//Load Mesh with openmesh
		sprintf_s(filename, "../2017192023_BigProject/asset/model/obj/oildrum/oildrum.obj", i);
		//要转换的obj文件地址
		if (!IO::read_mesh(mesh, filename, ropt))
		{
			std::cerr << "Error loading mesh from file " << filename << std::endl;
			return false;
		}
		sprintf_s(filename, "../2017192023_BigProject/asset/model/off/result", i);
		//目的off文件地址
		if (!OpenMesh::IO::write_mesh(mesh, filename))
		{
			std::cerr << "Cannot write mesh to file " << filename << std::endl;
		}
		else
			cout << "successfully save mesh to " << filename << endl;
	}
	system("pause");
}