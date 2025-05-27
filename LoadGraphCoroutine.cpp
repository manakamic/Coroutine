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
        return false; // �^�X�N���Ȃ��ꍇ�͏I��
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
                second.reset(); // optional ���N���A
            }
            catch (const std::exception& e) {
                // �G���[
                throw std::runtime_error("task.get()");
            }
        }
        else {
            task.resume();
            is_loaded = false;
        }
    }

    return is_loaded; // �ǂݍ��݊������ǂ�����Ԃ�
}

TaskT::Task<int> LoadGraphCoroutine::Start(const std::string& filePath) {
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
            co_await YieldFrame{};
        }
        else if (status == -1) { // �ǂݍ��݃G���[
            co_return ERROR_VALUE; // �G���[�R�[�h��Ԃ�
        }
    }
}
