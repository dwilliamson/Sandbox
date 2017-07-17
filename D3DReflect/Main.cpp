
#include <d3d11.h>
#include <d3d11shader.h>
#include <d3dcompiler.h>
#include <stdio.h>
#include <memory>
#include <vector>
#include <atlbase.h>


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
// TODO: fxc from 8.1?


std::vector<char> LoadShaderData(const char* filename)
{
	FILE* fp;
	if (fopen_s(&fp, filename, "rb") != 0)
		return {};

	fseek(fp, 0, SEEK_END);
	int shader_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::vector<char> shader_data(shader_size);
	if (fread(shader_data.data(), 1, shader_size, fp) != shader_size)
	{
		fclose(fp);
		return {};
	}

	fclose(fp);
	return shader_data;
}


CComPtr<ID3D11Device> CreateD3DDevice()
{
	// Create the device
	CComPtr<ID3D11Device> device;
	D3D_FEATURE_LEVEL flr = D3D_FEATURE_LEVEL_11_0;
	HRESULT hr;
	hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&flr,
		1,
		D3D11_SDK_VERSION,
		&device,
		nullptr,
		nullptr);
	if (hr != S_OK)
		return nullptr;

	return device;
}


const char* TAB = "";
#define FIELD_STR(a, b) printf("%s" #b ": %s\n", TAB, a.b);
#define FIELD_INT(a, b) printf("%s" #b ": %d\n", TAB, a.b);


int main()
{
	CComPtr<ID3D11Device> device = CreateD3DDevice();
	if (device == nullptr)
		return 1;

	std::vector<char> shader_data = LoadShaderData("D:\\dev\\celtoys\\Star\\pub\\Server\\Shaders\\Pixel\\Scattering\\Inscatter1Gen.psh.sobj");
	if (shader_data.size() == 0)
		return 1;

	ID3D11ShaderReflection* reflection;
	D3DReflect(shader_data.data(), shader_data.size(), IID_ID3D11ShaderReflection, (void**)&reflection);

	D3D11_SHADER_DESC desc;
	reflection->GetDesc(&desc);

	FIELD_STR(desc, Creator);
	FIELD_INT(desc, ConstantBuffers);
	FIELD_INT(desc, BoundResources);
	FIELD_INT(desc, InputParameters);
	FIELD_INT(desc, OutputParameters);

	for (unsigned int i = 0; i < desc.ConstantBuffers; i++)
	{
		ID3D11ShaderReflectionConstantBuffer* cb = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC desc;
		cb->GetDesc(&desc);

		TAB = "  ";
		FIELD_STR(desc, Name);
		TAB = "    ";
		FIELD_INT(desc, Variables);
		FIELD_INT(desc, Size);

		for (unsigned int j = 0; j < desc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(j);

			D3D11_SHADER_VARIABLE_DESC desc2;
			var->GetDesc(&desc2);

			TAB = "      ";
			FIELD_STR(desc2, Name);
			TAB = "        ";
			FIELD_INT(desc2, StartOffset);
			FIELD_INT(desc2, Size);

			ID3D11ShaderReflectionType* type = var->GetType();
			D3D11_SHADER_TYPE_DESC desc3;
			type->GetDesc(&desc3);
			FIELD_STR(desc3, Name);
			FIELD_INT(desc3, Members);
		}
	}

	for (unsigned int i = 0; i < desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC desc;
		reflection->GetResourceBindingDesc(i, &desc);

		TAB = "   ";
		FIELD_STR(desc, Name);
		TAB = "      ";
		FIELD_INT(desc, BindPoint);
		FIELD_INT(desc, BindCount);
	}

	reflection->Release();

	return 0;
}