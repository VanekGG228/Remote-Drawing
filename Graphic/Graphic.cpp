// Graphic.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Graphic.h"

#define MAX_LOADSTRING 100
#define DRAW 100

int X1 = 0;
int Y1 = 0;
int X2 = 0;
int Y2 = 0;
HINSTANCE hInst;            
SOCKET ConnectSocket;
WCHAR szTitle[MAX_LOADSTRING];                 
WCHAR szWindowClass[MAX_LOADSTRING];  
HWND hWnd;


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HDC hdc;
BOOL isDrawing = FALSE;
int Ystart;
int Xstart;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int Reciver(std::vector<char> Point, int* i) {
    int PosX = 0;
    while ((*i)<Point.size() && Point[*i] != ' ') {
        PosX = PosX * 10 + (Point[*i] - '0');
        (*i)++;
    }
    (*i)++;
    return PosX;
}


void DrawLine(HDC hdc, int x1, int y1, int x2, int y2) {
    MoveToEx(hdc, x1, y1, NULL); 
    LineTo(hdc, x2, y2);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRAPHIC, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHIC));

    WSADATA wsaData;
    int iResult;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Ошибка при инициализации Winsock: %d\n", iResult);
        return 1;
    }

    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Ошибка при создании сокета: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(27015); 
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); 

    iResult = connect(ConnectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("Ошибка при подключении к серверу: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

   
    MSG msg;
    int i = 0;
    hdc = GetDC(hWnd);
   

    while (1)
    {
        if (PeekMessageA(&msg,NULL, 0,0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else{

            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(ConnectSocket, &readSet);

            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;

            int result = select(0, &readSet, nullptr, nullptr, &timeout);
            if (result == SOCKET_ERROR) {
                printf("Ошибка при вызове select(): %d\n", WSAGetLastError());
                break;
            }
            else if (result > 0) {
                std::vector <char> Point(20);
                int recvResult = recv(ConnectSocket, Point.data(), Point.size(), 0);

                X1 = Y1 = X2 = Y2 = 0;

                i = 0;
                X1 = Reciver(Point, &i);
                Y1 = Reciver(Point, &i);
                X2 = Reciver(Point, &i);
                Y2 = Reciver(Point, &i);
                RECT clientRect;
                GetClientRect(hWnd, &clientRect);
                int width = clientRect.right - clientRect.left;
                int height = clientRect.bottom - clientRect.top;

                if (X1 > 0 && X2 > 0 && Y1 > 0 && Y2 > 0 && X1 < width && X2 < width && Y1 < height && Y2 < height) {
                    if (recvResult > 0) {
                        hdc = GetDC(hWnd);
                        MoveToEx(hdc, X2, Y2, NULL);
                        LineTo(hdc, X1, Y1);
                        ReleaseDC(hWnd, hdc);
                        InvalidateRect(hWnd, NULL, FALSE);
                        UpdateWindow(hWnd);
                    }
                }
            }


        }

    }

    closesocket(ConnectSocket);
    WSACleanup();

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHIC));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GRAPHIC);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX ,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   SetWindowPos(hWnd, HWND_TOP, 0, 0, 600, 550, SWP_SHOWWINDOW);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


void Process(std::vector<char> *Point,std::string X) {
    for (int i = 0; i < X.size(); i++) {
        Point->push_back(X[i]);
    }
    Point->push_back(' ');
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_LBUTTONDOWN:
        isDrawing = TRUE;
        Xstart = LOWORD(lParam);
        Ystart = HIWORD(lParam);
       
        break;
    case WM_MOUSEMOVE: {
        if (isDrawing) {

            RECT clientRect;
            GetClientRect(hWnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;
            int xPos = LOWORD(lParam);
            int yPos = HIWORD(lParam);         
            if (xPos > 0 && Xstart > 0 && yPos > 0 && Ystart > 0 && xPos < width && Xstart < width && yPos < height && Ystart < height) {
                HDC hdc = GetDC(hWnd);
                DrawLine(hdc, Xstart, Ystart, xPos, yPos);

                std::vector <char> Point;
                std::string X1 = std::to_string(xPos);
                std::string Y1 = std::to_string(yPos);
                std::string X2 = std::to_string(Xstart);
                std::string Y2 = std::to_string(Ystart);
                Xstart = xPos;
                Ystart = yPos;
                Process(&Point, X1);
                Process(&Point, Y1);
                Process(&Point, X2);
                Process(&Point, Y2);

                while (Point.size() < 20) Point.push_back(' ');

                int iResult1 = send(ConnectSocket, Point.data(), Point.size(), 0);
                if (iResult1 == SOCKET_ERROR) {
                    printf("Ошибка при отправке данных: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }

                ReleaseDC(hWnd, hdc);
            }
        }
        break;
    }
    case WM_LBUTTONUP: 
        {
           
            isDrawing = FALSE;
            break;
        }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        ValidateRect(hWnd,NULL);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
