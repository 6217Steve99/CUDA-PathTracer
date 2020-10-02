#pragma once
#pragma warning(disable: 4312)
#include <Windows.h>
#include "CasterLabWin.h"
#include "WindowException.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "DxgiInfoManager.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <random>



#include "ShaderStructs.h"

class Graphics
{
	friend class Bindable;
public:
	class Exception : public WindowException
	{
		using WindowException::WindowException;
	};
	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		HRESULT GetErrorCode() const noexcept;
		std::string GetErrorString() const noexcept;
		std::string GetErrorDescription() const noexcept;
		std::string GetErrorInfo() const noexcept;
	private:
		HRESULT hr;
		std::string info;
	};
	class InfoException : public Exception
	{
	public:
		InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
		const char* what() const noexcept override;
		const char* GetType() const noexcept override;
		std::string GetErrorInfo() const noexcept;
	private:
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override;
	private:
		std::string reason;
	};
public:
	Graphics(HWND hWnd, int width, int height);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
	void EndFrame();
	void BeginFrame(float red, float green, float blue) noexcept;
	void DrawIndexed(UINT count) noexcept(!IS_DEBUG);
	void SetProjection(DirectX::FXMMATRIX proj) noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	void SetCamera(DirectX::FXMMATRIX cam) noexcept;
	DirectX::XMMATRIX GetCamera() const noexcept;
	void EnableImgui() noexcept;
	void DisableImgui() noexcept;
	bool IsImguiEnabled() const noexcept;
private:
	DirectX::XMMATRIX projection;
	DirectX::XMMATRIX camera;
	bool imguiEnabled = true;
	int windowWidth;
	int windowHeight;
#ifndef NDEBUG
	DxgiInfoManager infoManager;
#endif
public:
	Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
private:
	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
public:

public:
	void Render();

private:
	Microsoft::WRL::ComPtr<IDXGIAdapter1> m_pCudaCapableAdapter;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>m_pVertexShader;
	Microsoft::WRL::ComPtr <ID3D11PixelShader>m_pPixelShader;
	Microsoft::WRL::ComPtr <ID3D11Buffer>m_VertexBuffer;
	Microsoft::WRL::ComPtr <ID3D11RasterizerState>m_pRasterState;
	Microsoft::WRL::ComPtr <ID3D11InputLayout>m_pLayout;
	IDXGIKeyedMutex *m_pKeyedMutex11;

	Vertex* d_VertexBufPtr = NULL;
	cudaExternalMemory_t extMemory;
	cudaExternalSemaphore_t extSemaphore;

	void Cleanup();
	bool findCUDADevice();
	static bool dynlinkLoadD3D11API();
	bool findDXDevice(char* device_name);
	cudaStream_t cuda_stream;
	char device_name[256];
};