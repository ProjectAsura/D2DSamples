//-------------------------------------------------------------------------------------------------
// File : App.h
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

#ifndef __APP_H__
#define __APP_H__

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <Windows.h>
#include <d2d1_2.h>     // Direct2D 1.2
#include <dwrite.h>     // DirectWrite
#include <d3d11.h>      // Direct3D 11


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////
class App
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    App();
    virtual ~App();
    void Run();

protected:
    //=============================================================================================
    // protected variables.
    //=============================================================================================
    bool InitWnd();
    void TermWnd();
    bool InitD2D();
    void TermD2D();
    bool InitD3D();
    void TermD3D();
    void OnRenderD3D();
    void OnRenderD2D();
    void OnResize( UINT width, UINT height );

    //=============================================================================================
    // protected methods.
    //=============================================================================================
    /* NOTHING */

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    HWND                    m_hWnd;
    HINSTANCE               m_hInstance;
    UINT                    m_Width;
    UINT                    m_Height;

    // Direct2D / DirectWrite
    ID2D1Factory1*          m_pD2DFactory;
    ID2D1Device*            m_pD2DDevice;
    ID2D1DeviceContext*     m_pD2DDeviceContext;
    ID2D1SolidColorBrush*   m_pD2DSolidColorBrush;
    ID2D1Bitmap1*           m_pD2DBitmap;
    IDWriteFactory*         m_pDWriteFactory;
    IDWriteTextFormat*      m_pTextFormat;

    // Direct3D 11
    ID3D11Device*           m_pD3DDevice;
    ID3D11DeviceContext*    m_pD3DDeviceContext;
    ID3D11RenderTargetView* m_pD3DRenderTargetView;
    ID3D11DepthStencilView* m_pD3DDepthStencilView;
    ID3D11InputLayout*      m_pD3DInputLayout;
    ID3D11VertexShader*     m_pD3DVertexShader;
    ID3D11PixelShader*      m_pD3DPixelShader;
    ID3D11Buffer*           m_pD3DVertexBuffer;
    D3D_FEATURE_LEVEL       m_FeatureLevel;
    D3D11_VIEWPORT          m_Viewport;

    // DXGI
    IDXGISwapChain*         m_pDXGISwapChain;
    IDXGIDevice*            m_pDXGIDevice;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    bool Init();
    void Term();
    void MainLoop();
    void Render();

    static LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp );
};

#endif//__APP_H__
