#include "ResultScene.h"
#include "Input.h"
#include "SceneManager.h"
#include "DirectXCommon.h"

#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif

void ResultScene::Initialize()
{
    // 初期化処理（リザルト画面のUIや背景などの生成）
}

void ResultScene::Update()
{
#ifdef USE_IMGUI
    ImGuiManager::GetInstance()->Begin();
    // UI処理...
   
#endif

    // ENTERキーを押したらタイトルへ戻る（ループ完了）
    if (Input::GetInstance()->TriggerKey(DIK_RETURN))
    {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }
}

void ResultScene::Draw()
{
    DirectXCommon::GetInstance()->PreDraw();

    // 描画処理...

#ifdef USE_IMGUI
    ImGuiManager::GetInstance()->End();
    ImGuiManager::GetInstance()->Draw();
#endif

    DirectXCommon::GetInstance()->PostDraw();
}

void ResultScene::Finalize()
{
    // メモリ解放処理
}