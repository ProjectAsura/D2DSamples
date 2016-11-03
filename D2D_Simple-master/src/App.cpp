//-------------------------------------------------------------------------------------------------
// File : App.cpp
// Desc : Sample Application
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "App.h"
#include <cstdio>


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


namespace /* anonymous */ {

//-------------------------------------------------------------------------------------------------
// Constant Values.
//-------------------------------------------------------------------------------------------------
static const WCHAR SAMPLE_CLASSNAME[] = L"SampleClass";


//-------------------------------------------------------------------------------------------------
//      解放処理を行います.
//-------------------------------------------------------------------------------------------------
template <typename T>
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
: m_hInstance       ( nullptr )
, m_hWnd            ( nullptr )
, m_Width           ( 960 )
, m_Height          ( 540 )
, m_pD2DFactory     ( nullptr )
, m_pDWriteFactory  ( nullptr )
, m_pRenderTarget   ( nullptr )
, m_pTextFormat     ( nullptr )
, m_pBrush          ( nullptr )
{ /* DO_NOTHING */ }

//-------------------------------------------------------------------------------------------------
//      デストラクタです.
//-------------------------------------------------------------------------------------------------
App::~App()
{ Term(); }

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
    // ウィンドウ初期化.
    if ( !OnInitWnd() )
    {
        ELOG( "Error : OnInitWnd() Failed. ");
        return false;
    }

    // D2D初期化.
    if ( !OnInitD2D() )
    {
        ELOG( "Error : OnInitD2D() Failed." );
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
    // D2D終了処理
    OnTermD2D();

    // ウィンドウ終了処理
    OnTermWnd();
}

//-------------------------------------------------------------------------------------------------
//      メインループです.
//-------------------------------------------------------------------------------------------------
void App::MainLoop()
{
    MSG msg = { 0 };

    while( WM_QUIT != msg.message )
    {
        if ( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) == TRUE )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
           OnRender();
        }
    }
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの初期化です.
//-------------------------------------------------------------------------------------------------
bool App::OnInitWnd()
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
    wc.cbWndExtra       = 0;
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

    return true;
}

//-------------------------------------------------------------------------------------------------
//      Direct2Dの初期化です..
//-------------------------------------------------------------------------------------------------
bool App::OnInitD2D()
{
    HRESULT hr = S_OK;
    hr = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, &m_pD2DFactory );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        ELOG( "Error : D2D1CreateFactory() Failed." );
        return false;
    }

    hr = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown**>( &m_pDWriteFactory ) );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        SafeRelease( m_pDWriteFactory );
        ELOG( "Error : DWriteCreateFactory() Failed." );
        return false;
    }

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

    m_pTextFormat->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
    m_pTextFormat->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );

    D2D1_SIZE_U size = D2D1::SizeU( m_Width, m_Height );
    hr = m_pD2DFactory->CreateHwndRenderTarget( 
        D2D1::RenderTargetProperties(), 
        D2D1::HwndRenderTargetProperties(m_hWnd, size),
        &m_pRenderTarget );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        SafeRelease( m_pDWriteFactory );
        SafeRelease( m_pTextFormat );
        SafeRelease( m_pRenderTarget );
        ELOG( "Error : ID2D1Factory::CreateHwndRenderTarget() Failed." );
        return false;
    }

    hr = m_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF( D2D1::ColorF::Black ),
        &m_pBrush );
    if ( FAILED( hr ) )
    {
        SafeRelease( m_pD2DFactory );
        SafeRelease( m_pDWriteFactory );
        SafeRelease( m_pTextFormat );
        SafeRelease( m_pRenderTarget );
        SafeRelease( m_pBrush );
        ELOG( "Error : ID2D1HWndRenderTarget::CreateSolidColorBrush() Failed.");
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------------------------
//      ウィンドウの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTermWnd()
{
    // ウィンドウクラスの登録を解除.
    if ( m_hInstance != nullptr )
    { UnregisterClass( L"d2dSampleClass", m_hInstance ); }

    m_hInstance = nullptr;
}

//-------------------------------------------------------------------------------------------------
//      Direct2Dの終了処理です.
//-------------------------------------------------------------------------------------------------
void App::OnTermD2D()
{
    SafeRelease( m_pBrush );
    SafeRelease( m_pRenderTarget );
    SafeRelease( m_pTextFormat );
    SafeRelease( m_pD2DFactory );
    SafeRelease( m_pDWriteFactory );
}

//-------------------------------------------------------------------------------------------------
//      描画処理です.
//-------------------------------------------------------------------------------------------------
void App::OnRender()
{
    D2D1_SIZE_F size = m_pRenderTarget->GetSize();

    WCHAR text[] = L"ぽえ～ん。";
    D2D1_RECT_F layout = D2D1::RectF( 0, 0, size.width, size.height );

    m_pRenderTarget->BeginDraw();

    m_pRenderTarget->Clear( D2D1::ColorF( D2D1::ColorF::White ) );
    m_pRenderTarget->SetTransform( D2D1::Matrix3x2F::Identity() );

    m_pRenderTarget->DrawTextW(
        text, 
        ARRAYSIZE(text) -1,
        m_pTextFormat,
        layout,
        m_pBrush );
    m_pRenderTarget->EndDraw();
}

//-------------------------------------------------------------------------------------------------
//      リサイズ時の処理です.
//-------------------------------------------------------------------------------------------------
void App::OnResize( UINT width, UINT height )
{
    D2D1_SIZE_U size;
    size.width  = width;
    size.height = height;

    m_pRenderTarget->Resize( size );
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
        }
    }

    return DefWindowProc( hWnd, uMsg, wp, lp );
}