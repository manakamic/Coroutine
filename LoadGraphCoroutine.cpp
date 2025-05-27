#include "DxLib.h"
#include "LoadGraphCoroutine.h"

void LoadGraphCoroutine::Clear() {
    task_list.clear();
    file_list.clear();
}

void LoadGraphCoroutine::SetFilePath(const std::string& filePath) {
    file_list[filePath] = ERROR_VALUE;
    task_list.emplace_back(std::make_unique<TaskMap>(std::make_pair(filePath, Start(filePath))));
}

int LoadGraphCoroutine::GetHandle(const std::string& filePath) {
    if (!file_list.contains(filePath)) { // contains : C++20
        return ERROR_VALUE;
    }

    return  file_list[filePath];
}

bool LoadGraphCoroutine::Process() {
    if (task_list.empty()) {
        return false; // タスクがない場合は終了
    }

    auto is_loaded = true;

    for (auto& handle : task_list) {
        auto& second = handle->second;

        if (!second.has_value()) {
            continue;
        }

        auto& task = second.value();

        if (task.is_done()) {
            try {
                file_list[handle->first] = task.get();
                second.reset(); // optional をクリア
            }
            catch (const std::exception& e) {
                // エラー
                throw std::runtime_error("task.get()");
            }
        }
        else {
            task.resume();
            is_loaded = false;
        }
    }

    return is_loaded; // 読み込み完了かどうかを返す
}

TaskT::Task<int> LoadGraphCoroutine::Start(const std::string& filePath) {
    // DxLibの非同期読み込みを開始
    SetUseASyncLoadFlag(TRUE);
    int handle = LoadGraph(filePath.c_str());
    SetUseASyncLoadFlag(FALSE);

    // ハンドルの取得自体に失敗した場合 (ファイルパスが異常など)
    if (handle == -1) {
        co_return ERROR_VALUE; // エラーコードを返す
    }

    // 読み込み完了まで待機
    while (true) {
        int status = CheckHandleASyncLoad(handle);

        if (status == FALSE) { // 読み込み完了
            co_return handle;
        }
        else if (status == TRUE) { // 読み込み中
            // コルーチンを中断し、制御をメインループに戻す
            // 次のフレームでメインループから resume() されるのを待つ
            co_await YieldFrame{};
        }
        else if (status == -1) { // 読み込みエラー
            co_return ERROR_VALUE; // エラーコードを返す
        }
    }
}
