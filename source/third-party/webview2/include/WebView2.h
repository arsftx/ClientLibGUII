#pragma once
// ============================================================================
// Minimal WebView2 Header for ECSRO ClientLib
// Pure C++ abstract classes - Compatible with older compilers
// ============================================================================

#ifndef __webview2_minimal_h__
#define __webview2_minimal_h__

#include <windows.h>
#include <unknwn.h>

// EventRegistrationToken - required for WebView2 callbacks
#ifndef _EVENTREGISTRATIONTOKEN_DEFINED
#define _EVENTREGISTRATIONTOKEN_DEFINED
typedef struct EventRegistrationToken {
    __int64 value;
} EventRegistrationToken;
#endif

// Forward declarations
class ICoreWebView2;
class ICoreWebView2Controller;
class ICoreWebView2Environment;
class ICoreWebView2Settings;
class ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler;
class ICoreWebView2CreateCoreWebView2ControllerCompletedHandler;
class ICoreWebView2EnvironmentOptions;

// ============================================================================
// GUIDs
// ============================================================================

// {b96d755e-0319-4e92-a296-23436f46a1fc}
static const GUID IID_ICoreWebView2Environment = 
    { 0xb96d755e, 0x0319, 0x4e92, { 0xa2, 0x96, 0x23, 0x43, 0x6f, 0x46, 0xa1, 0xfc } };

// {4d00c0d1-9434-4eb6-8078-8697a560334f}
static const GUID IID_ICoreWebView2Controller = 
    { 0x4d00c0d1, 0x9434, 0x4eb6, { 0x80, 0x78, 0x86, 0x97, 0xa5, 0x60, 0x33, 0x4f } };

// {76eceacb-0462-4d94-ac83-423a6793775e}
static const GUID IID_ICoreWebView2 = 
    { 0x76eceacb, 0x0462, 0x4d94, { 0xac, 0x83, 0x42, 0x3a, 0x67, 0x93, 0x77, 0x5e } };

// {4e8a3389-c9d8-4bd2-b6b5-124fee6cc14d}
static const GUID IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler = 
    { 0x4e8a3389, 0xc9d8, 0x4bd2, { 0xb6, 0xb5, 0x12, 0x4f, 0xee, 0x6c, 0xc1, 0x4d } };

// {6c4819f3-c9b7-4260-8127-c9f5bde7f68c}
static const GUID IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler = 
    { 0x6c4819f3, 0xc9b7, 0x4260, { 0x81, 0x27, 0xc9, 0xf5, 0xbd, 0xe7, 0xf6, 0x8c } };

// ============================================================================
// ICoreWebView2 - Main browser interface (simplified)
// ============================================================================
class ICoreWebView2 : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE get_Settings(ICoreWebView2Settings** settings) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Source(LPWSTR* uri) = 0;
    virtual HRESULT STDMETHODCALLTYPE Navigate(LPCWSTR uri) = 0;
    virtual HRESULT STDMETHODCALLTYPE NavigateToString(LPCWSTR htmlContent) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_NavigationStarting(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_NavigationStarting(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ContentLoading(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ContentLoading(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_SourceChanged(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_SourceChanged(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_HistoryChanged(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_HistoryChanged(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_NavigationCompleted(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_NavigationCompleted(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_FrameNavigationStarting(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_FrameNavigationStarting(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_FrameNavigationCompleted(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_FrameNavigationCompleted(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ScriptDialogOpening(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ScriptDialogOpening(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_PermissionRequested(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_PermissionRequested(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ProcessFailed(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ProcessFailed(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddScriptToExecuteOnDocumentCreated(LPCWSTR javaScript, void* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveScriptToExecuteOnDocumentCreated(LPCWSTR id) = 0;
    virtual HRESULT STDMETHODCALLTYPE ExecuteScript(LPCWSTR javaScript, void* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE CapturePreview(int imageFormat, void* imageStream, void* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE Reload() = 0;
    virtual HRESULT STDMETHODCALLTYPE PostWebMessageAsJson(LPCWSTR webMessageAsJson) = 0;
    virtual HRESULT STDMETHODCALLTYPE PostWebMessageAsString(LPCWSTR webMessageAsString) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_WebMessageReceived(void* handler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_WebMessageReceived(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE CallDevToolsProtocolMethod(LPCWSTR methodName, LPCWSTR parametersAsJson, void* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_BrowserProcessId(UINT32* value) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_CanGoBack(BOOL* canGoBack) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_CanGoForward(BOOL* canGoForward) = 0;
    virtual HRESULT STDMETHODCALLTYPE GoBack() = 0;
    virtual HRESULT STDMETHODCALLTYPE GoForward() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDevToolsProtocolEventReceiver(LPCWSTR eventName, void** receiver) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stop() = 0;
    virtual HRESULT STDMETHODCALLTYPE add_NewWindowRequested(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_NewWindowRequested(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_DocumentTitleChanged(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_DocumentTitleChanged(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_DocumentTitle(LPWSTR* title) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddHostObjectToScript(LPCWSTR name, VARIANT* object) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveHostObjectFromScript(LPCWSTR name) = 0;
    virtual HRESULT STDMETHODCALLTYPE OpenDevToolsWindow() = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ContainsFullScreenElementChanged(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ContainsFullScreenElementChanged(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ContainsFullScreenElement(BOOL* containsFullScreenElement) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_WebResourceRequested(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_WebResourceRequested(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE AddWebResourceRequestedFilter(LPCWSTR uri, int resourceContext) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveWebResourceRequestedFilter(LPCWSTR uri, int resourceContext) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_WindowCloseRequested(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_WindowCloseRequested(EventRegistrationToken token) = 0;
};

// ============================================================================
// ICoreWebView2Controller - Browser controller for sizing/visibility
// ============================================================================
class ICoreWebView2Controller : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE get_IsVisible(BOOL* isVisible) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_IsVisible(BOOL isVisible) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_Bounds(RECT* bounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_Bounds(RECT bounds) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ZoomFactor(double* zoomFactor) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_ZoomFactor(double zoomFactor) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_ZoomFactorChanged(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_ZoomFactorChanged(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBoundsAndZoomFactor(RECT bounds, double zoomFactor) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveFocus(int reason) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_MoveFocusRequested(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_MoveFocusRequested(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_GotFocus(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_GotFocus(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_LostFocus(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_LostFocus(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_AcceleratorKeyPressed(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_AcceleratorKeyPressed(EventRegistrationToken token) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_ParentWindow(HWND* parentWindow) = 0;
    virtual HRESULT STDMETHODCALLTYPE put_ParentWindow(HWND parentWindow) = 0;
    virtual HRESULT STDMETHODCALLTYPE NotifyParentWindowPositionChanged() = 0;
    virtual HRESULT STDMETHODCALLTYPE Close() = 0;
    virtual HRESULT STDMETHODCALLTYPE get_CoreWebView2(ICoreWebView2** coreWebView2) = 0;
};

// ============================================================================
// ICoreWebView2Environment - Factory for creating WebView2 instances
// ============================================================================
class ICoreWebView2Environment : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE CreateCoreWebView2Controller(
        HWND parentWindow,
        ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* handler) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateWebResourceResponse(
        void* content, int statusCode, LPCWSTR reasonPhrase, LPCWSTR headers, void** response) = 0;
    virtual HRESULT STDMETHODCALLTYPE get_BrowserVersionString(LPWSTR* versionInfo) = 0;
    virtual HRESULT STDMETHODCALLTYPE add_NewBrowserVersionAvailable(void* eventHandler, EventRegistrationToken* token) = 0;
    virtual HRESULT STDMETHODCALLTYPE remove_NewBrowserVersionAvailable(EventRegistrationToken token) = 0;
};

// ============================================================================
// Callback Handlers
// ============================================================================
class ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Environment* createdEnvironment) = 0;
};

class ICoreWebView2CreateCoreWebView2ControllerCompletedHandler : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE Invoke(HRESULT errorCode, ICoreWebView2Controller* createdController) = 0;
};

#endif // __webview2_minimal_h__
