#include "DxLib.h"
#include "LoadGraphAsync.h"

void LoadGraphAsync::Clear() {
    task_list.clear();
    file_list.clear();
}

void LoadGraphAsync::SetFilePath(const std::string& filePath) {
    file_list[filePath] = ERROR_VALUE;
    task_list.emplace_back(std::make_unique<TaskMap>(std::make_pair(filePath, Start(filePath))));
}

int LoadGraphAsync::GetHandle(const std::string& filePath) {
    if (!file_list.contains(filePath)) { // contains : C++20
        return ERROR_VALUE;
    }

    return  file_list[filePath];
}

bool LoadGraphAsync::Process() {
    if (task_list.empty()) {
        return false; // タスクがない場合は終了
    }

    auto is_loaded = true;

    for (auto& handle : task_list) {
        auto& second = handle->second;

        if (!second.has_value()) { // has_value : std::optional : C++17
            continue;
        }

        auto& task = second.value(); // value : std::optional : C++17

        if (task.is_done()) {
            // try-catch で例外をキャッチ
            try {
                file_list[handle->first] = task.get();
                second.reset(); // optional をクリア : reset : std::optional : C++17
            }
            catch (const std::exception& e) {
                // エラー
                throw std::runtime_error("task.get()");
            }
        }
        else {
            task.resume(); // コルーチンを再開
            is_loaded = false;
        }
    }

    return is_loaded; // 読み込み完了かどうかを返す
}

Task LoadGraphAsync::Start(const std::string& filePath) {
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
            co_await YieldFrame{}; // awaitable
        }
        else if (status == -1) { // 読み込みエラー
            co_return ERROR_VALUE; // エラーコードを返す
        }
    }
}
