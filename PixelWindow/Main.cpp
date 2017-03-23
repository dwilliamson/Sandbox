
#include <Windows.h>
#include <math.h>
#include <vector>


static const char* g_WindowClassName = "PixelWindow";


class PixelWindow
{
public:
	PixelWindow(const char* title, int width, int height);
	~PixelWindow();

	bool Poll();
	void Draw();
	void Close();

	unsigned char* Pixels()
	{
		return m_Pixels;
	}

	bool KeyHeld(int code)
	{
		return m_KeysHeld[code];
	}

private:
	static long WINAPI MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	int m_Width;
	int m_Height;

	HINSTANCE m_Instance;
	HWND m_Window;

	unsigned char* m_Pixels;
	HBITMAP m_Bitmap;

	bool m_KeysHeld[256];
};



namespace
{
	void* GetBitmapHandle(int width, int height, HBITMAP& handle)
	{
		// Create the DIB information
		BITMAPINFOHEADER bih;
		ZeroMemory(&bih, sizeof(BITMAPINFOHEADER));
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = width;
		bih.biHeight = -(int)height;
		bih.biPlanes = 1;
		bih.biBitCount = (WORD)24;
		bih.biCompression = BI_RGB;

		// Get the device context for the entire screen
		HDC screen_dc = GetWindowDC(NULL);
		if (screen_dc == NULL)
			return NULL;

		// Create a Device Independant Bitmap and retrieve the bitmap handle and address
		void* buffer = NULL;
		handle = CreateDIBSection(screen_dc, (LPBITMAPINFO)&bih, DIB_RGB_COLORS, (LPVOID *)&buffer, NULL, 0);

		// Release the previous DC
		ReleaseDC(NULL, screen_dc);

		return buffer;
	}


	void DrawBitmap(HWND hWnd, HBITMAP handle, int width, int height)
	{
		// Calculate the destination blit region
		RECT client;
		GetClientRect(hWnd, &client);
		client.right -= client.left;
		client.bottom -= client.top;

		// Get a compatible Bitmap DC (for bit-conversions)
		HDC hdc = GetDC(hWnd);
		HDC bitmap_dc = CreateCompatibleDC(hdc);

		// Select the desired bitmap object into the device context, saving the old object
		HBITMAP oldbmp = (HBITMAP)SelectObject(bitmap_dc, handle);

		// Blast the contents to the window
		StretchBlt(hdc, 0, 0, client.right, client.bottom, bitmap_dc, 0, 0, width, height, SRCCOPY);

		// Now revert to the old object
		SelectObject(bitmap_dc, oldbmp);

		// Delete the previously created device context
		DeleteDC(bitmap_dc);

		// Release the window DC for future use
		ReleaseDC(hWnd, hdc);
	}


	bool PollMessageQueue()
	{
		MSG		msg;

		// Loop until all messages have been processed
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				return false;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return true;
	}
}


long WINAPI PixelWindow::MessageHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Locate owner
	PixelWindow* window = NULL;
	if (hWnd != NULL)
		window = (PixelWindow*)GetWindowLong(hWnd, GWL_USERDATA);

	switch (message)
	{
	case WM_CREATE:
		break;

	case WM_KEYDOWN:
		if (window != NULL)
			window->m_KeysHeld[wParam] = true;
		break;
	case WM_KEYUP:
		if (window != NULL)
			window->m_KeysHeld[wParam] = false;
		break;

		/*case WM_PAINT:
		{
		PixelWindow* window = (PixelWindow*)GetWindowLong(hWnd, GWL_USERDATA);
		if (window != NULL)
		window->Draw();
		break;
		}*/

	case WM_CLOSE:
		if (window != NULL)
			window->Close();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

PixelWindow::PixelWindow(const char* title, int width, int height)
	: m_Width(width)
	, m_Height(height)
	, m_Instance(NULL)
	, m_Window(NULL)
	, m_Pixels(NULL)
	, m_Bitmap(NULL)
{
	memset(m_KeysHeld, 0, sizeof(m_KeysHeld));

	// Get application's instance
	m_Instance = GetModuleHandle(NULL);
	if (m_Instance == NULL)
		return;

	// Setup the window class elements
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MessageHandler;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_Instance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = g_WindowClassName;
	wc.lpszMenuName = NULL;

	// Attempt to register the class with windows
	if (!RegisterClass(&wc))
	{
		m_Instance = NULL;
		return;
	}

	// Adjust size of window so that client rect is correctly sized
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_POPUP;
	RECT rect = { 0, 0, width, height };
	AdjustWindowRect(&rect, dwStyle, FALSE);

	// Create the window
	m_Window = CreateWindow(
		g_WindowClassName,
		title,
		dwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top,
		NULL, NULL,
		m_Instance,
		NULL);
	if (m_Window == NULL)
		return;

	// Attach this class to the window handle for message handler
	SetWindowLong(m_Window, GWL_USERDATA, (long)this);
	m_Pixels = (unsigned char*)GetBitmapHandle(width, height, m_Bitmap);
	if (m_Pixels == NULL)
		return;

	// Show everything
	UpdateWindow(m_Window);
	ShowWindow(m_Window, SW_SHOW);
}


PixelWindow::~PixelWindow()
{
	Close();
}


void PixelWindow::Draw()
{
	DrawBitmap(m_Window, m_Bitmap, m_Width, m_Height);
}


bool PixelWindow::Poll()
{
	if (!PollMessageQueue())
	{
		m_Window = NULL;
		return false;
	}

	return true;
}


void PixelWindow::Close()
{
	// Release the DIB
	if (m_Bitmap != NULL)
	{
		DeleteObject(m_Bitmap);
		m_Bitmap = NULL;
	}

	// Post WM_DESTROY
	if (m_Window != NULL)
	{
		DestroyWindow(m_Window);
		m_Window = NULL;
	}

	// Release the user class
	if (m_Instance != NULL)
	{
		UnregisterClass(g_WindowClassName, m_Instance);
		m_Instance = NULL;
	}
}


namespace
{
	void DrawMip(int size, int mip, const std::vector<unsigned char*>& mips, unsigned char* pixels)
	{
		int mip_size = size >> mip;
		unsigned char* mip_pixels = mips[mip];

		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				unsigned char* dst_pixel = pixels + (y * size + x) * 3;
				unsigned char* src_pixel = mip_pixels + ((y >> mip) * mip_size + (x >> mip)) * 3;

				dst_pixel[0] = src_pixel[0];
				dst_pixel[1] = src_pixel[1];
				dst_pixel[2] = src_pixel[2];
			}
		}
	}
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int size = 256;
	PixelWindow window("Hello", size, size);

	std::vector<unsigned char*> mips;
	mips.push_back(window.Pixels());

	int mip_size = size / 2;
	while (mip_size > 0)
	{
		mips.push_back(new unsigned char[mip_size * mip_size * 3]);
		mip_size /= 2;
	}

	while (window.Poll() && !window.KeyHeld(VK_ESCAPE))
	{
		unsigned char* pixels = window.Pixels();

		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				unsigned char* pixel = pixels + (y * size + x) * 3;

				float r = sin((float)x * 0.05f) * sin((float)(x + 0.1) * 0.03f) + cos((float)y * 0.02f) * cos((float)(y + 0.2) * 0.01f);
				float g = sin((float)x * 0.07f) * sin((float)(x + 0.4) * 0.02f) + cos((float)y * 0.05f) * cos((float)(y + 0.1) * 0.005f);

				r = r * 0.5f;
				r = 1 - (1 - r) * (1 - r);
				r = max(-1, r);

				g = g * 0.5f;

				pixel[0] = 0;
				pixel[1] = abs(g) * 255;
				pixel[1] = 0;
				pixel[2] = abs(r) * 255;
			}
		}

		int src_size = size;
		int mip_size = size / 2;
		for (size_t i = 1; i < mips.size(); i++)
		{
			unsigned char* src_pixels = mips[i - 1];
			unsigned char* mip_pixels = mips[i];

			for (int y = 0; y < mip_size; y++)
			{
				for (int x = 0; x < mip_size; x++)
				{
					unsigned char* mip_pixel = mip_pixels + (y * mip_size + x) * 3;

					unsigned char* src_pixel_00 = src_pixels + ((y << 1) * src_size + (x << 1)) * 3;
					unsigned char* src_pixel_10 = src_pixels + ((y << 1) * src_size + (x << 1) + 1) * 3;
					unsigned char* src_pixel_01 = src_pixels + (((y << 1) + 1) * src_size + (x << 1)) * 3;
					unsigned char* src_pixel_11 = src_pixels + (((y << 1) + 1) * src_size + (x << 1) + 1) * 3;

					int ox = x & 1;
					int oy = y & 1;

					unsigned char* src_pixel = src_pixel_00;
					if (ox == 1 && oy == 0)
						src_pixel = src_pixel_10;
					if (ox == 0 && oy == 1)
						src_pixel = src_pixel_01;
					if (ox == 1 && oy == 1)
						src_pixel = src_pixel_11;

					mip_pixel[0] = src_pixel[0];
					mip_pixel[1] = src_pixel[1];
					mip_pixel[2] = src_pixel[2];
				}
			}

			src_size /= 2;
			mip_size /= 2;
		}

		if (window.KeyHeld('1'))
			DrawMip(size, 1, mips, pixels);
		if (window.KeyHeld('2'))
			DrawMip(size, 2, mips, pixels);
		if (window.KeyHeld('3'))
			DrawMip(size, 3, mips, pixels);
		if (window.KeyHeld('4'))
			DrawMip(size, 4, mips, pixels);
		if (window.KeyHeld('5'))
			DrawMip(size, 5, mips, pixels);
		if (window.KeyHeld('6'))
			DrawMip(size, 6, mips, pixels);
		if (window.KeyHeld('7'))
			DrawMip(size, 7, mips, pixels);

		window.Draw();
	}

	return 1;
}
