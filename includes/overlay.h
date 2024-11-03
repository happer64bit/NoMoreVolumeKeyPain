#ifndef OVERLAY_H
#define OVERLAY_H

#include <windows.h>

extern HWND hOverlayWnd; // Declaration only, no definition

void ShowOverlay(int volume);
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // OVERLAY_H
