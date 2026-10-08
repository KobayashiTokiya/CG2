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
	modelData_.material.textureIndex = TextureManager::GetInstance()->GetSrvIndex(modelData_.material.textureFilePath);
}

void Model::Draw()
{
	ID3D12GraphicsCommandList* commandList = modelCommon_->GetDxCommon()->GetCommandList();
	// VertexBufferViewを設定
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	// マテリアルCBufferの場所を設定 (CBV)
	commandList->SetGraphicsRootConstantBufferView(0, materialResource.Get()->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定 (Table)
	//commandList->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(modelData_.material.textureIndex));
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
		else if (identifier == "f")
		{
			std::vector<VertexData> faceVertices;
			std::string vertexDefinition;

			// 1行に含まれる頂点（v/vt/vn）をすべて取得
			while (s >> vertexDefinition)
			{
				std::istringstream v(vertexDefinition);
				std::string indexStr;

				uint32_t posIndex = 0;
				uint32_t uvIndex = 0;
				uint32_t normIndex = 0;

				// 位置/UV/法線インデックスを解析（空文字チェック付き）
				if (std::getline(v, indexStr, '/') && !indexStr.empty())
				{
					posIndex = std::stoi(indexStr);
				}
				if (std::getline(v, indexStr, '/') && !indexStr.empty())
				{
					uvIndex = std::stoi(indexStr);
				}
				if (std::getline(v, indexStr, '/') && !indexStr.empty())
				{
					normIndex = std::stoi(indexStr);
				}

				// 要素を取得（1-based index を 0-based に変換）
				Vector4 position = (posIndex > 0) ? positions[posIndex - 1] : Vector4(0.0f, 0.0f, 0.0f, 1.0f);
				Vector2 texcoord = (uvIndex > 0) ? texcoords[uvIndex - 1] : Vector2(0.0f, 0.0f);
				Vector3 normal = (normIndex > 0) ? normals[normIndex - 1] : Vector3(0.0f, 0.0f, 1.0f);

				// 座標系・UVの変換
				position.x *= -1.0f;
				normal.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;

				faceVertices.push_back({ position, texcoord, normal });
			}

			// 多角形（4頂点以上）を三角形に分割（Triangle Fan）して時計回り順で登録
			for (size_t i = 1; i + 1 < faceVertices.size(); ++i)
			{
				modelData.vertices.push_back(faceVertices[i + 1]);
				modelData.vertices.push_back(faceVertices[i]);
				modelData.vertices.push_back(faceVertices[0]);
			}
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
	DirectXCommon::GetInstance();

	//マテリアルリソースを作る
	materialResource = DirectXCommon::GetInstance()->CreateBufferResource(sizeof(Material));

	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();
}
#pragma endregion