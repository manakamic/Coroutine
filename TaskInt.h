#pragma once
#include <coroutine>
#include <optional>
#include <stdexcept> // std::runtime_error
#include <utility> // std::exchange
#include <exception> // std::exception_ptr, std::current_exception, std::rethrow_exception

// ===================================================================================
// coroutine を使用して int を扱う Task
// ===================================================================================
namespace TaskInt {

    class [[nodiscard]] Task { // [[nodiscard]] : C++17
    public:
        struct promise_type; // 前方宣言
        // std::coroutine_handle : C++20
        using Handle = std::coroutine_handle<promise_type>;

        struct promise_type {
            Task get_return_object() { return Task{ Handle::from_promise(*this) }; }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_value(int val) noexcept { value = val; } // intなのでムーブは不要(コピーで十分)
            // std::current_exception : C++11
            void unhandled_exception() noexcept { exception = std::current_exception(); }

            std::optional<int> value; // std::optional : C++17
            std::exception_ptr exception = nullptr; // std::exception_ptr : C++11
        };

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
        [[nodiscard]] int get() const
        {
            if (!is_done()) {
                throw std::runtime_error("Task is not done yet.");
            }

            // 構造化束縛 : C++17
            auto& [value, exception] = handle.promise(); // promise : promise_type のこと

            if (exception) {
                std::rethrow_exception(exception);
            }

            if (value.has_value()) {
                // 値を取り出した後、optionalをクリアする
                const auto result = *value; // std::optional の値を取り出す場合は * を使う
                value.reset(); // optional をクリア

                return result;
            }
            else {
                throw std::runtime_error("Task completed without a value.");
            }
        }

        Handle handle;
    };
}
