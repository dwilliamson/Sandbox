
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

	RECT rc;

	GetWindowRect(m_Window, &rc);

	int x = (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2;
	int y = (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2;
	SetWindowPos(m_Window, HWND_TOP, x, y, 0, 0, SWP_NOSIZE);

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


/*namespace
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
}*/


#include <assert.h>

// For M_PI
// TODO: Use Boost instead.
// TODO: OK, only kidding.
#define _USE_MATH_DEFINES

#include <math.h>


struct Sample
{
	Sample()
		: x(0)
		, y(0)
	{
	}

	float x, y;
};


float RandomFloat(float float_max)
{
	// OH MY GOD I'd forgotten on how awful the numbers generated by this are
	return (rand() / (float)RAND_MAX) * float_max;
}


int RandomInt(int int_max)
{
	return int((rand() / (float)RAND_MAX) * int_max);
}


struct PoissonGenerator
{
	PoissonGenerator(float range_x, float range_y, float radius, int nb_candidates)
		: range_x(range_x)
		, range_y(range_y)
		, cell_size(radius * (1.0f / sqrtf(2.0f)))
		, nb_cells_x(int(range_x / cell_size) + 1)
		, nb_cells_y(int(range_y / cell_size) + 1)
		, nb_cells(nb_cells_x * nb_cells_y)
		, nb_candidates(nb_candidates)
		, radius(radius)
		, cells(nb_cells, -1)
	{
	}

	void Start()
	{
		// Clear output to allow restarts
		active_samples.clear();
		output_samples.clear();
		std::fill(cells.begin(), cells.end(), -1);

		// Seed with an unconstrained sample
		Sample sample;
		sample.x = RandomFloat(range_x);
		sample.y = RandomFloat(range_y);
		AddSample(sample);
	}

	void AddSample(const Sample& sample)
	{
		active_samples.push_back(sample);
		output_samples.push_back(sample);

		int x = int(sample.x / cell_size);
		int y = int(sample.y / cell_size);
		int i = y * nb_cells_x + x;
		assert(cells[i] == -1);
		cells[i] = output_samples.size() - 1;
	}

	bool SampleInRange(float x, float y)
	{
		int cx = int(x / cell_size);
		int cy = int(y / cell_size);

		float radius_2 = radius * radius;

		// Only need to search immediate neighbourhood due to cell size calculated from radius
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				int ix = cx + i;
				int iy = cy + j;

				// Wrap mode addressing to ensure tiling
				if (ix < 0)
					ix += nb_cells_x;
				if (ix >= nb_cells_x)
					ix -= nb_cells_x;
				if (iy < 0)
					iy += nb_cells_y;
				if (iy >= nb_cells_y)
					iy -= nb_cells_y;

				int cell_idx = iy * nb_cells_x + ix;

				int sample_idx = cells[cell_idx];
				if (sample_idx == -1)
					continue;

				// Looking for samples inside the provided radius
				Sample& sample = output_samples[sample_idx];
				float dx = x - sample.x;
				float dy = y - sample.y;
				float d = dx * dx + dy * dy;
				if (d < radius_2)
					return true;
			}
		}

		return false;
	}

	bool AddNextSample()
	{
		if (active_samples.size() == 0)
			return false;

		// Choose a random sample from the active list
		int sample_idx = RandomInt(active_samples.size() - 1);
		Sample& sample = active_samples[sample_idx];

		// Search random candidate samples
		bool found = false;
		for (int i = 0; i < nb_candidates; i++)
		{
			// Generate point in the annulus
			float r = radius + RandomFloat((float)radius);
			float t = RandomFloat(float(2 * M_PI));
			float x = sample.x + cos(t) * r;
			float y = sample.y + sin(t) * r;

			// Reject out of range
			if (x < 0 || y < 0 || x >= range_x || y >= range_y)
				continue;

			// Are there any other samples too close?
			if (!SampleInRange(x, y))
			{
				// No, add this new one
				Sample k_sample;
				k_sample.x = x;
				k_sample.y = y;
				AddSample(k_sample);
				found = true;
				break;
			}
		}

		// Remove sample from the active list if none of its candidate tests succeed
		if (found == false)
		{
			active_samples[sample_idx] = active_samples.back();
			active_samples.pop_back();
		}

		return true;
	}

	float range_x;
	float range_y;

	float cell_size;

	int nb_cells_x;
	int nb_cells_y;

	int nb_cells;

	float radius;

	int nb_candidates;

	std::vector<Sample> active_samples;

	std::vector<int> cells;

	std::vector<Sample> output_samples;
};


#include <time.h>
#include <io.h>
#include <fcntl.h>


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int width = 1200;
	int height = 800;
	PixelWindow window("Bridson Poisson Distribution", width, height);

	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	float radius = 10;
	int k = 50;

	srand((unsigned int)time(0));

	//PoissonGenerator generator(1, 0.5f, 0.005f, k);

	// Assuming cell count ratio of 1/2
	// nb_cells = N
	// nb_cells_x = nb_cells_y * 2
	// N = 2 * nb_cells_y * nb_cells_y
	// nb_cells_y = sqrt(N / 2)
	float nb_cells = (256 * 6) / 0.336f;
	float nb_cells_y = sqrt(nb_cells / 2);

	// nb_cells_y = range_y / cell_size + 1
	// nb_cells_y - 1 = range_y / cell_size
	// cell_size = range_y / (nb_cells_y - 1)
	float cell_size = M_PI / (nb_cells_y - 1);
	radius = cell_size * sqrtf(2);
	PoissonGenerator generator(2 * M_PI, M_PI, radius, k);
	generator.Start();


	static const bool visualise = false;

	while (window.Poll() && !window.KeyHeld(VK_ESCAPE))
	{
		unsigned char* pixels = window.Pixels();

		bool added = generator.AddNextSample();

		if (visualise || !added)
		{
			float sx = width / (2 * M_PI);
			float sy = height / M_PI;

			for (size_t i = 0; i < generator.output_samples.size(); i++)
			{
				Sample& s = generator.output_samples[i];
				unsigned char* pixel = pixels + ((int)(s.y * sy) * width + (int)(s.x * sx)) * 3;
				pixel[0] = 255;
				pixel[1] = 255;
				pixel[2] = 255;
			}

			for (size_t i = 0; i < generator.active_samples.size(); i++)
			{
				Sample& s = generator.active_samples[i];
				unsigned char* pixel = pixels + ((int)(s.y * sy) * width + (int)(s.x * sx)) * 3;
				pixel[0] = generator.active_samples.size() & 1 ? 255 : 0;
				pixel[1] = 0;
				pixel[2] = 255;
			}

			window.Draw();
		}

		if (!added && window.KeyHeld(VK_SPACE))
		{
			printf("%d - %f\n", generator.output_samples.size(), generator.output_samples.size() / (float)generator.nb_cells);
			generator.Start();
			memset(pixels, 0, width * height * 3);
		}
	}

	return 1;
}

