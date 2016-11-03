//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Application Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <App.h>
#include <cstdio>
#include <DirectXMath.h>
#include <array>


#ifndef DLOG
#if defined(DEBUG) || defined(_DEBUG)
#define DLOG( x, ... ) std::sprintf( x"\n", ##__VA_ARGS__ )
#else
#define DLOG( x, ... ) ((void)0)
#endif
#endif//DLOG

#ifndef ELOG
#define ELOG( x, ... ) std::fprintf( stderr, "[File: %s, Line: %d] "x"\n", __FILE__, __LINE__, ##__VA_ARGS__ )
#endif//ELOG

#define SAMPLE_CLASSNAME TEXT("SampleClass")


namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////////////////////////////////
// SimpleVertex structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct SimpleVertex
{
    DirectX::XMFLOAT3 Position;     //!< 位置座標です.
    DirectX::XMFLOAT4 Color;        //!< 頂点カラーです.
};

//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
template<typename T>
void SafeRelease( T*& ptr )
{
    if ( ptr )
    { ptr->Release(); }

    ptr = nullptr;
}

} // namespace /* anonymous */


///////////////////////////////////////////////////////////////////////////////////////////////////
// App class
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//      コンストラクタです.
//-------------------------------------------------------------------------------------------------
App::App()
: m_hWnd                ( nullptr )
, m_hInstance           ( nullptr )
, m_Width               ( 960 )
, m_Height              ( 540 )
, m_pD2DFactory         ( nullptr )
, m_pD2DDevice          ( nullptr )
, m_pD2DDeviceContext   ( nullptr )
, m_pD2DSolidColorBrush ( nullptr )
, m_pD2DBitmap          ( nullptr )
, m_pDWriteFactory      ( nullptr )
, m_pTextFormat         ( nullptr )
, m_pD3DDevice          ( nullptr )
, m_pD3DDeviceContext   ( nullptr )
, m_pD3DRenderTargetView( nullptr )
, m_pD3DDepthStencilView( nullptr )
, m_pD3DInputLayout     ( nullptr )
, m_pD3DVertexShader    ( nullptr )
, m_pD3DPixelShader     ( nullptr )
, m_pD3DVertexBuffer    ( nullptr )
, m_pDXGISwapChain      ( nullptr )
, m_pDXGIDevice         ( nullptr )
{
}

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{
    Term();
}

//-------------------------------------------------------------------------------------------------
//      アプリケーションを実行します.
//-------------------------------------------------------------------------------------------------
void App::Run()
{
    if ( Init() )
    { MainLoop(); }

    Term();
}

//-------------------------------------------------------------------------------------------------
//      初期化処理です.
//-------------------------------------------------------------------------------------------------
bool App::Init()
{
    HRESULT hr = CoInitialize( nullptr );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : CoInitialize() Failed." );
        return false;
    }

    // ウィンドウの初期化.
    if ( !InitWnd() )
    {
        ELOG( "Error : InitWnd() Failed." );
        return false;
    }

    // Direct3D の初期化.
    if ( !InitD3D() )
    {
        ELOG( "Error : InitD3D() Failed." );
        return false;
    }

    // Direct2D の初期化.
    if ( !InitD2D() )
    {
        ELOG( "Error : InitD2D() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      終了処理です.
//-------------------------------------------------------------------------------------------------
void App::Term()
{
    TermD2D();
    TermD3D();
    TermWnd();
}

//-------------------------------------------------------------------------------------------------
//      メインループです.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = { 0 };

    while( WM_QUIT != msg.message )
    {
        if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }
}


//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化です.
//-------------------------------------------------------------------------------------------------
bool App::InitWnd()
{
    HINSTANCE hInst = GetModuleHandle( nullptr );
    if ( !hInst )
    {
        ELOG( "Error : GetModuleHandle() Failed." );
        return false;
    }

    // 拡張ウィンドウクラスの設定.
    WNDCLASSEXW wc;
    wc.cbSize           = sizeof( WNDCLASSEXW );
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = MsgProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = sizeof(LONG_PTR);
    wc.hInstance        = hInst;
    wc.hIcon            = LoadIcon( nullptr, IDI_APPLICATION );
    wc.hCursor          = LoadCursor( nullptr, IDC_ARROW );
    wc.hbrBackground    = (HBRUSH)( COLOR_WINDOW + 1 );
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = SAMPLE_CLASSNAME;
    wc.hIconSm          = LoadIcon( nullptr, IDI_APPLICATION );

    // ウィンドウクラスを登録します.
    if ( !RegisterClassExW( &wc ) )
    {
        // エラーログ出力.
        ELOG( "Error : RegisterClassEx() Failed." );

        // 異常終了.
        return false;
    }

    m_hInstance = hInst;

    // 矩形の設定.
    RECT rc = { 0, 0, m_Width, m_Height };

    // 指定されたクライアント領域を確保するために必要なウィンドウ座標を計算します.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
    AdjustWindowRect( &rc, style, FALSE );

    // ウィンドウを生成します.
    m_hWnd = CreateWindowW(
        SAMPLE_CLASSNAME,
        L"d2d simple",
        style,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        ( rc.right - rc.left ),
        ( rc.bottom - rc.top ),
        nullptr,
        nullptr,
        hInst,
        this
    );

    // 生成チェック.
    if ( !m_hWnd )
    {
        // エラーログ出力.
        ELOG( "Error : CreateWindow() Failed." );

        // 異常終了.
        return false;
    }

    // ウィンドウを表示します.
    ShowWindow( m_hWnd, SW_SHOWNORMAL );
    UpdateWindow( m_hWnd );

    // フォーカスを設定します.
    SetFocus( m_hWnd );

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      Direct3D の初期化です.
//-------------------------------------------------------------------------------------------------
bool App::InitD3D()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif//defined(DEBUG) || defined(_DEBUG)
    createDeviceFlags |= D3D11_CREATE_DEVICE_BGRA_SUPPORT;

    // 機能レベル.
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

    UINT numFeatureLevels = sizeof( featureLevels ) / sizeof( featureLevels[0] );

    // スワップチェインの設定.
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory( &sd, sizeof(sd) );
    sd.BufferCount                          = 2;
    sd.BufferDesc.Width                     = m_Width;
    sd.BufferDesc.Height                    = m_Height;
    sd.BufferDesc.Format                    = DXGI_FORMAT_B8G8R8A8_UNORM;   // Direct2D を使う関係でこのフォーマット.
    sd.BufferDesc.RefreshRate.Numerator     = 60;
    sd.BufferDesc.RefreshRate.Denominator   = 1;
    sd.BufferUsage                          = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.OutputWindow                         = m_hWnd;
    sd.SampleDesc.Count                     = 1;
    sd.SampleDesc.Quality                   = 0;
    sd.Windowed                             = TRUE;

    // Direct2D を使う場合は HARDWARE しか使えない.
    hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 
        createDeviceFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &sd,
        &m_pDXGISwapChain,
        &m_pD3DDevice,
        &m_FeatureLevel,
        &m_pD3DDeviceContext );

    if ( FAILED( hr ) )
    {
        ELOG( "Error : D3D11CreateDeviceAndSwapChain() Failed." );
        return false;
    }

    // IDXGIDeviceを取得.
    hr = m_pD3DDevice->QueryInterface( IID_IDXGIDevice, (LPVOID*)&m_pDXGIDevice );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : QueryInterface() Failed." );
        return false;
    }

    // レンダーターゲットを設定.
    {
        ID3D11Texture2D* pTexture;
        hr = m_pDXGISwapChain->GetBuffer( 0, IID_ID3D11Texture2D, (LPVOID*)&pTexture );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGSwapChain::GetBuffer() Failed." );
            SafeRelease( pTexture );
            return false;
        }

        hr = m_pD3DDevice->CreateRenderTargetView( pTexture, nullptr, &m_pD3DRenderTargetView );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateRenderTargetView() Failed." );
            SafeRelease( pTexture );
            return false;
        }

        SafeRelease( pTexture );
    }

    // 深度ステンシルビューを生成.
    {
        ID3D11Texture2D* pTexture;
        D3D11_TEXTURE2D_DESC td;
        ZeroMemory( &td, sizeof(td) );
        td.Width                = m_Width;
        td.Height               = m_Height;
        td.MipLevels            = 1;
        td.ArraySize            = 1;
        td.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
        td.SampleDesc.Count     = 1;
        td.SampleDesc.Quality   = 0;
        td.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
        td.CPUAccessFlags       = 0;
        td.MiscFlags            = 0;

        hr = m_pD3DDevice->CreateTexture2D( &td, nullptr, &pTexture );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateTexture2D() Failed." );
            SafeRelease( pTexture );
            return false;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC dd;
        ZeroMemory( &dd, sizeof(dd) );
        dd.Format        = td.Format;
        dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        hr = m_pD3DDevice->CreateDepthStencilView( pTexture, &dd, &m_pD3DDepthStencilView );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateDepthStencilView() Failed." );
            SafeRelease( pTexture );
            return false;
        }

        SafeRelease( pTexture );
    }

    // 頂点バッファを生成.
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory( &bd, sizeof(bd) );
        bd.ByteWidth = sizeof(SimpleVertex) * 3;
        bd.Usage     = D3D11_USAGE_DEFAULT;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        const std::array<SimpleVertex, 3> vertex = {{
                { {-0.3f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
                { { 0.0f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
                { { 0.3f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f} }
       }};

        D3D11_SUBRESOURCE_DATA res;
        ZeroMemory( &res, sizeof(res) );
        res.pSysMem = vertex.data();

        hr = m_pD3DDevice->CreateBuffer( &bd, &res, &m_pD3DVertexBuffer );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11dDevice::CreateBuffer() Failed." );
            return false;
        }
    }

    // 頂点シェーダ・入力レイアウト生成.
    {
        #include "../res/Compiled/SimpleVS_VSFunc.inc"

        hr = m_pD3DDevice->CreateVertexShader( SimpleVS_VSFunc, sizeof(SimpleVS_VSFunc), nullptr, &m_pD3DVertexShader );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateVertexShader() Failed." );
            return false;
        }

        D3D11_INPUT_ELEMENT_DESC elementDesc[] = {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "VTX_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
        };

        hr = m_pD3DDevice->CreateInputLayout( elementDesc, 2, SimpleVS_VSFunc, sizeof(SimpleVS_VSFunc), &m_pD3DInputLayout );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreateInputLayout() Failed." );
            return false;
        }
    }

    // ピクセルシェーダ生成.
    {
        #include "../res/Compiled/SimplePS_PSFunc.inc"

        hr = m_pD3DDevice->CreatePixelShader( SimplePS_PSFunc, sizeof(SimplePS_PSFunc), nullptr, &m_pD3DPixelShader );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : ID3D11Device::CreatePixelShader() Failed." );
            return false;
        }
    }

    // ビューポートを設定.
    m_Viewport.Width    = FLOAT( m_Width );
    m_Viewport.Height   = FLOAT( m_Height );
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;

    m_pD3DDeviceContext->RSSetViewports( 1, &m_Viewport );

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      Direct2D の初期化です.
//-------------------------------------------------------------------------------------------------
bool App::InitD2D()
{
    HRESULT hr = S_OK;

    // D2Dファクトリーを生成.
    hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pD2DFactory );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        ELOG( "Error : D2D1CreateFactory() Failed." );
        return false;
    }

    // DWriteファクトリーを生成.
    hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>( &m_pDWriteFactory ) );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        SafeRelease( m_pDWriteFactory );
        ELOG( "Error : DWriteCreateFactory() Failed." );
        return false;
    }

    // フォントの設定.
    static const WCHAR fontname[] = L"メイリオ";
    static const FLOAT fontsize = 50.0f;
    hr = m_pDWriteFactory->CreateTextFormat(
        fontname,
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontsize,
        L"",
        &m_pTextFormat );

    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        SafeRelease( m_pDWriteFactory );
        SafeRelease( m_pTextFormat );
        ELOG( "Error : IDWriteFactory::CreateTextFormat() Failed." );
        return false;
    }

    // 中央に表示するように設定.
    m_pTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
    m_pTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );

    // D2Dデバイスを生成.
    hr = m_pD2DFactory->CreateDevice( m_pDXGIDevice, &m_pD2DDevice );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1Factory1::CreateDevice() Failed." );
        return false;
    }

    // D2Dデバイスコンテキストを生成.
    hr = m_pD2DDevice->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DDeviceContext );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1Device::CreateDeviceContext() Failed." );
        return false;
    }

    // IDXGISurfaceを取得.
    IDXGISurface* pSurface;
    hr = m_pDXGISwapChain->GetBuffer( 0, IID_IDXGISurface, (LPVOID*)&pSurface );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." );
        return false;
    }

    const auto bitmapProp = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // D2Dビットマップを生成.
    hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface( pSurface, bitmapProp, &m_pD2DBitmap );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1DeviceContext::CreateBitmapFromDxgiSurface() Failed." );
        SafeRelease( pSurface );
        return false;
    }

    SafeRelease( pSurface );

    // カラーブラシを生成.
    hr = m_pD2DDeviceContext->CreateSolidColorBrush( (D2D1::ColorF(D2D1::ColorF::White)), &m_pD2DSolidColorBrush );
    if ( FAILED( hr ) )
    {
        ELOG( "Error : ID2D1DeviceContext::CreateSolidColorBrush() Failed." );
        return false;
    }

    // 正常終了.
    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermWnd()
{
    // ウィンドウクラスの登録を解除.
    if ( m_hInstance != nullptr )
    { UnregisterClass( L"d2dSampleClass", m_hInstance ); }

    m_hInstance = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      Direct3D の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermD3D()
{
    // ステートをクリアして，残っている描画コマンドを実行.
    if ( m_pD3DDeviceContext )
    {
        m_pD3DDeviceContext->ClearState();
        m_pD3DDeviceContext->Flush();
    }

    SafeRelease( m_pD3DInputLayout );
    SafeRelease( m_pD3DVertexShader );
    SafeRelease( m_pD3DPixelShader );
    SafeRelease( m_pD3DVertexBuffer );
    SafeRelease( m_pD3DDepthStencilView );
    SafeRelease( m_pD3DRenderTargetView );
    SafeRelease( m_pD3DDeviceContext );
    SafeRelease( m_pD3DDevice );

    SafeRelease( m_pDXGISwapChain );
    SafeRelease( m_pDXGIDevice );
}

//-------------------------------------------------------------------------------------------------
//      Direct2D の終了処理です.
//-------------------------------------------------------------------------------------------------
void App::TermD2D()
{
    SafeRelease( m_pTextFormat );
    SafeRelease( m_pDWriteFactory );

    SafeRelease( m_pD2DBitmap );
    SafeRelease( m_pD2DSolidColorBrush );
    SafeRelease( m_pD2DDeviceContext );
    SafeRelease( m_pD2DDevice );
    SafeRelease( m_pD2DFactory );
}

//-------------------------------------------------------------------------------------------------
//      描画処理です.
//-------------------------------------------------------------------------------------------------
void App::Render()
{
    // Direct3D を描画.
    OnRenderD3D();

    // Direct2D を描画.
    OnRenderD2D();

    // 描画コマンドをフラッシュして表示.
    m_pDXGISwapChain->Present( 0, 0 );
}

//-------------------------------------------------------------------------------------------------
//      Direct3D の描画処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRenderD3D()
{
    FLOAT clearColor[4] = { 0.392156899f, 0.584313750f, 0.929411829f, 1.0f };   // CornflowerBlue.
    UINT  stride = sizeof(SimpleVertex);
    UINT  offset = 0;

    m_pD3DDeviceContext->OMSetRenderTargets( 1, &m_pD3DRenderTargetView, m_pD3DDepthStencilView );
    m_pD3DDeviceContext->ClearRenderTargetView( m_pD3DRenderTargetView, clearColor );
    m_pD3DDeviceContext->ClearDepthStencilView( m_pD3DDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );

    m_pD3DDeviceContext->IASetInputLayout( m_pD3DInputLayout );
    m_pD3DDeviceContext->IASetVertexBuffers( 0, 1, &m_pD3DVertexBuffer, &stride, &offset );
    m_pD3DDeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    m_pD3DDeviceContext->VSSetShader( m_pD3DVertexShader, nullptr, 0 );
    m_pD3DDeviceContext->PSSetShader( m_pD3DPixelShader,  nullptr, 0 );

    m_pD3DDeviceContext->Draw( 3, 0 );
}

//-------------------------------------------------------------------------------------------------
//      Direct2D の描画処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRenderD2D()
{
    const WCHAR text[] = L"ぽえ～ん。";
    const UINT  textSize = sizeof(text) / sizeof(text[0]);
    D2D1_RECT_F layout = D2D1::RectF( 0, 0, FLOAT(m_Width), FLOAT(m_Height) );

    m_pD2DDeviceContext->SetTarget( m_pD2DBitmap );
    m_pD2DDeviceContext->BeginDraw();
    m_pD2DDeviceContext->DrawTextW( text, textSize, m_pTextFormat, layout, m_pD2DSolidColorBrush );
    m_pD2DDeviceContext->EndDraw();
}

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnResize( UINT width, UINT height )
{
    m_Width  = ( width  > 1 ) ? width  : 1;
    m_Height = ( height > 1 ) ? height : 1;

    m_Viewport.Width  = FLOAT( m_Width );
    m_Viewport.Height = FLOAT( m_Height );

    if ( m_pDXGISwapChain != nullptr
      && m_pD3DDeviceContext != nullptr )
    {
        // フラッシュしておく.
        m_pDXGISwapChain->Present( 0, 0 );

        // ターゲットを外す.
        ID3D11RenderTargetView* pNullRTV = nullptr;
        m_pD3DDeviceContext->OMSetRenderTargets( 1, &pNullRTV, nullptr );
        m_pD2DDeviceContext->SetTarget( nullptr );

        // 解放する.
        SafeRelease( m_pD3DRenderTargetView );
        SafeRelease( m_pD3DDepthStencilView );
        SafeRelease( m_pD2DBitmap );

        // バックバッファをリサイズ.
        HRESULT hr = m_pDXGISwapChain->ResizeBuffers( 2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0 );
        if ( FAILED( hr ) )
        {
            ELOG( "Error : IDXGISwapChain::ResizeBuffer() Failed." );
            return;
        }

        // レンダーターゲットを作成しなおす.
        {
            ID3D11Texture2D* pTexture;
            hr = m_pDXGISwapChain->GetBuffer( 0, IID_ID3D11Texture2D, (LPVOID*)&pTexture );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IDXGSwapChain::GetBuffer() Failed." );
                SafeRelease( pTexture );
                return;
            }

            hr = m_pD3DDevice->CreateRenderTargetView( pTexture, nullptr, &m_pD3DRenderTargetView );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : ID3D11Device::CreateRenderTargetView() Failed." );
                SafeRelease( pTexture );
                return;
            }

            SafeRelease( pTexture );
        }

        // 深度ステンシルビューを生成しなおす.
        {
            ID3D11Texture2D* pTexture;
            D3D11_TEXTURE2D_DESC td;
            ZeroMemory( &td, sizeof(td) );
            td.Width                = m_Width;
            td.Height               = m_Height;
            td.MipLevels            = 1;
            td.ArraySize            = 1;
            td.Format               = DXGI_FORMAT_D24_UNORM_S8_UINT;
            td.SampleDesc.Count     = 1;
            td.SampleDesc.Quality   = 0;
            td.BindFlags            = D3D11_BIND_DEPTH_STENCIL;
            td.CPUAccessFlags       = 0;
            td.MiscFlags            = 0;

            hr = m_pD3DDevice->CreateTexture2D( &td, nullptr, &pTexture );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : ID3D11Device::CreateTexture2D() Failed." );
                SafeRelease( pTexture );
                return;
            }

            D3D11_DEPTH_STENCIL_VIEW_DESC dd;
            ZeroMemory( &dd, sizeof(dd) );
            dd.Format        = td.Format;
            dd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            hr = m_pD3DDevice->CreateDepthStencilView( pTexture, &dd, &m_pD3DDepthStencilView );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : ID3D11Device::CreateDepthStencilView() Failed." );
                SafeRelease( pTexture );
                return;
            }

            SafeRelease( pTexture );
        }

        // D2Dビットマップを作成しなおす.
        {
            IDXGISurface* pSurface;
            hr = m_pDXGISwapChain->GetBuffer( 0, IID_IDXGISurface, (LPVOID*)&pSurface );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : IDXGISwapChain::GetBuffer() Failed." );
                return;
            }

            const auto bitmapProp = D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

            hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface( pSurface, bitmapProp, &m_pD2DBitmap );
            if ( FAILED( hr ) )
            {
                ELOG( "Error : ID2D1DeviceContext::CreateBitmapFromDxgiSurface() Failed." );
                SafeRelease( pSurface );
                return;
            }

            SafeRelease( pSurface );
        }

        // ビューポートを設定.
        m_Viewport.Width    = FLOAT( m_Width );
        m_Viewport.Height   = FLOAT( m_Height );
        m_Viewport.MinDepth = 0.0f;
        m_Viewport.MaxDepth = 1.0f;
        m_Viewport.TopLeftX = 0;
        m_Viewport.TopLeftY = 0;
        m_pD3DDeviceContext->RSSetViewports( 1, &m_Viewport );
    }
}

//-------------------------------------------------------------------------------------------------
//      メッセージプロシージャです.
//-------------------------------------------------------------------------------------------------
LRESULT App::MsgProc( HWND hWnd, UINT uMsg, WPARAM wp, LPARAM lp )
{
    if ( uMsg == WM_CREATE )
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lp;
        App *pApp = (App *)pcs->lpCreateParams;

        SetWindowLongPtrW( hWnd, GWLP_USERDATA, PtrToUlong(pApp) );
    }
    else
    {
        App *pApp = reinterpret_cast<App*>(
            static_cast<LONG_PTR>( GetWindowLongPtrW( hWnd, GWLP_USERDATA )));

        switch( uMsg )
        {
            case WM_DESTROY:
                { PostQuitMessage( 0 ); }
                break;

            case WM_SIZE:
                {
                    UINT w = (UINT)LOWORD( lp );
                    UINT h = (UINT)HIWORD( lp );

                    if ( pApp )
                    { pApp->OnResize( w, h ); }
                }
                break;

            default:
                { /* DO_NOTHING */ }
                break;
        }
    }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}