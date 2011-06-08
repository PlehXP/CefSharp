#include "stdafx.h"

#include "CefWpfWebBrowser.h"

namespace CefSharp
{
    void CefWpfWebBrowser::OnInitialized()
    {
        _browserInitialized->Set();
    }

    void CefWpfWebBrowser::SetTitle(String^ title)
    {
        _title = title;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Title"));
    }

    void CefWpfWebBrowser::SetAddress(String^ address)
    {
        _address = address;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Address"));
    }

    void CefWpfWebBrowser::SetNavState(bool isLoading, bool canGoBack, bool canGoForward)
    {
        if(isLoading != _isLoading) 
        {
            _isLoading = isLoading;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"IsLoading"));
        }

        if(canGoBack != _canGoBack) 
        {
            _canGoBack = canGoBack;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoBack"));
        }

        if(canGoForward != _canGoForward)
        {
            _canGoForward = canGoForward;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoForward"));
        }
    }

    void CefWpfWebBrowser::AddFrame(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->AddCount();
    }

    void CefWpfWebBrowser::FrameLoadComplete(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->Signal();
    }

    void CefWpfWebBrowser::SetJsResult(String^ result)
    {
        _jsResult = result;
        _runJsFinished->Set();
    }

    void CefWpfWebBrowser::SetJsError()
    {
        _jsError = true;
        _runJsFinished->Set();
    }

    void CefWpfWebBrowser::RaiseConsoleMessage(String^ message, String^ source, int line)
    {
        ConsoleMessage(this, gcnew ConsoleMessageEventArgs(message, source, line));
    }

    Size CefWpfWebBrowser::ArrangeOverride(Size size)
    {
        try
        {
            _clientAdapter->GetCefBrowser()->SetSize(PET_VIEW, (int)size.Width, (int)size.Height);
        }
        catch (...)
        {
            // ArrangeOverride may be called one or more times before Cef is initialized
        }

        return Image::ArrangeOverride(size);
    }

    void CefWpfWebBrowser::OnGotFocus(RoutedEventArgs^ e)
    {
        _clientAdapter->GetCefBrowser()->SendFocusEvent(true);
        Image::OnGotFocus(e);
    }

    void CefWpfWebBrowser::OnLostFocus(RoutedEventArgs^ e)
    {
        _clientAdapter->GetCefBrowser()->SendFocusEvent(false);
        Image::OnLostFocus(e);
    }

    void CefWpfWebBrowser::OnKeyDown(KeyEventArgs^ e)
    {
        Console::WriteLine("key down: {0}", e->Key);
    }

    void CefWpfWebBrowser::OnKeyUp(KeyEventArgs^ e)
    {
        Console::WriteLine("key up: {0}", e->Key);
    }

    void CefWpfWebBrowser::OnMouseMove(MouseEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        _clientAdapter->GetCefBrowser()->SendMouseMoveEvent((int)point.X, (int)point.Y, false);
    }

    void CefWpfWebBrowser::OnMouseWheel(MouseWheelEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        _clientAdapter->GetCefBrowser()->SendMouseWheelEvent((int)point.X, (int)point.Y, e->Delta);
    }

    void CefWpfWebBrowser::OnMouseLeave(MouseEventArgs^ e)
    {
        _clientAdapter->GetCefBrowser()->SendMouseMoveEvent(0, 0, true);
    }

    void CefWpfWebBrowser::OnMouseDown(MouseButtonEventArgs^ e)
    {
        Keyboard::Focus(this); // XXX: temporary

        Point point = e->GetPosition(this);
        CefBrowser::MouseButtonType mbt;
        if (e->RightButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_RIGHT;
        }
        else if (e->LeftButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_LEFT;
        }

        _clientAdapter->GetCefBrowser()->SendMouseClickEvent((int)point.X, (int)point.Y, mbt, false, 1);
    }

    void CefWpfWebBrowser::OnMouseUp(MouseButtonEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        CefBrowser::MouseButtonType mbt;
        if (e->RightButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_RIGHT;
        }
        else if (e->LeftButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_LEFT;
        }

        _clientAdapter->GetCefBrowser()->SendMouseClickEvent((int)point.X, (int)point.Y, mbt, true, 1);
    }

    void CefWpfWebBrowser::SetCursor(CefCursorHandle cursor)
    {
        SafeFileHandle^ handle = gcnew SafeFileHandle((IntPtr)cursor, false);
        Dispatcher->BeginInvoke(DispatcherPriority::Render,
            gcnew Action<SafeFileHandle^>(this, &CefWpfWebBrowser::SetCursor), handle);
    }

    void CefWpfWebBrowser::SetCursor(SafeFileHandle^ handle)
    {
        Cursor = CursorInteropHelper::Create(handle);
    }

    void CefWpfWebBrowser::SetBuffer(int width, int height, const CefRect& dirtyRect, const void* buffer)
    {
        _width = width;
        _height = height;
        _buffer = (void *)buffer;

        Dispatcher->Invoke(DispatcherPriority::Render,
            gcnew Action<WriteableBitmap^>(this, &CefWpfWebBrowser::SetBitmap), _bitmap);
    }

    // XXX: don't know how to Invoke a parameterless delegate...
    void CefWpfWebBrowser::SetBitmap(WriteableBitmap^ bitmap)
    {
        int length = _width * _height * 4;

        if (!_bitmap ||
            _bitmap->PixelWidth != _width ||
            _bitmap->PixelHeight != _height)
        {
            Source = _bitmap = gcnew WriteableBitmap(_width, _height, 96 * _transform.M11, 96 * _transform.M22, PixelFormats::Bgr32, nullptr);
        }

        Int32Rect rect;
        rect.X = 0;
        rect.Y = 0;
        rect.Width = _width;
        rect.Height = _height;

        _bitmap->WritePixels(rect, (IntPtr)_buffer, length, _bitmap->BackBufferStride);
    }
}
/*
#include "CefWpfWebBrowser.h"

namespace CefSharp
{
    void CefWpfWebBrowser::OnInitialized()
    {
        _browserInitialized->Set();
    }

    Size CefWpfWebBrowser::ArrangeOverride(Size size)
    {
        try
        {
            _handlerAdapter->GetCefBrowser()->SetSize(PET_VIEW, (int)size.Width, (int)size.Height);
        }
        catch (...)
        {
            // ArrangeOverride may be called one or more times before Cef is initialized
        }

        return Image::ArrangeOverride(size);
    }

    void CefWpfWebBrowser::OnGotFocus(RoutedEventArgs^ e)
    {
        _handlerAdapter->GetCefBrowser()->SendFocusEvent(true);
        Image::OnGotFocus(e);
    }

    void CefWpfWebBrowser::OnLostFocus(RoutedEventArgs^ e)
    {
        _handlerAdapter->GetCefBrowser()->SendFocusEvent(false);
        Image::OnLostFocus(e);
    }

    void CefWpfWebBrowser::OnMouseMove(MouseEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        _handlerAdapter->GetCefBrowser()->SendMouseMoveEvent((int)point.X, (int)point.Y, false);
    }

    void CefWpfWebBrowser::OnMouseWheel(MouseWheelEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        _handlerAdapter->GetCefBrowser()->SendMouseWheelEvent((int)point.X, (int)point.Y, e->Delta);
    }

    void CefWpfWebBrowser::OnMouseLeave(MouseEventArgs^ e)
    {
        _handlerAdapter->GetCefBrowser()->SendMouseMoveEvent(0, 0, true);
    }

    void CefWpfWebBrowser::OnMouseDown(MouseButtonEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        CefBrowser::MouseButtonType mbt;
        if (e->RightButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_RIGHT;
        }
        else
        {
            mbt = CefBrowser::MouseButtonType::MBT_LEFT;
        }

        _handlerAdapter->GetCefBrowser()->SendMouseClickEvent((int)point.X, (int)point.Y, mbt, false, 1);
    }

    void CefWpfWebBrowser::OnMouseUp(MouseButtonEventArgs^ e)
    {
        Point point = e->GetPosition(this);
        CefBrowser::MouseButtonType mbt;
        if (e->RightButton == MouseButtonState::Pressed)
        {
            mbt = CefBrowser::MouseButtonType::MBT_RIGHT;
        }
        else
        {
            mbt = CefBrowser::MouseButtonType::MBT_LEFT;
        }

        _handlerAdapter->GetCefBrowser()->SendMouseClickEvent((int)point.X, (int)point.Y, mbt, true, 1);
    }

    void CefWpfWebBrowser::SetCursor(CefCursorHandle cursor)
    {
        SafeFileHandle^ handle = gcnew SafeFileHandle((IntPtr)cursor, false);
        Dispatcher->BeginInvoke(DispatcherPriority::Render,
            gcnew Action<SafeFileHandle^>(this, &CefWpfWebBrowser::SetCursor), handle);
    }

    void CefWpfWebBrowser::SetCursor(SafeFileHandle^ handle)
    {
        Cursor = CursorInteropHelper::Create(handle);
    }

    void CefWpfWebBrowser::SetBuffer(int width, int height, const CefRect& dirtyRect, const void* buffer)
    {
        _width = width;
        _height = height;
        _buffer = (void *)buffer;

        Dispatcher->Invoke(DispatcherPriority::Render,
            gcnew Action<WriteableBitmap^>(this, &CefWpfWebBrowser::SetBitmap), _bitmap);
    }

    // XXX: don't know how to Invoke a parameterless delegate...
    void CefWpfWebBrowser::SetBitmap(WriteableBitmap^ bitmap)
    {
        int length = _width * _height * 4;

        if (!_bitmap ||
            _bitmap->PixelWidth != _width ||
            _bitmap->PixelHeight != _height)
        {
            Source = _bitmap = gcnew WriteableBitmap(_width, _height, 96 * _transform.M11, 96 * _transform.M22, PixelFormats::Bgr32, nullptr);
        }

        Int32Rect rect;
        rect.X = 0;
        rect.Y = 0;
        rect.Width = _width;
        rect.Height = _height;

        _bitmap->WritePixels(rect, (IntPtr)_buffer, length, _bitmap->BackBufferStride);
    }
}

/*
#include "stdafx.h"

#include "CefWebBrowser.h"
#include "JsTask.h"
#include "ScriptException.h"

namespace CefSharp
{
    void CefWebBrowser::Load(String^ url)
    {
        WaitForInitialized();

        _loadCompleted->Reset();
        _handlerAdapter->GetCefBrowser()->GetMainFrame()->LoadURL(toNative(url));
    }

    void CefWebBrowser::Stop()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->StopLoad();
    }

    void CefWebBrowser::Back()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->GoBack();
    }

    void CefWebBrowser::Forward()
    {
    	WaitForInitialized();

        _handlerAdapter->GetCefBrowser()->GoForward();
    }

    void CefWebBrowser::Reload()
    {
        Reload(false);
    }

    void CefWebBrowser::Reload(bool ignoreCache)
    {
    	WaitForInitialized();

        if(ignoreCache)
        {
            _handlerAdapter->GetCefBrowser()->ReloadIgnoreCache();
        }
        else
        {
            _handlerAdapter->GetCefBrowser()->Reload();
        }
    }

    String^ CefWebBrowser::RunScript(String^ script, String^ scriptUrl, int startLine)
    {
    	WaitForInitialized();

        return RunScript(script, scriptUrl, startLine, -1);
    }

    String^ CefWebBrowser::RunScript(String^ script, String^ scriptUrl, int startLine, int timeout)
    {
    	WaitForInitialized();

        
        _jsError = false;
        _jsResult = nullptr;
/*
        script = 
            "(function() {"
            "   try { "
            "      __js_run_done(" + script + ");"
            "   } catch(e) {"
            "      __js_run_err(e);"
            "   }"
            "})();";

        
        CefRefPtr<JsTask> task = new JsTask(this, toNative(script), toNative(scriptUrl), startLine);
        _handlerAdapter->GetCefBrowser()->GetMainFrame()->ExecuteJavaScriptTask(static_cast<CefRefPtr<CefV8Task>>(task));

        if(!_runJsFinished->WaitOne(timeout))
        {
            throw gcnew TimeoutException(L"Timed out waiting for JavaScript to return");
        }

        if(_jsError == false) 
        {
            return _jsResult;
        }
        throw gcnew ScriptException("An error occurred during javascript execution");
    }

    void CefWebBrowser::OnInitialized()
    {
        BeginInvoke(gcnew Action<EventArgs^>(this, &CefWebBrowser::OnSizeChanged), EventArgs::Empty);
        _browserInitialized->Set();
    }

    void CefWebBrowser::OnHandleCreated(EventArgs^ e)
    {
        if (DesignMode == false) 
        {
            _handlerAdapter = new HandlerAdapter(this);
            CefRefPtr<HandlerAdapter> ptr = _handlerAdapter.get();

            CefString urlStr = toNative(_address);

            CefWindowInfo windowInfo;

            HWND hWnd = static_cast<HWND>(Handle.ToPointer());
            RECT rect;
            GetClientRect(hWnd, &rect);
            windowInfo.SetAsChild(hWnd, rect);

            CefBrowser::CreateBrowser(windowInfo, false, static_cast<CefRefPtr<CefHandler>>(ptr), urlStr);
        }
    }

    void CefWebBrowser::OnSizeChanged(EventArgs^ e)
    {
        if (DesignMode == false && IsInitialized)
        {
            HWND hWnd = static_cast<HWND>(Handle.ToPointer());
            RECT rect;
            GetClientRect(hWnd, &rect);
            HDWP hdwp = BeginDeferWindowPos(1);

            HWND browserHwnd = _handlerAdapter->GetBrowserHwnd();
            hdwp = DeferWindowPos(hdwp, browserHwnd, NULL, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
            EndDeferWindowPos(hdwp);
        }
    }

    void CefWebBrowser::OnGotFocus(EventArgs^ e)
    {
        if (IsInitialized && !DesignMode)
        {
            _handlerAdapter->GetCefBrowser()->SetFocus(true);
        }
    }

    void CefWebBrowser::SetTitle(String^ title)
    {
        _title = title;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Title"));
    }

    void CefWebBrowser::SetAddress(String^ address)
    {
        _address = address;
        PropertyChanged(this, gcnew PropertyChangedEventArgs(L"Address"));
    }

    void CefWebBrowser::SetNavState(bool isLoading, bool canGoBack, bool canGoForward)
    {
        if(isLoading != _isLoading) 
        {
            _isLoading = isLoading;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"IsLoading"));
        }

        if(canGoBack != _canGoBack) 
        {
            _canGoBack = canGoBack;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoBack"));
        }

        if(canGoForward != _canGoForward)
        {
            _canGoForward = canGoForward;
            PropertyChanged(this, gcnew PropertyChangedEventArgs(L"CanGoForward"));
        }
    }

    void CefWebBrowser::AddFrame(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->AddCount();
    }

    void CefWebBrowser::FrameLoadComplete(CefRefPtr<CefFrame> frame)
    {
        _loadCompleted->Signal();
    }

    void CefWebBrowser::WaitForLoadCompletion()
    {
        WaitForLoadCompletion(-1);
    }

    void CefWebBrowser::WaitForLoadCompletion(int timeout)
    {
        _loadCompleted->Wait(timeout);
    }

    void CefWebBrowser::SetJsResult(String^ result)
    {
        _jsResult = result;
        _runJsFinished->Set();
    }

    void CefWebBrowser::SetJsError()
    {
        _jsError = true;
        _runJsFinished->Set();
    }

    void CefWebBrowser::RaiseConsoleMessage(String^ message, String^ source, int line)
    {
        ConsoleMessage(this, gcnew ConsoleMessageEventArgs(message, source, line));
    }

    void CefWebBrowser::WaitForInitialized()
    {
        if (IsInitialized) return;

        // TODO: risk of infinite lock
        _browserInitialized->WaitOne();
    }
}
*/