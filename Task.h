#pragma once
#include <coroutine>
#include <optional>
#include <stdexcept> // std::runtime_error
#include <utility> // std::exchange
#include <exception> // std::exception_ptr, std::current_exception, std::rethrow_exception

// ===================================================================================
// Task を int ではなく template で好きな戻り値を扱える様にした class
// ===================================================================================
namespace TaskT {

    template <typename T>
    class [[nodiscard]] Task { // [[nodiscard]] : C++17
    public:
        struct promise_type; // 前方宣言
        using Handle = std::coroutine_handle<promise_type>;

        struct promise_type {
            Task get_return_object() { return Task<T>{ Handle::from_promise(*this) }; }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            // Coroutine が終了した時に呼ばれる T を move して受け取る
            void return_value(T val) noexcept { value = std::move(val); }
            // std::current_exception : C++11
            void unhandled_exception() noexcept { exception = std::current_exception(); }

            // T の値が有効かどうかを処理するための std::optional<T>
            std::optional<T> value; // std::optional : C++17
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

        void resume() const { if (handle && !handle.done()) { handle.resume(); } }
        bool is_done() const { return !handle || handle.done(); }
        [[nodiscard]] T get() const {
            if (!is_done()) {
                throw std::runtime_error("Task is not done yet.");
            }

            // 構造化束縛 : C++17
            auto& [value, exception] = handle.promise();

            if (exception) {
                std::rethrow_exception(exception);
            }

            if (value.has_value()) {
                // 値を取り出した後、optionalをクリアする
                const auto result = std::move(*value);
                value.reset();

                return result;
            }
            else {
                throw std::runtime_error("Task completed without a value.");
            }
        }

        Handle handle;
    };
}
