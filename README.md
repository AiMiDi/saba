## Path

Saba uses UTF-8 as the path string.

## Load model

`saba::MMDModel` is a MMD(PMD/PMX) model.

```cpp
// Load model.
std::shared_ptr<saba::MMDModel> mmdModel;
std::string mmdDataPath = "";	// Set MMD data path(default toon texture path).
std::string ext = saba::PathUtil::GetExt(modelPath);
if (ext == "pmd")
{
	auto pmdModel = std::make_unique<saba::PMDModel>();
	if (!pmdModel->Load(modelPath, mmdDataPath))
	{
		std::cout << "Load PMDModel Fail.\n";
		return false;
	}
	mmdModel = std::move(pmdModel);
}
else if (ext == "pmx")
{
	auto pmxModel = std::make_unique<saba::PMXModel>();
	if (!pmxModel->Load(modelPath, mmdDataPath))
	{
		std::cout << "Load PMXModel Fail.\n";
		return false;
	}
	mmdModel = std::move(pmxModel);
}
else
{
	std::cout << "Unsupported Model Ext : " << ext << "\n";
	return false;
}
```

For `mmdDataPath`, specify the data path of default MMD toon texture.

## Load animation.

`saba::VMDAnimation` is a MMD(VMD) animation.

```cpp
// Load animation.
auto vmdAnim = std::make_unique<saba::VMDAnimation>();
if (!vmdAnim->Create(mmdModel))
{
	std::cout << "Create VMDAnimation Fail.\n";
	return false;
}
for (const auto& vmdPath : vmdPaths)
{
	saba::VMDFile vmdFile;
	if (!saba::ReadVMDFile(&vmdFile, vmdPath.c_str()))
	{
		std::cout << "Read VMD File Fail.\n";
		return false;
	}
	if (!vmdAnim->Add(vmdFile))
	{
		std::cout << "Add VMDAnimation Fail.\n";
		return false;
	}
}
```

## Update animation

```cpp
// Initialize pose.
{
	// Sync physics animation.
	mmdModel->InitializeAnimation();
	vmdAnim->SyncPhysics((float)animTime * 30.0f);
}

// Update animation(animation loop).
{
	// Update animation.
	mmdModel->BeginAnimation();
	// vmdAnim->Evaluate((float)animTime * 30.0f);
	// mmdModel->UpdateMorphAnimation(1.0f / 60.0f);
	// mmdModel->UpdateNodeAnimation(false);
	// mmdModel->UpdatePhysicsAnimation();
	// mmdModel->UpdateNodeAnimation(true);
	mmdModel->UpdateAllAnimation(vmdAnim.get(), (float)animTime * 30.0f, 1.0f / 60.0f);
	mmdModel->EndAnimation();

	// Update vertex.
	mmdModel->Update();
}
```

Execute `mmdModel->InitializeAnimation()` and `vmdAnim->SyncPhysics()` only once.

`mmdModel->InitializeAnimation()` is initialize the animation.

`vmdAnim->SyncPhysics()` is sync the physical animation.

Reflecting the physical animation immediately may cause the model to be destroyed.

The animation loop looks like the following.

1. Updates morph, node, and physics animation at once. `mmdModel->UpdateAllAnimation()`
2. Update vertices. `mmdModel->Update()`

or

1. Evaluate VMD animation. `vmdAnim->Evaluate()`
2. Update morph animation. `mmdModel->UpdateMorphAnimation()`
3. Update node animation. `mmdModel->UpdateNodeAnimation(false)`
4. Update physics animation. `mmdModel->UpdatePhysicsAnimation()`
5. Update node animation after physics simulation. `mmdModel->UpdateNodeAnimation(true)`
6. Update vertices. `mmdModel->Update()`

## Get updated vertices

The updated vertices are obtained as follows.

```cpp
const glm::vec3* positions = mmdModel->GetUpdatePositions();
const glm::vec3* normals = mmdModel->GetUpdateNormals();
const glm::vec2* uvs = mmdModel->GetUpdateUVs();
```

## Get vertex indices

Vertex index size is either 1, 2, or 4 bytes. Get the index size with `mmdModel->GetIndexElementSize()`.

The vertex indices are obtained as follows.

```cpp
if (mmdModel->GetIndexElementSize() == 1)
{
	uint8_t* mmdIndices = (uint8_t*)mmdModel->GetIndices();
}
else if (mmdModel->GetIndexElementSize() == 2)
{
	uint16_t* mmdIndices = (uint16_t*)mmdModel->GetIndices();
}
else if (mmdModel->GetIndexElementSize() == 4)
{
	uint32_t* mmdIndices = (uint32_t*)mmdModel->GetIndices();
}
```

## Write OBJ file

```cpp
// Output OBJ file.
std::ofstream objFile;
objFile.open("output.obj");
if (!objFile.is_open())
{
	std::cout << "Open OBJ File Fail.\n";
	return false;
}
objFile << "# mmmd2obj\n";
objFile << "mtllib output.mtl\n";

// Write positions.
size_t vtxCount = mmdModel->GetVertexCount();
const glm::vec3* positions = mmdModel->GetUpdatePositions();
for (size_t i = 0; i < vtxCount; i++)
{
	objFile << "v " << positions[i].x << " " << positions[i].y << " " << positions[i].z << "\n";
}
const glm::vec3* normals = mmdModel->GetUpdateNormals();
for (size_t i = 0; i < vtxCount; i++)
{
	objFile << "vn " << normals[i].x << " " << normals[i].y << " " << normals[i].z << "\n";
}
const glm::vec2* uvs = mmdModel->GetUpdateUVs();
for (size_t i = 0; i < vtxCount; i++)
{
	objFile << "vt " << uvs[i].x << " " << uvs[i].y << "\n";
}

// Copy vertex indices.
std::vector<size_t> indices(mmdModel->GetIndexCount());
if (mmdModel->GetIndexElementSize() == 1)
{
	uint8_t* mmdIndices = (uint8_t*)mmdModel->GetIndices();
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i] = mmdIndices[i];
	}
}
else if (mmdModel->GetIndexElementSize() == 2)
{
	uint16_t* mmdIndices = (uint16_t*)mmdModel->GetIndices();
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i] = mmdIndices[i];
	}
}
else if (mmdModel->GetIndexElementSize() == 4)
{
	uint32_t* mmdIndices = (uint32_t*)mmdModel->GetIndices();
	for (size_t i = 0; i < indices.size(); i++)
	{
		indices[i] = mmdIndices[i];
	}
}
else
{
	return false;
}

// Write faces.
size_t subMeshCount = mmdModel->GetSubMeshCount();
const saba::MMDSubMesh* subMeshes = mmdModel->GetSubMeshes();
for (size_t i = 0; i < subMeshCount; i++)
{
	objFile << "\n";
	objFile << "usemtl " << subMeshes[i].m_materialID << "\n";

	for (size_t j = 0; j < subMeshes[i].m_vertexCount; j += 3)
	{
		auto vtxIdx = subMeshes[i].m_beginIndex + j;
		auto vi0 = indices[vtxIdx + 0] + 1;
		auto vi1 = indices[vtxIdx + 1] + 1;
		auto vi2 = indices[vtxIdx + 2] + 1;
		objFile << "f "
			<< vi0 << "/" << vi0 << "/" << vi0 << " "
			<< vi1 << "/" << vi1 << "/" << vi1 << " "
			<< vi2 << "/" << vi2 << "/" << vi2 << "\n";
	}
}
objFile.close();

// Write materials.
std::ofstream mtlFile;
mtlFile.open("output.mtl");
if (!mtlFile.is_open())
{
	std::cout << "Open MTL File Fail.\n";
	return false;
}

objFile << "# mmmd2obj\n";
size_t materialCount = mmdModel->GetMaterialCount();
const saba::MMDMaterial* materials = mmdModel->GetMaterials();
for (size_t i = 0; i < materialCount; i++)
{
	const auto& m = materials[i];
	mtlFile << "newmtl " << i << "\n";

	mtlFile << "Ka " << m.m_ambient.r << " " << m.m_ambient.g << " " << m.m_ambient.b << "\n";
	mtlFile << "Kd " << m.m_diffuse.r << " " << m.m_diffuse.g << " " << m.m_diffuse.b << "\n";
	mtlFile << "Ks " << m.m_specular.r << " " << m.m_specular.g << " " << m.m_specular.b << "\n";
	mtlFile << "d " << m.m_alpha << "\n";
	mtlFile << "map_Kd " << m.m_texture << "\n";
	mtlFile << "\n";
}
mtlFile.close();
```
