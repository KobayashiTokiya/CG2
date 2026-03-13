#include "Model.h"
#include "TextureManager.h"
#include "ModelCommon.h"

void Model::Initialize(ModelCommon* modelCommon, const std::string directorypath, const std::string& filename)
{
	//ModelCommonのポインタを引数からメンバ変数に記録する
	modelCommon_ = modelCommon;

	//モデル読み込み
	modelData_ = LoadObjFile(directorypath,filename);

	// 頂点データの初期化
	CreateVertexData();

	// マテリアルの初期化
	CreateMaterialData();

	// テクスチャ読み込み
	// Objファイルから読み取ったテクスチャのファイルパスを使って読み込む
	TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);

	// テクスチャ番号を取得して、メンバ変数に書き込む
	// modelData_ の中の material (MaterialData構造体) に textureIndex を保存します
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData_.material.textureFilePath);
}

void Model::Draw()
{
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDxCommon()->GetCommandList();
	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	// マテリアルCBufferの場所を設定 (CBV)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource.Get()->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定 (Table)
	commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex));
	// 描画(DrawCall/ドローコール)
	commandList->DrawInstanced(UINT(modelData_.vertices.size()), 1, 0, 0);
}

Model::MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	//1.中で必要となる変数の宣言
	MaterialData materialData;                         //構築するMaterialData
	std::string line;                                  // ファイルから読んで1行を格納するもの
	//とりあえず開けなかったら止める

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());

	//3.実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		//identifierに応じた処理
		if (identifier == "map_Kd")
		{
			std::string textureFilename;
			s >> textureFilename;
			//連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	//4.MaterialDataを返す
	return materialData;

}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	//1.中に必要となる変数の宣言
	ModelData modelData;           //構築するModelData
	std::vector<Vector4>positions; //位置
	std::vector<Vector3>normals;   //法線
	std::vector<Vector2>texcoords; //テクスチャ座標
	std::string line;              //ファイルから読んだ1行を格納するもの

	//2.ファイルを開く
	std::ifstream file(directoryPath + "/" + filename);//ファイルを開く
	assert(file.is_open());//とりあえず開けなかったら止める

	//3.実際のファイルを読み、ModelDataを構築していく
	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;//先頭の識別子を読む


		// identifierに応じた処理
		//頂点情報を読む
		if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt")
		{
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		//三角形を作る
		else if (identifier == "f")
		{
			VertexData triangle[3];
			//面を三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
				//頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/');// /区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				//要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position,texcoord,normal };
				//modelData.vertices.push_back(vertex);

				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;

				triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順で登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib")
		{
			//materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			//基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	//4.ModelDataを返す
	return modelData;

}

#pragma region 初期化用データ作成関数群
void Model::CreateVertexData()
{
	DirectXCommon* dxCommon = modelCommon_->GetDxCommon();

	//モデルの実際の頂点数を取得する
	UINT vertexCount = static_cast<UINT>(modelData_.vertices.size());

	//VertexResourceを作る
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * vertexCount);

	//VertexBufferViewを作成する(値を設定するだけ)
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData) * vertexCount;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));

	//読み込んだOBJのデータをGPUにコピー(転送)する
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * vertexCount);
}

void Model::CreateMaterialData()
{
	DirectXCommon* dxCommon = modelCommon_->GetDxCommon();

	//マテリアルリソースを作る
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));

	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();
}
#pragma endregion