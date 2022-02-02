// A Direct3D example in straightforward C++.
//


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <stdlib.h>
#include <stdint.h>
#include <wchar.h>
#include <assert.h>

#pragma comment (lib, "user32.lib")
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3dcompiler.lib")



#define ASSERT(expr)    assert(expr)
#define ASSERT_HR(hr)   ASSERT(SUCCEEDED(hr))



// Window Properties

static wchar_t const    *window_title       = L"Hello Triangle in D3D12";

// These are updated every time the window has been resized.
static int              window_width        = 720;
static int              window_height       = 480;
static float            window_aspect       = (float)window_height /
                                                (float)window_width;
static bool             window_resized      = false;



// What We Seek to Draw

typedef struct Vertex {
    float pos[2];
    float uv[2];
    float color[4];
} Vertex;

static Vertex triangle[] = {
    {{-0.0f,  0.7f}, {1.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{ 0.7f, -0.7f}, {3.0f, 3.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{-0.7f, -0.7f}, {0.0f, 3.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
};

// Checkerboard texture with 32% transparency.
static uint32_t checkers[] = {
    0x20000000, 0x20ffffff,
    0x20ffffff, 0x20000000,
};
static size_t checkers_width = 2;
static size_t checkers_height = 2;

static float background[] = {0.117f, 0.117f, 0.120f, 1.0f};



// The Window Procedure

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wp, LPARAM lp)
{
    LRESULT lr = 0;

    switch (message) {
    case WM_SIZE:
        window_width = LOWORD(lp);
        window_height = HIWORD(lp);
        window_aspect = (float)window_height / (float)window_width;
        window_resized = true;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        lr = DefWindowProcW(window, message, wp, lp);
        break;
    }
    return lr;
}



// main()
// Everything takes places inside here.

int WINAPI WinMain(HINSTANCE instance, HINSTANCE instance_p, LPSTR cmd_line, int cmd_show)
{
    // Create a window.

    HWND window;
    {
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = window_proc;
        wc.hInstance = instance;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = window_title;

        if (!RegisterClassExW(&wc))
            ASSERT(0);

        DWORD style = WS_OVERLAPPEDWINDOW;
        DWORD style_ex = WS_EX_APPWINDOW | WS_EX_NOREDIRECTIONBITMAP;

        window = CreateWindowExW(
            style_ex, wc.lpszClassName, window_title, style,
            CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,
            NULL, NULL, wc.hInstance, NULL);
        ASSERT(window);

        // Showing the window here is a bad idea in a real program.
        // It is better to do it after D3D12 has been fully initialized.
        ShowWindow(window, cmd_show);
    }



    // Create a device that represents the default adapter.
    // Create a command queue for this device.

    ID3D12Device *device;
    ID3D12CommandQueue *cmd_queue;
    {
        HRESULT hr;

        hr = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
        ASSERT_HR(hr);

        D3D12_COMMAND_QUEUE_DESC _cmd_queue = {0};
        _cmd_queue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = device->CreateCommandQueue(&_cmd_queue, IID_PPV_ARGS(&cmd_queue));
        ASSERT_HR(hr);
    }



    // Create the swap chain.

    IDXGISwapChain3 *swapchain;
    UINT buffer_count = 2;
    {
        IDXGIFactory2 *dxgi;
        HRESULT hr;

        hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgi));
        ASSERT_HR(hr);

        DXGI_SWAP_CHAIN_DESC1 _swapchain = {0};
        _swapchain.Width = 0;
        _swapchain.Height = 0;
        _swapchain.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        _swapchain.SampleDesc = {1, 0};
        _swapchain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        _swapchain.BufferCount = buffer_count;
        _swapchain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

        hr = dxgi->CreateSwapChainForHwnd(
            (IUnknown *)cmd_queue, window, &_swapchain,
            NULL, NULL, (IDXGISwapChain1 **)&swapchain);
        ASSERT_HR(hr);

        dxgi->Release();
    }



    // Create the root signature.

    ID3D12RootSignature *signature;
    UINT table_slot;
    UINT consts_slot;
    {
        HRESULT hr;


        D3D12_DESCRIPTOR_RANGE range = {0};
        range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        range.NumDescriptors = 1;
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        D3D12_ROOT_PARAMETER table = {0};
        table.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        table.DescriptorTable.NumDescriptorRanges = 1;
        table.DescriptorTable.pDescriptorRanges = &range;
        table.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_PARAMETER consts = {0};
        consts.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        consts.Constants.Num32BitValues = 4;
        consts.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        D3D12_ROOT_PARAMETER params[] = {table, consts};
        table_slot = 0;
        consts_slot = 1;


        D3D12_STATIC_SAMPLER_DESC sampler = {0};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_STATIC_SAMPLER_DESC samplers[] = {sampler};


        D3D12_ROOT_SIGNATURE_DESC _signature = {0};
        _signature.NumParameters = _countof(params);
        _signature.pParameters = params;
        _signature.NumStaticSamplers = _countof(samplers);
        _signature.pStaticSamplers = samplers;
        _signature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ID3DBlob *blob;
        hr = D3D12SerializeRootSignature(
            &_signature, D3D_ROOT_SIGNATURE_VERSION_1, &blob, NULL);
        ASSERT_HR(hr);

        hr = device->CreateRootSignature(
            0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&signature));
        ASSERT_HR(hr);

        blob->Release();
    }



    // Create the Pipeline State Object.

    ID3D12PipelineState *pipeline;
    {
        ID3DBlob *vs, *ps;
        ID3DBlob *error;
        HRESULT hr;


        hr = D3DCompileFromFile(L"shaders.hlsl", NULL, NULL, "vs", "vs_5_0", 0, 0, &vs, &error);
        if (FAILED(hr)) {
            const char *message = (const char *)error->GetBufferPointer();
            OutputDebugStringA(message);
            ASSERT(0);
        }

        hr = D3DCompileFromFile(L"shaders.hlsl", NULL, NULL, "ps", "ps_5_0", 0, 0, &ps, &error);
        if (FAILED(hr)) {
            const char *message = (const char *)error->GetBufferPointer();
            OutputDebugStringA(message);
            ASSERT(0);
        }

        D3D12_INPUT_ELEMENT_DESC input_elements[] = {
            {"POSITION",    0, DXGI_FORMAT_R32G32_FLOAT,        0, offsetof(Vertex, pos),   D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,        0, offsetof(Vertex, uv),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"COLOR",       0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, offsetof(Vertex, color), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        // Enable alpha blending.
        D3D12_BLEND_DESC blend = {0};
        blend.RenderTarget[0].BlendEnable = TRUE;
        blend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_RASTERIZER_DESC rasterizer = {0};
        rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizer.CullMode = D3D12_CULL_MODE_NONE;

        D3D12_DEPTH_STENCIL_DESC depth_stencil = {0};
        depth_stencil.DepthEnable = FALSE;
        depth_stencil.StencilEnable = FALSE;


        D3D12_GRAPHICS_PIPELINE_STATE_DESC _pipeline = {0};
        _pipeline.pRootSignature = signature;
        _pipeline.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
        _pipeline.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
        _pipeline.BlendState = blend;
        _pipeline.SampleMask = UINT_MAX;
        _pipeline.RasterizerState = rasterizer;
        _pipeline.DepthStencilState = depth_stencil;
        _pipeline.InputLayout = {input_elements, _countof(input_elements)};
        _pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        _pipeline.NumRenderTargets = 1;
        _pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        _pipeline.SampleDesc = {1, 0};

        hr = device->CreateGraphicsPipelineState(&_pipeline, IID_PPV_ARGS(&pipeline));
        ASSERT_HR(hr);


        vs->Release();
        ps->Release();
    }



    // Create a command allocator and a command list.

    ID3D12CommandAllocator *cmd_alloc;
    ID3D12GraphicsCommandList *cmd_list;
    {
        HRESULT hr;

        hr = device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_alloc));
        ASSERT_HR(hr);

        hr = device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmd_alloc, pipeline, IID_PPV_ARGS(&cmd_list));
        ASSERT_HR(hr);

        hr = cmd_list->Close();
        ASSERT_HR(hr);
    }



    // Create an upload buffer.

    ID3D12Resource *upload_buffer;
    SIZE_T triangle_offset;
    SIZE_T checkers_offset;
    {
        HRESULT hr;


        D3D12_HEAP_PROPERTIES heap = {0};
        heap.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC buffer = {0};
        buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        buffer.Alignment = 0;
        buffer.Width = 64 * 1024; // Buffer alignment.
        buffer.Height = 1;
        buffer.DepthOrArraySize = 1;
        buffer.MipLevels = 1;
        buffer.Format = DXGI_FORMAT_UNKNOWN;
        buffer.SampleDesc = {1, 0};
        buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        buffer.Flags = D3D12_RESOURCE_FLAG_NONE;

        hr = device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE,
            &buffer, D3D12_RESOURCE_STATE_GENERIC_READ,
            NULL, IID_PPV_ARGS(&upload_buffer));
        ASSERT_HR(hr);


        // Upload the data.

        uint8_t *p;
        hr = upload_buffer->Map(0, NULL, (void **)&p);
        ASSERT_HR(hr);
        {
            // Vertex data.

            triangle_offset = 0;

            memcpy(p, triangle, sizeof(triangle));


            // Texture data.

            checkers_offset = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;

            p += checkers_offset;
            size_t stride = checkers_width * sizeof(*checkers);

            for (size_t i = 0; i < sizeof(checkers); i += stride) {
                memcpy(p, (uint8_t *)checkers + i, stride);
                p += D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
            }
        }
        upload_buffer->Unmap(0, NULL);
    }



    // Create a vertex buffer.

    ID3D12Resource *vertex_buffer;
    D3D12_VERTEX_BUFFER_VIEW vbv;
    {
        HRESULT hr;


        D3D12_HEAP_PROPERTIES heap = {0};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC buffer = {0};
        buffer.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        buffer.Alignment = 0;
        buffer.Width = sizeof(triangle);
        buffer.Height = 1;
        buffer.DepthOrArraySize = 1;
        buffer.MipLevels = 1;
        buffer.Format = DXGI_FORMAT_UNKNOWN;
        buffer.SampleDesc = {1, 0};
        buffer.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        buffer.Flags = D3D12_RESOURCE_FLAG_NONE;

        hr = device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE,
            &buffer, D3D12_RESOURCE_STATE_COPY_DEST,
            NULL, IID_PPV_ARGS(&vertex_buffer));
        ASSERT_HR(hr);


        vbv.BufferLocation = vertex_buffer->GetGPUVirtualAddress();
        vbv.StrideInBytes = sizeof(Vertex);
        vbv.SizeInBytes = sizeof(triangle);
    }



    // Create a texture resource.

    ID3D12Resource *checkers_texture;
    ID3D12DescriptorHeap *srv_heap;
    {
        HRESULT hr;


        D3D12_HEAP_PROPERTIES heap = {0};
        heap.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC texture = {0};
        texture.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texture.Alignment = 0;
        texture.Width = (UINT)checkers_width;
        texture.Height = (UINT)checkers_height;
        texture.DepthOrArraySize = 1;
        texture.MipLevels = 1;
        texture.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texture.SampleDesc = {1, 0};
        texture.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texture.Flags = D3D12_RESOURCE_FLAG_NONE;

        hr = device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE,
            &texture, D3D12_RESOURCE_STATE_COPY_DEST,
            NULL, IID_PPV_ARGS(&checkers_texture));
        ASSERT_HR(hr);


        D3D12_DESCRIPTOR_HEAP_DESC _srv_heap = {0};
        _srv_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        _srv_heap.NumDescriptors = 1;
        _srv_heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        hr = device->CreateDescriptorHeap(&_srv_heap, IID_PPV_ARGS(&srv_heap));
        ASSERT_HR(hr);

        device->CreateShaderResourceView(
            checkers_texture, NULL, srv_heap->GetCPUDescriptorHandleForHeapStart());
    }



    // Create a fence.

    ID3D12Fence *fence;
    UINT64 fence_value;
    HANDLE fence_event;
    {
        HRESULT hr;
        hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        ASSERT_HR(hr);

        fence_value = 0;

        fence_event = CreateEventW(NULL, FALSE, FALSE, NULL);
        ASSERT(fence_event);
    }



    // The program loop.


    ID3D12Resource *render_targets[2]; // buffer_count
    ID3D12DescriptorHeap *rtv_heap = NULL;
    D3D12_CPU_DESCRIPTOR_HANDLE rtv_base;
    UINT rtv_stride;

    // To create render targets the first time the program runs.
    window_resized = true;


    LARGE_INTEGER tick_0, freq;
    QueryPerformanceCounter(&tick_0);
    QueryPerformanceFrequency(&freq);

    LARGE_INTEGER tick_p, tick_n;
    tick_p.QuadPart = tick_0.QuadPart;
    tick_n.QuadPart = tick_0.QuadPart + freq.QuadPart;

    double uptime = 0.0;
    int frame_count = 0;


    bool first_time = true;



    while (1) {
        MSG msg;
        if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }



        // Create the render targets.

        if (window_resized) {
            window_resized = false;


            if (rtv_heap) {
                cmd_list->ClearState(NULL);
                for (UINT i = 0; i < buffer_count; i++)
                    render_targets[i]->Release();
                rtv_heap->Release();
            }


            HRESULT hr;


            hr = swapchain->ResizeBuffers(
                buffer_count, window_width, window_height, DXGI_FORMAT_UNKNOWN, 0);
            ASSERT_HR(hr);


            D3D12_DESCRIPTOR_HEAP_DESC _rtv_heap = {0};
            _rtv_heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            _rtv_heap.NumDescriptors = buffer_count;

            hr = device->CreateDescriptorHeap(&_rtv_heap, IID_PPV_ARGS(&rtv_heap));
            ASSERT_HR(hr);

            rtv_base = rtv_heap->GetCPUDescriptorHandleForHeapStart();
            rtv_stride = device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_base;

            for (UINT i = 0; i < buffer_count; i++) {
                hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&render_targets[i]));
                ASSERT_HR(hr);

                device->CreateRenderTargetView(render_targets[i], NULL, rtv_handle);
                rtv_handle.ptr += rtv_stride;
            }
        }



        // Fill the command list.
        {
            HRESULT hr;


            hr = cmd_alloc->Reset();
            ASSERT_HR(hr);

            hr = cmd_list->Reset(cmd_alloc, pipeline);
            ASSERT_HR(hr);


            // Initialization that invoke the command list.

            if (first_time) {
                first_time = false;


                // Transfer vertex data in the upload buffer to the vertex buffer.

                cmd_list->CopyBufferRegion(
                    vertex_buffer, 0, upload_buffer, triangle_offset, sizeof(triangle));


                // Transfer texture data in upload buffer to the texture resource.

                D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {0};

                device->GetCopyableFootprints(
                    &checkers_texture->GetDesc(), 0, 1, checkers_offset,
                    &footprint, NULL, NULL, NULL);

                D3D12_TEXTURE_COPY_LOCATION src = {0};
                src.pResource = upload_buffer;
                src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                src.PlacedFootprint = footprint;

                D3D12_TEXTURE_COPY_LOCATION dst = {0};
                dst.pResource = checkers_texture;
                dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dst.SubresourceIndex = 0;

                cmd_list->CopyTextureRegion(&dst, 0, 0, 0, &src, NULL);


                D3D12_RESOURCE_BARRIER vb = {0};
                vb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                vb.Transition.pResource = vertex_buffer;
                vb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                vb.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                vb.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

                D3D12_RESOURCE_BARRIER tex = {0};
                tex.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                tex.Transition.pResource = checkers_texture;
                tex.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                tex.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                tex.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

                D3D12_RESOURCE_BARRIER bars[] = {vb, tex};

                cmd_list->ResourceBarrier(_countof(bars), bars);
            }


            cmd_list->SetGraphicsRootSignature(signature);

            cmd_list->SetDescriptorHeaps(1, &srv_heap);
            cmd_list->SetGraphicsRootDescriptorTable(
                table_slot, srv_heap->GetGPUDescriptorHandleForHeapStart());

            float consts[] = {
                (float)window_width, (float)window_height, window_aspect, (float)uptime
            };
            cmd_list->SetGraphicsRoot32BitConstants(consts_slot, _countof(consts), consts, 0);


            D3D12_VIEWPORT viewport = {0};
            viewport.Width = (float)window_width;
            viewport.Height = (float)window_height;

            D3D12_RECT scissor = {0};
            scissor.right = (ULONG)window_width;
            scissor.bottom = (ULONG)window_height;

            cmd_list->RSSetViewports(1, &viewport);
            cmd_list->RSSetScissorRects(1, &scissor);


            UINT render_target_index = swapchain->GetCurrentBackBufferIndex();


            D3D12_RESOURCE_BARRIER rt = {0};
            rt.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            rt.Transition.pResource = render_targets[render_target_index];
            rt.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            rt.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            rt.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

            cmd_list->ResourceBarrier(1, &rt);


            D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle = rtv_base;
            rtv_handle.ptr += rtv_stride * render_target_index;
            cmd_list->OMSetRenderTargets(1, &rtv_handle, FALSE, NULL);

            cmd_list->ClearRenderTargetView(rtv_handle, background, 0, NULL);
            cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            cmd_list->IASetVertexBuffers(0, 1, &vbv);
            cmd_list->DrawInstanced(_countof(triangle), 1, 0, 0);


            rt.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            rt.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;

            cmd_list->ResourceBarrier(1, &rt);


            hr = cmd_list->Close();
            ASSERT_HR(hr);
        }



        // Render.
        {
            HRESULT hr;

            cmd_queue->ExecuteCommandLists(1, (ID3D12CommandList **)&cmd_list);

            bool vsync = true;
            hr = swapchain->Present((vsync) ? 1 : 0, 0);
            ASSERT_HR(hr);
        }



        // Wait for the GPU to finish.
        {
            HRESULT hr;

            hr = cmd_queue->Signal(fence, fence_value);
            ASSERT_HR(hr);

            if (fence->GetCompletedValue() < fence_value) {
                hr = fence->SetEventOnCompletion(fence_value, fence_event);
                ASSERT_HR(hr);

                WaitForSingleObject(fence_event, INFINITE);
            }
            fence_value++;
        }



        // Compute various time measurements.
        // Compute the frame rate of the program.
        {
            LARGE_INTEGER tick;
            QueryPerformanceCounter(&tick);

            frame_count++;

            uptime = (double)(tick.QuadPart - tick_0.QuadPart) / (double)freq.QuadPart;

            if (tick.QuadPart >= tick_n.QuadPart) {
                double FPS = (double)frame_count *
                    ((double)freq.QuadPart / (double)(tick.QuadPart - tick_p.QuadPart));

                wchar_t stats[1024];
                swprintf_s(stats, 1024, L"%s [Uptime: %.0fs, FPS: %.1f]",
                           window_title, uptime, FPS);
                SetWindowTextW(window, stats);

                tick_p.QuadPart = tick.QuadPart;
                tick_n.QuadPart = tick.QuadPart + freq.QuadPart;

                frame_count = 0;
            }
        }
    }



    // Clean up.

    for (UINT i = 0; i < buffer_count; i++)
        render_targets[i]->Release();
    rtv_heap->Release();

    CloseHandle(fence_event);
    fence->Release();

    srv_heap->Release();
    checkers_texture->Release();
    vertex_buffer->Release();
    upload_buffer->Release();

    cmd_list->Release();
    cmd_alloc->Release();
    pipeline->Release();
    signature->Release();
    swapchain->Release();
    cmd_queue->Release();
    device->Release();



    // Indicate that the program terminated successfully.
    const wchar_t *success = L"You have succeeded in the game's industry.";
    OutputDebugStringW(success);



    return 0;
}
