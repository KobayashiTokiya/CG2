#pragma once

#include <string>
#include <vector>
#include "externals/nlohmann/json.hpp"
#include "Vector.h"

// --- コライダー情報 ---
struct ColliderData
{
	std::string type;
	Vector3 center;
	Vector3 size;
};

// --- トランスフォーム情報 ---
struct TransformData
{
	Vector3 translation;
	Vector3 rotation;
	Vector3 scaling;
};

// --- オブジェクト1個分のデータ ---
struct ObjectData
{
	std::string type;        // MESH,LIGHT,CAMERA
	std::string name;        // 立方体,球
	TransformData transform;
	std::string fileName;    // cube,sphere
	ColliderData collider;   // コライダー情報
};

// --- レベルデータ全体 ---
struct LevelData
{
	std::string name;
	std::vector<ObjectData> objects;
};

// --- レベルデータローダークラス ---
class LevelLoader
{
public:
	// JSONファイルを読み込んで LevelData を動的生成して返す
	static LevelData* LoadLevelFile(const std::string& fileName);
private:
	// オブジェクト1個分の読み込み処理
	static void LoadObject(nlohmann::json& object, LevelData* levelData);
};