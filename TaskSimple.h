#pragma once
#include <coroutine>
#include <utility> // std::exchange

// ===================================================================================
// coroutine を使用して int を扱う シンプルな Task
// ===================================================================================
namespace TaskSimple {

    class [[nodiscard]] Task { // [[nodiscard]] : C++17
    public:
        struct promise_type {
            Task get_return_object() { return Task{ Handle::from_promise(*this) }; }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_value(int val) noexcept { value = val; }
            void unhandled_exception() noexcept {}

            int value{-1};
        };

        using Handle = std::coroutine_handle<promise_type>;

        // コンストラクタ、ムーブコンストラクタ、ムーブ代入演算子、デストラクタ
        Task(const Handle h) : handle(h) {}
        Task(Task&& other) noexcept : handle(std::exchange(other.handle, nullptr)) {}
        Task& operator=(Task&& other) noexcept {
            if (this != &other) {
                if (handle) handle.destroy();
                handle = std::exchange(other.handle, nullptr);
            }
            return *this;
        }
        ~Task() { if (handle) handle.destroy(); }
        // コピーコンストラクタとコピー代入演算子を禁止
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        // 実際に resume するのはこの関数ではなく Handle の resume
        void resume() const { if (handle && !handle.done()) { handle.resume(); } }
		bool is_done() const { return !handle || handle.done(); } // done : Coroutine が終わっているかどうか
        int get() const {
            if (!is_done()) {
                return -1;
            }

            return handle.promise().value;
        }

        Handle handle;
    };
}
