//#define TEMPLATE

#include "DxLib.h"
#include <tchar.h>

// Coroutine �ƌ����ʖ��� ���\�b�h���������@�\��
// LoadGraphCoroutine �� LoadGraphAsync �œ����O��
// �����܂ōu�`�� 2 Class ��؂�ւ��Č����邾���̑Ή�
// �{���Ȃ�ǂ��Ȃ����@
#ifdef TEMPLATE
#include "LoadGraphCoroutine.h"
using Coroutine = LoadGraphCoroutine;
#else
#include "LoadGraphAsync.h"
using Coroutine = LoadGraphAsync;
#endif

namespace {
    constexpr auto WINDOW_TITLE = _T("C++20 Coroutine");
    constexpr auto SCREEN_WIDTH = 1280;
    constexpr auto SCREEN_HEIGHT = 720;
    constexpr auto SCREEN_DEPTH = 32;
}

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    auto window_mode = FALSE;

#ifdef _DEBUG
    window_mode = TRUE;
#endif

    SetMainWindowText(WINDOW_TITLE);

    ChangeWindowMode(window_mode);

    SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_DEPTH);

    if (DxLib_Init() == -1) {
        return -1;
    }

    std::vector<std::string> file_paths = {
        _T("res/none.jpg"), _T("res/01.jpg"), _T("res/02.jpg"), _T("res/03.jpg"),
        _T("res/04.jpg"), _T("res/05.jpg"), _T("res/06.jpg"), _T("res/07.jpg")
    };

    for (const auto& file_path : file_paths) {
        Coroutine::SetFilePath(file_path);
    }

    std::vector<int> handles;
    auto index = 0;
    auto input_space = 0;
    auto counter = 0;
    const auto str_color = GetColor(255, 255, 255);
    // ���C�����[�v�����X�b�L��������ׂɃL�[���͂̏����̓����_�ōs��
    auto input_process = [&index, &input_space, &handles]() -> bool {
        if (CheckHitKey(KEY_INPUT_ESCAPE)) {
            return false;
        }

        const auto now_space = CheckHitKey(KEY_INPUT_SPACE);

        // �P�Ȃ�X�y�[�X�L�[�̃G�b�W����
        if (input_space == 0 && now_space == 1) {
            index++;

            if (index >= handles.size()) {
                index = 0;
            }
        }

        input_space = now_space;

        return true;
    };

    while (ProcessMessage() != -1) {
        const auto loading = handles.size() != file_paths.size();

        if (loading) {
            if (Coroutine::Process()) {
                for (const auto& file_path : file_paths) {
                    handles.emplace_back(Coroutine::GetHandle(file_path));
                }
            }

            ++counter;
        }
        else {
            if (!input_process()) {
                break;
            }
        }

        ClearDrawScreen();

        // �񓯊����[�h���I�������`�悷��
        if (!loading) {
            DrawExtendGraph(0, 0, 1024, 1024, handles[index], FALSE);
        }

        // loading ���ł� counter ���i�ނ̂Ŕ񓯊����[�h���m�F�ł���
        DrawFormatString(0, 0, str_color, _T("Frame[%d]"), counter);
        DrawFormatString(0, 32, str_color, _T("Loaded[%d] : Index[%d]"), (0 == loading), index);
        ScreenFlip();
    }

    Coroutine::Clear();

    for (const auto& handle : handles) {
        if (handle != -1) {
            DeleteGraph(handle);
        }
    }

    DxLib_End();

    return 0;
}
