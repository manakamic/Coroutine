#pragma once
#define SIMPLE

#ifdef SIMPLE
#include "TaskSimple.h"
using Task = TaskSimple::Task;
// TaskSimple.h �� include ���Ă��Ȃ����̂�ǉ�
#include <optional>
#include <stdexcept> // std::runtime_error
#else
#include "TaskInt.h"
using Task = TaskInt::Task;
#endif

#include <memory>
#include <string>
#include <vector>
#include <map>

class LoadGraphAsync {
private:
    LoadGraphAsync() = default; // �C���X�^���X����}��

public:
    // static �Ȃ̂̓C���X�^���X�����œ��삳���邽��
    static void Clear();
    static void SetFilePath(const std::string& filePath);
    static int GetHandle(const std::string& filePat);
    static bool Process();

private:
    static Task Start(const std::string& filePath);

    using TaskInstance = std::optional<Task>;
    using TaskMap = std::pair<const std::string, TaskInstance>;
    using TaskHandle = std::unique_ptr<TaskMap>;

    // inline : C++17
    inline static std::map<const std::string, int> file_list;
    // std::optional : C++17
    inline static std::vector<TaskHandle> task_list;

    //
    // Awaiter ����
    // (�t���[���ҋ@�p)
    //
    struct YieldFrame {
        bool await_ready() noexcept { return false; }
        void await_suspend(std::coroutine_handle<>) noexcept {} // Type Erasure
        void await_resume() noexcept {}
    };

    static constexpr auto ERROR_VALUE = -1;
};
