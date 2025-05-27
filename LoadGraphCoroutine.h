#pragma once
#include "Task.h"
#include <memory>
#include <string>
#include <vector>
#include <map>

class LoadGraphCoroutine {
private:
    LoadGraphCoroutine() = default; // インスタンス化を抑制

public:
    // static なのはインスタンス無しで動作させるため
    static void Clear();
    static void SetFilePath(const std::string& filePath);
    static int GetHandle(const std::string& filePat);
    static bool Process();

private:
    static TaskT::Task<int> Start(const std::string& filePath);

    using TaskInstance = std::optional<TaskT::Task<int>>;
    using TaskMap = std::pair<const std::string, TaskInstance>;
    using TaskHandle = std::unique_ptr<TaskMap>;

    inline static std::map<const std::string, int> file_list;
    inline static std::vector<TaskHandle> task_list;

    //
    // Awaiter 実装
    // (フレーム待機用)
    //
    struct YieldFrame {
        bool await_ready() noexcept { return false; }
        void await_suspend(std::coroutine_handle<>) noexcept {}
        void await_resume() noexcept {}
    };

    static constexpr auto ERROR_VALUE = -1;
};

