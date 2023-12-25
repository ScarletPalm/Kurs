#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <richedit.h>
#include <commdlg.h>
#include <winuser.h>

const int ID_BUTTON_COMPILE = 1;
const int ID_EDIT_INPUT = 2;
const int ID_EDIT_OUTPUT = 3;
const int ID_BUTTON_SAVE = 4;
const int ID_BUTTON_OPEN = 5;
const int ID_BUTTON_HELP = 6;

int nVScrollPos = 0;
int nHScrollPos = 0;


// Функция для компиляции и запуска кода на C++
void CompileAndRun(HWND hwndInput, HWND hwndOutput) {
    // Получаем длину текста входного поля
    int textLength = GetWindowTextLengthA(hwndInput);
    std::string code;

    // Выделяем память и извлекаем текст
    if (textLength > 0) {
        code.resize(textLength + 1);
        GetWindowTextA(hwndInput, &code[0], textLength + 1);
    }

    // Открываем временный файл для записи кода на C++
    std::ofstream codeFile("temp.cpp");

    if (!codeFile.is_open()) {
        MessageBoxA(NULL, "Ошибка открытия файла для записи.", "Ошибка", MB_ICONERROR);
        return;
    }

    // Записываем код в файл
    codeFile << code;

    // Закрываем файл
    codeFile.close();

    // Компилируем код с использованием системной команды
    std::string compileCommand = "g++ -o output temp.cpp 2>&1";
    FILE* pipe = _popen(compileCommand.c_str(), "r");

    if (!pipe) {
        MessageBoxA(NULL, "Ошибка открытия канала.", "Ошибка", MB_ICONERROR);
        return;
    }

    char buffer[128];
    std::string result = "";

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }

    int compileResult = _pclose(pipe);

    // Отображаем вывод в поле вывода
    SetWindowTextA(hwndOutput, result.c_str());

    if (compileResult != 0) {
        MessageBoxA(NULL, "Ошибка компиляции.", "Ошибка", MB_ICONERROR);
        return;
    }

    // Запускаем скомпилированную программу и захватываем ее вывод
    pipe = _popen("output", "r");
    if (!pipe) {
        MessageBoxA(NULL, "Ошибка открытия канала.", "Ошибка", MB_ICONERROR);
        return;
    }

    result = "";

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr)
            result += buffer;
    }

    int runResult = _pclose(pipe);

    // Отображаем вывод в поле вывода
    SetWindowTextA(hwndOutput, result.c_str());

    if (runResult != 0) {
        MessageBoxA(NULL, "Ошибка выполнения программы.", "Ошибка", MB_ICONERROR);
    }

    // Удаляем временные файлы
    std::remove("temp.cpp");
    std::remove("output");
}

void OnSize(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    int cxClient, cyClient;
    cxClient = LOWORD(lParam);
    cyClient = HIWORD(lParam);

    // Используем двойную буферизацию
    HDWP hdwp = BeginDeferWindowPos(4);

    // Откладываем изменение размеров элементов управления ввода и вывода
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_EDIT_INPUT), NULL, 10, 10, cxClient - 20, cyClient - 200, SWP_NOREPOSITION);
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_EDIT_OUTPUT), NULL, 10, cyClient - 190, cxClient - 20, 100, SWP_NOREPOSITION);
    // Изменяем размер кнопок
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_BUTTON_COMPILE), NULL, 10, cyClient - 70, 220, 30, TRUE);
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_BUTTON_SAVE), NULL, 240, cyClient - 70, 150, 30, TRUE);
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_BUTTON_OPEN), NULL, 400, cyClient - 70, 150, 30, TRUE);

    // Перемещаем кнопку справки на новую строку
    DeferWindowPos(hdwp, GetDlgItem(hwnd, ID_BUTTON_HELP), NULL, cxClient - 90, cyClient - 36, 80, 30, TRUE);

    // Завершаем двойную буферизацию
    EndDeferWindowPos(hdwp);
}



// Показать диалоговое окно справки
void HelpDialog(HWND hwnd) {
    // Показываем информационное окно со справкой
    MessageBox(hwnd, L"Текстовый редактор для программирования на языке C++\nСтудента МИРЭА Института ИИТ Группы ИКБО-10-22\nСвищёв Е.А", L"Справка", MB_ICONINFORMATION);
}

// Функция для сохранения кода в файл
void SaveCode(HWND hwndInput) {
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = L""; // Используем широкие символы

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"C++ Files (*.cpp)\0*.cpp\0All Files (*.*)\0*.*\0"; // Используем широкий строковый литерал
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) / sizeof(szFileName[0]);
    ofn.lpstrTitle = L"Сохранить код C++"; // Используем широкий строковый литерал
    ofn.Flags = OFN_OVERWRITEPROMPT;

    if (GetSaveFileName(&ofn)) { // Используем функцию со широкими символами
        std::wstring filePath = ofn.lpstrFile; // Используем широкую строку
        std::ofstream file(filePath);

        if (file.is_open()) {
            int textLength = GetWindowTextLengthA(hwndInput);
            std::string code;

            if (textLength > 0) {
                code.resize(textLength + 1);
                GetWindowTextA(hwndInput, &code[0], textLength + 1);
                file << code;
            }

            file.close();
            MessageBox(NULL, L"Код успешно сохранён.", L"Успех", MB_ICONINFORMATION); // Используем широкий строковый литерал
        }
        else {
            MessageBox(NULL, L"Ошибка сохранения кода в файл.", L"Ошибка", MB_ICONERROR); // Используем широкий строковый литерал
        }
    }
    else {
        // Пользователь отменил операцию сохранения
    }
}

// Загрузить код из файла
void LoadCode(HWND hwndInput) {
    OPENFILENAME ofn;
    wchar_t szFileName[MAX_PATH] = L"";

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = L"C++ Files (*.cpp)\0*.cpp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName) / sizeof(szFileName[0]);
    ofn.lpstrTitle = L"Открыть код C++";
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileName(&ofn)) {
        std::wstring filePath = ofn.lpstrFile;
        std::ifstream file(filePath);

        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            SetWindowTextA(hwndInput, buffer.str().c_str());
        }
        else {
            MessageBox(NULL, L"Ошибка загрузки кода из файла.", L"Ошибка", MB_ICONERROR);
        }
    }
    else {
        // Пользователь отменил операцию открытия
    }
}

// Добавляем следующие вспомогательные функции
int GetScrollLimit(HWND hwnd, int nBar) {
    SCROLLINFO si = { sizeof(SCROLLINFO), SIF_RANGE };
    GetScrollInfo(hwnd, nBar, &si);
    return si.nMax - max(0, si.nPage - 1);
}

// Процедура окна
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        // Создаем поле редактирования для ввода
        LoadLibrary(TEXT("msftedit.dll"));
        // Создаем поле редактирования для ввода с вертикальным и горизонтальным полосами прокрутки
        HWND hwndInput = CreateWindowExW(0, MSFTEDIT_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL,
            10, 10, 450, 200, hwnd, (HMENU)ID_EDIT_INPUT, GetModuleHandle(NULL), NULL);

        // Создаем поле редактирования для вывода с вертикальными и горизонтальными полосами прокрутки
        HWND hwndOutput = CreateWindowExW(0, MSFTEDIT_CLASS, L"", WS_CHILD | WS_VISIBLE | WS_BORDER |
            ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY | WS_VSCROLL | WS_HSCROLL,
            10, 220, 450, 100, hwnd, (HMENU)ID_EDIT_OUTPUT, GetModuleHandle(NULL), NULL);

        // Создаем кнопку компиляции
        CreateWindowW(L"BUTTON", L"Компилировать и Запустить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 330, 220, 30, hwnd, (HMENU)ID_BUTTON_COMPILE, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Сохранить Код", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            240, 330, 150, 30, hwnd, (HMENU)ID_BUTTON_SAVE, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Открыть Код", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            400, 330, 150, 30, hwnd, (HMENU)ID_BUTTON_OPEN, GetModuleHandle(NULL), NULL);

        CreateWindowW(L"BUTTON", L"Справка", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            480, 10, 80, 30, hwnd, (HMENU)ID_BUTTON_HELP, GetModuleHandle(NULL), NULL);
        OnSize(hwnd, wParam, lParam);
        break;
    }
    case WM_SIZE:
        OnSize(hwnd, wParam, lParam);
        return 0;


    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON_COMPILE && HIWORD(wParam) == BN_CLICKED) {
            // Нажата кнопка, компилируем и запускаем код
            HWND hwndInput = GetDlgItem(hwnd, ID_EDIT_INPUT);
            HWND hwndOutput = GetDlgItem(hwnd, ID_EDIT_OUTPUT);
            CompileAndRun(hwndInput, hwndOutput);
        }
        else if (LOWORD(wParam) == ID_BUTTON_SAVE && HIWORD(wParam) == BN_CLICKED) {
            // Нажата кнопка, сохраняем код
            HWND hwndInput = GetDlgItem(hwnd, ID_EDIT_INPUT);
            SaveCode(hwndInput);
        }
        else if (LOWORD(wParam) == ID_BUTTON_OPEN && HIWORD(wParam) == BN_CLICKED) {
            // Нажата кнопка, открываем код
            HWND hwndInput = GetDlgItem(hwnd, ID_EDIT_INPUT);
            LoadCode(hwndInput);
        }
        else if (LOWORD(wParam) == ID_BUTTON_HELP && HIWORD(wParam) == BN_CLICKED) {
            // Нажата кнопка справки, показываем диалоговое окно справки
            HelpDialog(hwnd);
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_VSCROLL: {
        HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_INPUT);
        int nScrollCode = LOWORD(wParam);
        int nPos = nVScrollPos;

        switch (nScrollCode) {
        case SB_TOP:
            nPos = 0;
            break;

        case SB_BOTTOM:
            nPos = GetScrollLimit(hwndEdit, SB_VERT);
            break;

        case SB_LINEUP:
            nPos -= 3;  // Прокрутка вверх на три строки
            break;

        case SB_LINEDOWN:
            nPos += 3;  // Прокрутка вниз на три строки
            break;

        case SB_PAGEUP:
            nPos -= 10;  // Прокрутка вверх на десять строк
            break;

        case SB_PAGEDOWN:
            nPos += 10;  // Прокрутка вниз на десять строк
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nPos = HIWORD(wParam);
            break;
        }

        nPos = max(0, min(nPos, GetScrollLimit(hwndEdit, SB_VERT)));
        SetScrollPos(hwndEdit, SB_VERT, nPos, TRUE);
        nVScrollPos = nPos;

        // Прокрутка поля редактирования
        SendMessage(hwndEdit, EM_LINESCROLL, 0, nPos - nVScrollPos);
        break;
    }

    case WM_HSCROLL: {
        HWND hwndEdit = GetDlgItem(hwnd, ID_EDIT_INPUT);
        int nScrollCode = LOWORD(wParam);
        int nPos = nHScrollPos;

        switch (nScrollCode) {
        case SB_LEFT:
            nPos = 0;
            break;

        case SB_RIGHT:
            nPos = GetScrollLimit(hwndEdit, SB_HORZ);
            break;

        case SB_LINELEFT:
            nPos -= 5;  // Прокрутка влево на пять символов
            break;

        case SB_LINERIGHT:
            nPos += 5;  // Прокрутка вправо на пять символов
            break;

        case SB_PAGELEFT:
            nPos -= 20;  // Прокрутка влево на двадцать символов
            break;

        case SB_PAGERIGHT:
            nPos += 20;  // Прокрутка вправо на двадцать символов
            break;

        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nPos = HIWORD(wParam);
            break;
        }

        nPos = max(0, min(nPos, GetScrollLimit(hwndEdit, SB_HORZ)));
        SetScrollPos(hwndEdit, SB_HORZ, nPos, TRUE);
        nHScrollPos = nPos;

        // Прокрутка поля редактирования
        SendMessage(hwndEdit, EM_LINESCROLL, nPos - nHScrollPos, 0);
        break;
    }


    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Зарегистрировать класс окна
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"MyClass";

    if (!RegisterClassW(&wc))
        return 1;

    // Создать окно
    HWND hwnd = CreateWindowExW(0, L"MyClass", L"C++ Code Editor", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
        NULL, NULL, hInstance, NULL);

    if (!hwnd)
        return 2;

    // Показать и запустить окно
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
