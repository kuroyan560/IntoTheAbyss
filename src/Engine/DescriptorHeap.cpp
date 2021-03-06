#include "DescriptorHeap.h"

DescriptorHeapBase::DescriptorHeapBase(ComPtr<ID3D12Device>Device, const D3D12_DESCRIPTOR_HEAP_TYPE& Type, const int& MaxDescriptorNum)
{
    HRESULT result;

    D3D12_DESCRIPTOR_HEAP_FLAGS flag = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
    {
        flag = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }

    // SRV_CBV_UAV のディスクリプタヒープ
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc{
      Type,
      MaxDescriptorNum,
      flag,
      0
    };
    result = Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&heap));
    headHandleCpu = heap.Get()->GetCPUDescriptorHandleForHeapStart();
    headHandleGpu = heap.Get()->GetGPUDescriptorHandleForHeapStart();
    incrementSize = Device->GetDescriptorHandleIncrementSize(heapDesc.Type);
}

void DescriptorHeap_CBV_SRV_UAV::CreateCBV(const ComPtr<ID3D12Device>& Device, const D3D12_CONSTANT_BUFFER_VIEW_DESC& Desc)
{
    Device->CreateConstantBufferView(&Desc, GetCpuHandleEnd());
    OnCreateView();
}

void DescriptorHeap_CBV_SRV_UAV::CreateSRV(const ComPtr<ID3D12Device>& Device, const ComPtr<ID3D12Resource>& Buff, const D3D12_SHADER_RESOURCE_VIEW_DESC& Desc)
{
    //ヒープにシェーダリソースビュー作成
    Device->CreateShaderResourceView(
        Buff.Get(),	//ビューと関連付けるバッファ
        &Desc,				//テクスチャ設定情報
        GetCpuHandleEnd());
    OnCreateView();
}

void DescriptorHeap_CBV_SRV_UAV::CreateUAV(const ComPtr<ID3D12Device>& Device, const ComPtr<ID3D12Resource>& Buff, const D3D12_UNORDERED_ACCESS_VIEW_DESC& Desc)
{
    //ヒープにアンオーダードアクセスビュー作成
    Device->CreateUnorderedAccessView(
        Buff.Get(), //ビューと関連付けるバッファ
        nullptr,
        &Desc,  //データ設定情報
        GetCpuHandleEnd());
    OnCreateView();
}

void DescriptorHeap_RTV::CreateRTV(const ComPtr<ID3D12Device>& Device, const ComPtr<ID3D12Resource>& Buff)
{
    //ヒープにレンダーターゲットビュー作成
    Device->CreateRenderTargetView(
        Buff.Get(), //ビューと関連付けるバッファ
        nullptr,
        GetCpuHandleEnd());
    OnCreateView();
}

void DescriptorHeap_DSV::CreateDSV(const ComPtr<ID3D12Device>& Device, const ComPtr<ID3D12Resource>& Buff)
{
    //ヒープにデプスステンシルビュー作成
    Device->CreateDepthStencilView(
        Buff.Get(), //ビューと関連付けるバッファ
        nullptr,
        GetCpuHandleEnd());
    OnCreateView();
}