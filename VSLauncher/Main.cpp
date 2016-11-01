
#include <windows.h>

// http://msdn.microsoft.com/en-us/library/yf86a8ts(v=vs.80).aspx
// http://msdn.microsoft.com/en-us/library/68shb4dw(v=vs.80).aspx
// http://msdn.microsoft.com/en-US/library/ms228691(v=vs.80).aspx

#pragma warning( disable : 4278 )
#pragma warning( disable : 4146 )

//The following #import imports EnvDTE based on its LIBID.
#import "libid:80cc9f66-e7d8-4ddd-85b6-d9e6cd0e93e2" version("8.0") lcid("0") raw_interfaces_only named_guids

//The following #import imports EnvDTE80 based on its LIBID.
#import "libid:1A31287A-4D7D-413e-8E32-3B374931BD89" version("8.0") lcid("0") raw_interfaces_only named_guids

//The following #import imports EnvDTE90 based on its LIBID.
#import "libid: 2ce2370e-d744-4936-a090-3fffe667b0e1" version("9.0") lcid("0") raw_interfaces_only named_guids

//The following #import imports EnvDTE100 based on its LIBID.
#import "libid: 26ad1324-4b7c-44bc-84f8-b86aed45729f" version("10.0") lcid("0") raw_interfaces_only named_guids

#pragma warning( default : 4146 )
#pragma warning( default : 4278 )

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	CoInitialize(0);

	CLSID clsid;
	CLSIDFromProgID(L"VisualStudio.DTE.10.0", &clsid);

	// Get any running instance of Visual Studio
	IUnknown* dte_unknown = 0;
	HRESULT hr = GetActiveObject(clsid, 0, &dte_unknown);
	if (dte_unknown)
	{
		EnvDTE80::DTE2Ptr dte = dte_unknown;

		// Get a list of commands from Visual Studio
		EnvDTE::CommandsPtr commands;
		hr = dte->get_Commands(&commands);

		// Get a pointer to the macro
		EnvDTE::CommandPtr command;
		variant_t key = L"Macros.StarVSMacros.Debug.AttachToGame";
		hr = commands->Item(key, 0, &command);

		// Get the macro GUID and ID
		bstr_t cmd_guid;
		hr = command->get_Guid(cmd_guid.GetAddress());
		long cmd_id;
		hr = command->get_ID(&cmd_id);

		// Execute the macro
		variant_t custom_in, custom_out;
		commands->Raise(cmd_guid, cmd_id, custom_in.GetAddress(), custom_out.GetAddress());
	}

	CoUninitialize();

	return 0;
}