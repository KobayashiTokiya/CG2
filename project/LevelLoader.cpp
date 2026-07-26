#include "LevelLoader.h"
#include <fstream>
#include <cassert>
#include <Windows.h> 

void LevelLoader::LoadObject(nlohmann::json& object, LevelData* levelData)
{
	assert(object.contains("type"));

	// 種別を取得
	std::string type = object["type"].get<std::string>();

	// MESH の場合の処理
	if (type.compare("MESH") == 0)
	{
		// 要素追加
		levelData->objects.emplace_back(ObjectData{});
		ObjectData& objectData = levelData->objects.back();

		objectData.type = type;
		if (object.contains("name"))
		{
			objectData.name = object["name"].get<std::string>();
		}

		// ファイル名
		if (object.contains("file_name"))
		{
			objectData.fileName = object["file_name"].get<std::string>();
		}

		// トランスフォームのパラメータ読み込み
		if (object.contains("transform"))
		{
			nlohmann::json& transform = object["transform"];

			// 平行移動
			objectData.transform.translation.x = (float)transform["translation"][0];
			objectData.transform.translation.y = (float)transform["translation"][2];
			objectData.transform.translation.z = (float)transform["translation"][1];

			// 回転角（マイナス処理）
			objectData.transform.rotation.x = -(float)transform["rotation"][0];
			objectData.transform.rotation.y = -(float)transform["rotation"][2];
			objectData.transform.rotation.z = -(float)transform["rotation"][1];

			// スケーリング
			objectData.transform.scaling.x = (float)transform["scaling"][0];
			objectData.transform.scaling.y = (float)transform["scaling"][2];
			objectData.transform.scaling.z = (float)transform["scaling"][1];
		}

		// コライダーのパラメータ読み込み
		if (object.contains("collider"))
		{
			nlohmann::json& collider = object["collider"];
			objectData.collider.type = collider["type"].get<std::string>();

			objectData.collider.center.x = (float)collider["center"][0];
			objectData.collider.center.y = (float)collider["center"][2];
			objectData.collider.center.z = (float)collider["center"][1];

			objectData.collider.size.x = (float)collider["size"][0];
			objectData.collider.size.y = (float)collider["size"][2];
			objectData.collider.size.z = (float)collider["size"][1];
		}
	}

	if (object.contains("children"))
	{
		for (nlohmann::json& child : object["children"])
		{
			LoadObject(child, levelData);
		}
	}
}

LevelData* LevelLoader::LoadLevelFile(const std::string& fileName)
{
	// 1. ファイルパスの設定 ("Resource/scene.json")
	const std::string kDefaultBaseDirectory = "Resource/levels/";
	const std::string kExtension = ".json";
	const std::string fullPath = kDefaultBaseDirectory + fileName + kExtension;

	// 2. ファイルを開く
	std::ifstream file;

	file.open(fullPath);
	if (file.fail())
	{
		assert(0);
	}

	// 3. JSONパース
	nlohmann::json deserialized;
	file >> deserialized;

	// 4. データチェック
	assert(deserialized.is_object());
	assert(deserialized.contains("name"));
	assert(deserialized["name"].is_string());

	std::string name = deserialized["name"].get<std::string>();
	assert(name.compare("scene") == 0);

	// 5. データ格納用インスタンス生成
	LevelData* levelData = new LevelData();

	// "objects" の全オブジェクトを走査
	for (nlohmann::json& object : deserialized["objects"])
	{
		LoadObject(object, levelData);
	}

	return levelData;
}