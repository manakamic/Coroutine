#pragma once
#include <coroutine>
#include <optional>
#include <stdexcept> // std::runtime_error
#include <utility> // std::exchange
#include <exception> // std::exception_ptr, std::current_exception, std::rethrow_exception

// ===================================================================================
// Task �� int �ł͂Ȃ� template �ōD���Ȗ߂�l��������l�ɂ��� class
// ===================================================================================
namespace TaskT {

    template <typename T>
    class [[nodiscard]] Task { // [[nodiscard]] : C++17
    public:
        struct promise_type; // �O���錾
        using Handle = std::coroutine_handle<promise_type>;

        struct promise_type {
            Task get_return_object() { return Task<T>{ Handle::from_promise(*this) }; }
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            // Coroutine ���I���������ɌĂ΂�� T �� move ���Ď󂯎��
            void return_value(T val) noexcept { value = std::move(val); }
            // std::current_exception : C++11
            void unhandled_exception() noexcept { exception = std::current_exception(); }

            // T �̒l���L�����ǂ������������邽�߂� std::optional<T>
            std::optional<T> value; // std::optional : C++17
            std::exception_ptr exception = nullptr; // std::exception_ptr : C++11
        };

        // �R���X�g���N�^�A���[�u�R���X�g���N�^�A���[�u������Z�q�A�f�X�g���N�^
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
        // �R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~
        Task(const Task&) = delete;
        Task& operator=(const Task&) = delete;

        void resume() const { if (handle && !handle.done()) { handle.resume(); } }
        bool is_done() const { return !handle || handle.done(); }
        [[nodiscard]] T get() const {
            if (!is_done()) {
                throw std::runtime_error("Task is not done yet.");
            }

            // �\�������� : C++17
            auto& [value, exception] = handle.promise();

            if (exception) {
                std::rethrow_exception(exception);
            }

            if (value.has_value()) {
                // �l�����o������Aoptional���N���A����
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
