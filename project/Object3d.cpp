#include "Object3d.h"
#include "Object3dCommon.h"
#include "TextureManager.h"

void Object3d::Initialize(Object3dCommon* object3dCommon)
{
	//引数で受け取ってメンバ変数に記録する
	this->object3dCommon = object3dCommon;

	//モデル読み込み
	modelData = LoadObjFile("Resource", "plane.obj");

	//初期化の呼び出し
	CreateVertexData();           //頂点データ
	CreateMaterialData();         //マテリアルデータ
	CreateTransformationData();   //座標変換行列データ
	CreateDirectionalLightData(); //平行光源データ

	//.objの参照しているテクスチャファイル読み込み
	TextureManager::GetInstance()->LoadTexture(modelData.material.textureFilePath);
	//読み込んだテクスチャの番号を取得
	modelData.material.textureIndex = TextureManager::GetInstance()->GetTextureIndexByFilePath(modelData.material.textureFilePath);

	//Transform変数を作る
	transform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} };
	cameraTransform = { {1.0f,1.0f,1.0f},{0.3f,0.0f,0.0f},{0.0f,4.0f,-10.0f} };
}

void Object3d::Update()
{
	// [TransformからWorldMatrixを作る]
	// ※MakeAffineMatrixはMatrix.hにある関数名に合わせてください
	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	// [cameraTransformからcameraMatrixを作る]
	Matrix4x4 cameraMatrix = MatrixMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);

	// [cameraMatrixからviewMatrixを作る] (逆行列にする)
	Matrix4x4 viewMatrix = MatrixMath::Inverse(cameraMatrix);

	// [ProjectionMatrixを作って透視投影行列を書き込む]
	// 0.45f:視野角, 1280/720:アスペクト比, 0.1f:近平面, 100.0f:遠平面
	Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(0.45f, 1280.0f / 720.0f, 0.1f, 100.0f);

	// --- 行列の合成と転送 ---

	// [transformationMatrixData->WVP = worldMatrix*viewMatrix*projectionMatrix]
	// C++では * ではなく Multiply 関数を使って掛け合わせます
	Matrix4x4 worldViewProjectionMatrix =MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

	// 定数バッファに書き込む
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	// [transformationMatrixData->World = worldMatrix]
	transformationMatrixData->World = worldMatrix;
}

void Object3d::Draw()
{

}

Object3d::MaterialData Object3d::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
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

Object3d::ModelData Object3d::LoadObjFile(const std::string& directoryPath, const std::string& filename)
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
void Object3d::CreateVertexData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//VertexResourceを作る
	vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * 6);

	//VertexBufferViewを作成する(値を設定するだけ)
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(VertexData)*6;
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//VertexResourceにデータを書き込むためのアドレスを取得してvertexDataに割り当てる
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
}

void Object3d::CreateMaterialData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//マテリアルリソースを作る
	materialResource = dxCommon->CreateBufferResource(sizeof(Material));

	//マテリアルリソースにデータを書き込むためのアドレスを取得してmaterialDataに割り当てる
	materialResource->Map(0,nullptr,reinterpret_cast<void**>(&materialData));

	//マテリアルデータの初期値を書き込む
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();
}

void Object3d::CreateTransformationData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//座標変換行列リソースを作る
	transformationResource = dxCommon->CreateBufferResource(sizeof(TransformationMatrix));

	//座標変換行列リソースにデータを書き込むためのアドレスを取得してtransformationMatrixDataに割り当てる
	transformationResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));

	//単位行列を書き込んでおく
	transformationMatrixData->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixData->World = MatrixMath::MakeIdentity4x4();
}

void Object3d::CreateDirectionalLightData()
{
	DirectXCommon* dxCommon = object3dCommon->GetDxCommon();

	//平行光源リソースを作る
	directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));

	//平行光源リソースにデータを書き込むためのアドレスを取得してdirectionalLightDataに割り当てる
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));

	//デフォルト値を書き込んでおく
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);  // 真下に向いて照らす
	directionalLightData->intensity = 1.0f;                        // 明るさ（1.0が基本）
}
#pragma endregion