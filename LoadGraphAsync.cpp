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
        return false; // �^�X�N���Ȃ��ꍇ�͏I��
    }

    auto is_loaded = true;

    for (auto& handle : task_list) {
        auto& second = handle->second;

        if (!second.has_value()) { // has_value : std::optional : C++17
            continue;
        }

        auto& task = second.value(); // value : std::optional : C++17

        if (task.is_done()) {
            // try-catch �ŗ�O���L���b�`
            try {
                file_list[handle->first] = task.get();
                second.reset(); // optional ���N���A : reset : std::optional : C++17
            }
            catch (const std::exception& e) {
                // �G���[
                throw std::runtime_error("task.get()");
            }
        }
        else {
            task.resume(); // �R���[�`�����ĊJ
            is_loaded = false;
        }
    }

    return is_loaded; // �ǂݍ��݊������ǂ�����Ԃ�
}

Task LoadGraphAsync::Start(const std::string& filePath) {
    // DxLib�̔񓯊��ǂݍ��݂��J�n
    SetUseASyncLoadFlag(TRUE);
    int handle = LoadGraph(filePath.c_str());
    SetUseASyncLoadFlag(FALSE);

    // �n���h���̎擾���̂Ɏ��s�����ꍇ (�t�@�C���p�X���ُ�Ȃ�)
    if (handle == -1) {
        co_return ERROR_VALUE; // �G���[�R�[�h��Ԃ�
    }

    // �ǂݍ��݊����܂őҋ@
    while (true) {
        int status = CheckHandleASyncLoad(handle);

        if (status == FALSE) { // �ǂݍ��݊���
            co_return handle;
        }
        else if (status == TRUE) { // �ǂݍ��ݒ�
            // �R���[�`���𒆒f���A��������C�����[�v�ɖ߂�
            // ���̃t���[���Ń��C�����[�v���� resume() �����̂�҂�
            co_await YieldFrame{}; // awaitable
        }
        else if (status == -1) { // �ǂݍ��݃G���[
            co_return ERROR_VALUE; // �G���[�R�[�h��Ԃ�
        }
    }
}
