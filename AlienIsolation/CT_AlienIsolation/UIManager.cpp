#include "UIManager.h"
#include "Main.h"
#include "AlienIsolation.h"
#include "Util/Log.h"
#include "ImGui/imgui_impl_dx11.h"

#include <CommonStates.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#pragma comment(lib, "DirectXTK.lib")

std::unique_ptr< PrimitiveBatch< VertexPositionColor > > m_PrimitiveBatch;
std::unique_ptr<CommonStates> m_States;
std::unique_ptr<BasicEffect> m_Effect;
ID3D11InputLayout* m_pInputLayout;

UIManager::UIManager()
{
  m_initialized = false;

  IDXGISwapChain* pSwapChain = AI::Rendering::GetSwapChain();
  ID3D11DeviceContext* pContext = AI::Rendering::GetContext();
  ID3D11Device* pDevice = AI::Rendering::GetDevice();

  printf("0x%X\n", pSwapChain);
  printf("0x%X\n", pContext);
  printf("0x%X\n", pDevice);

  Log::Write("IDXGISwapChain 0x%X", pSwapChain);
  Log::Write("ID3D11DeviceContext 0x%X", pContext);
  Log::Write("ID3D11Device 0x%X", pDevice);

  ID3D11Device* pDevice_Swap = nullptr;
  ID3D11DeviceContext* pContext_Swap = nullptr;
  pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice_Swap);
  pDevice_Swap->GetImmediateContext(&pContext_Swap);

  Log::Write("ID3D11Device through SwapChain 0x%X", pDevice_Swap);
  Log::Write("ID3D11DeviceContext through SwapChain 0x%X", pContext_Swap);

  m_States = std::make_unique<CommonStates>(pDevice);
  m_Effect = std::make_unique<BasicEffect>(pDevice);
  m_Effect->SetVertexColorEnabled(true);
  m_Effect->SetProjection(XMMatrixOrthographicOffCenterRH(0.f, (float)1920, (float)1077, 0.f, 0.f, 1.f));

  Log::Write("HWND 0x%X", AI::Rendering::GetHwnd());
  Log::Write("FindWindow 0x%X", FindWindow(L"Alien: Isolation", NULL));

  void const* shaderByteCode;
  size_t byteCodeLength;

  m_Effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

  if (FAILED(pDevice->CreateInputLayout(VertexPositionColor::InputElements,
    VertexPositionColor::InputElementCount,
    shaderByteCode, byteCodeLength,
    &m_pInputLayout)))
  {
    MessageBox(nullptr, L"m_pDevice->CreateInputLayout failed", L"Error", MB_OK | MB_ICONERROR);
    return;
  }

  m_PrimitiveBatch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(pContext);

  if (!ImGui_ImplDX11_Init((void*)FindWindow(L"Alien: Isolation", NULL), pDevice, pContext))
  {
    Log::Error("Failed to initialize ImGui DX11 handler. GetLastError 0x%X", GetLastError());
    return;
  }

  DWORD thread_id = GetWindowThreadProcessId(FindWindowA("Alien: Isolation", NULL), NULL);
  if (!thread_id)
    thread_id = GetWindowThreadProcessId(FindWindowA(NULL, "Alien: Isolation"), NULL);

  if (thread_id)
  {
    if (!(hGetMessage = SetWindowsHookEx(WH_GETMESSAGE, this->GetMessage_Callback, g_mainHandle->GetDllHandle(), thread_id)))
      Log::Write("Couldn't create WH_GETMESSAGE hook. LastError 0x%X", GetLastError());
  }
  else 
    Log::Warning("Could not find window thread handle");

  m_enabled = false;
  m_hasKeyboardFocus = false;
  m_hasMouseFocus = false;

  ImGuiStyle& Style = ImGui::GetStyle();
  Style.WindowRounding = 0.0f;
  Style.ChildWindowRounding = 0.0f;
  Style.Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
  Style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);

  m_initialized = true;
}

void UIManager::Toggle()
{
  m_enabled = !m_enabled;
  AI::Input::Mouse* pMouse = AI::Input::GetMouse();
  if (pMouse)
  {
    pMouse->m_showCursor = m_enabled;
    pMouse->m_enableGameInput = !m_enabled;
  }
}

void UIManager::Draw()
{
  if (!m_enabled || !m_initialized) return;

  AI::Input::Mouse* pMouse = AI::Input::GetMouse();
  if (pMouse)
    pMouse->m_showCursor = m_enabled;

  ImGui_ImplDX11_NewFrame();
  {
    ImGui::ShowTestWindow();

    ImGui::SetNextWindowSize(ImVec2(300, 500));
    ImGui::Begin("Cinematic Tools for Alien: Isolation", nullptr);
    {
      g_mainHandle->GetCameraManager()->DrawUI();

    } ImGui::End();
  }

  ImGui::Render();

  m_hasKeyboardFocus = ImGui::GetIO().WantCaptureKeyboard;
  m_hasMouseFocus = ImGui::GetIO().WantCaptureMouse;
}

UIManager::~UIManager()
{
  
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall UIManager::GetMessage_Callback(int nCode, WPARAM wParam, LPARAM lParam)
{
  if (AI::Rendering::HasFocus())
  {
    MSG* pMsg = (MSG*)lParam;
    if (!ImGui_ImplDX11_WndProcHandler(pMsg->hwnd, pMsg->message, pMsg->wParam, pMsg->lParam))
    {
      if (pMsg->message == WM_SIZE)
      {

      }
    }
  }

  return CallNextHookEx(g_mainHandle->GetUIManager()->GetMessageHook(), nCode, wParam, lParam);
}


