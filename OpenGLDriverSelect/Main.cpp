
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <gl/gl.h>


#pragma comment(lib, "opengl32.lib")


IMAGE_THUNK_DATA* GetOriginalFirstThunk(HMODULE hModule, IMAGE_IMPORT_DESCRIPTOR* pImportDesc)
{
	return (IMAGE_THUNK_DATA*)((BYTE*)hModule + pImportDesc->OriginalFirstThunk);
}

IMAGE_THUNK_DATA* GetFirstThunk(HMODULE hModule, IMAGE_IMPORT_DESCRIPTOR* pImportDesc)
{
	return (IMAGE_THUNK_DATA*)((BYTE*)hModule + pImportDesc->FirstThunk);
}


IMAGE_IMPORT_DESCRIPTOR* GetImportDescriptor(HMODULE hModule, char* pszDllName)
{
	// Get the DOS Header from which we can get the optional header.
	IMAGE_DOS_HEADER* pDOSHeader = (IMAGE_DOS_HEADER*)hModule;
	IMAGE_OPTIONAL_HEADER* pOptionHeader = (IMAGE_OPTIONAL_HEADER*)((BYTE*)hModule + pDOSHeader->e_lfanew + 24);

	// To get the Import descriptor datastructure.
	IMAGE_IMPORT_DESCRIPTOR* pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*)((BYTE*)hModule + pOptionHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	// Iterate through the IMPORT DESCRIPTOR till we find the address of
	// Kernel32.dll in our case, since LoadLibrary is from Kernel32.dll
	char* pszImpAddr = 0;
	while (pImportDesc->FirstThunk)
	{
		// Get the load address of the DLL.
		pszImpAddr = (char*)((BYTE*)hModule + pImportDesc->Name);
		if (_stricmp(pszDllName, pszImpAddr))
		{
			pImportDesc++;
			continue;
		}
		else
		{
			return pImportDesc;
		}
	}
	return NULL;
}


//typedef DWORD Address;
typedef ULONGLONG Address;

// This function gets the address of the LoadLibraryA function.
Address* GetCurrentFunctAddr(HMODULE hMod,
	IMAGE_THUNK_DATA* pOriginalFirstThunk,
	IMAGE_THUNK_DATA* pFirstThunk,
	char* pszFunctionName)
{
	// From the IMAGE thunk datastructure we
	// get the address of the imported function.
	char* szTest;
	while (pOriginalFirstThunk->u1.Function)
	{
		// Get the import load address of the function,
		// in our case LoadLibraryA.
		szTest = (char*)((BYTE*)hMod + pOriginalFirstThunk->u1.AddressOfData + 2);
		if (_stricmp(pszFunctionName, szTest) == 0)
		{
			return (Address*)&pFirstThunk->u1.Function;
		}
		pOriginalFirstThunk++;
		pFirstThunk++;
	}
	return NULL;
}

BOOL ChangeAddress(Address* dwOldAddress, Address dwNewAddress)
{
	DWORD dwOld;
	if (!(VirtualProtect(dwOldAddress, 8, PAGE_READWRITE, &dwOld)))
	{
		return FALSE;
	}
	*dwOldAddress = dwNewAddress;
	if (!(VirtualProtect(dwOldAddress, 8, PAGE_EXECUTE, &dwOld)))
	{
		return FALSE;
	}
	else
	{
		//OutputDebugString( "Change Address Final.." );
		//printf("Change Address Final..\n");
		return TRUE;
	}
}

BOOL ReRouteAPI(HMODULE hMod, char* pszDllName, char* pszFunctionName, Address dwNewAddress)
{
	//printf( "ReRouteAPI is entered....." );
	IMAGE_IMPORT_DESCRIPTOR* IID = GetImportDescriptor(hMod, pszDllName);
	if (IID == NULL) return FALSE;
	IMAGE_THUNK_DATA* OriginalFirstThunk = GetOriginalFirstThunk(hMod, IID);
	IMAGE_THUNK_DATA* FirstThunk = GetFirstThunk(hMod, IID);
	//printf( "Change Address start.." );
	Address* dwOldFunctionAddress = GetCurrentFunctAddr(hMod, OriginalFirstThunk,
		FirstThunk, pszFunctionName);
	if (dwOldFunctionAddress == NULL) return FALSE;
	return ChangeAddress(dwOldFunctionAddress, dwNewAddress);
}
/* End code taken */

/* Our replacement EnumDisplayDevicesA function */
DWORD realDevNum;

BOOL __stdcall MyEnumDisplayDevicesA(LPCSTR lpDevice, DWORD iDevNum, PDISPLAY_DEVICEA lpDisplayDevice, DWORD dwFlags)
{
	//we only support 1 device
	if (iDevNum != 0)
	{
		return FALSE;
	}

	//call the real EnumDisplayDevicesA with our pre-selected device number
	if (!EnumDisplayDevicesA(lpDevice, realDevNum, lpDisplayDevice, dwFlags))
	{
		return FALSE;
	}

	//patch the state flags so OpenGL32.dll picks this one
	lpDisplayDevice->StateFlags = 4;
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
			PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
			32,                        //Colordepth of the framebuffer.
			0, 0, 0, 0, 0, 0,
			0,
			0,
			0,
			0, 0, 0, 0,
			24,                        //Number of bits for the depthbuffer
			8,                        //Number of bits for the stencilbuffer
			0,                        //Number of Aux buffers in the framebuffer.
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		HDC ourWindowHandleToDeviceContext = GetDC(hWnd);

		int  letWindowsChooseThisPixelFormat;
		letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd);
		SetPixelFormat(ourWindowHandleToDeviceContext, letWindowsChooseThisPixelFormat, &pfd);

		HGLRC ourOpenGLRenderingContext = wglCreateContext(ourWindowHandleToDeviceContext);
		wglMakeCurrent(ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);

		printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
		printf("GL_VERSION: %s\n", glGetString(GL_VERSION));

		wglDeleteContext(ourOpenGLRenderingContext);
		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;

}

void PrintDevices()
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(DISPLAY_DEVICE);
	int i = 0;
	while (EnumDisplayDevices(NULL, i, &dd, 0))
	{
		if ((dd.StateFlags & DISPLAY_DEVICE_ACTIVE) != 0)
			printf("%d - %S\n", i, dd.DeviceString);
		i++;
	}
}


int main(int argc, char* argv[])
{
	//realDevNum = 3;

	PrintDevices();

	//get the handle to the opengl32.dll library
	HMODULE ogl = LoadLibrary(L"Opengl32.dll");
	//re-route EnumDisplayDevicesA to our function
	ReRouteAPI(ogl, "user32.dll", "EnumDisplayDevicesA", (uint64_t)MyEnumDisplayDevicesA);

	//code to test out our device replacement
	MSG msg = { 0 };
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = L"oglversionchecksample";
	wc.style = CS_OWNDC;
	if (!RegisterClass(&wc))
		return 1;
	CreateWindowW(wc.lpszClassName, L"openglversioncheck", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, GetModuleHandle(NULL), 0);

	while (GetMessage(&msg, NULL, 0, 0) > 0)
		DispatchMessage(&msg);

	return 0;
}