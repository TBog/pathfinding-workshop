#include "pch.h"

#include "Mesh.h"

#include "Utils.h"
#include "Material.h"

#include "..\Render\RenderManager.h"


//-------------------------------------------------------------------

Mesh::Mesh( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes, ID3D11InputLayout *_vertexLayout )
    : m_VB        ( NULL )
    , m_VBStride  ( 0 )
    , m_VBVertexCount( 0 )
    , m_IB        ( NULL )
    , m_IBIndexCount( 0 )
    , m_vertexLayout( _vertexLayout )
    , m_material  ( NULL )
{
    m_vertexLayout->AddRef();

    _CreateBuffers( _vertexData, _vertexDataSizeInBytes, _vertexDataStride, _indexData, _indexDataSizeInBytes );
}

//-------------------------------------------------------------------

Mesh::~Mesh( )
{
    _DestroyBuffers( );

    SAFE_RELEASE( m_material );
}

//-------------------------------------------------------------------

void Mesh::Draw( )
{
    _DrawMesh( );
}

//-------------------------------------------------------------------

void Mesh::SetMaterial( Material *_material )
{
    if ( _material )
    {
        _material->AddRef( );
    }
    
    SAFE_RELEASE( m_material );
    
    m_material = _material;
}

//-------------------------------------------------------------------

void Mesh::_CreateBuffers( BYTE *_vertexData, int _vertexDataSizeInBytes, int _vertexDataStride, BYTE *_indexData, int _indexDataSizeInBytes )
{
    {
        D3D11_BUFFER_DESC DescVB;
        DescVB.Usage            = D3D11_USAGE_IMMUTABLE;
        DescVB.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
        DescVB.CPUAccessFlags   = 0;
        DescVB.MiscFlags        = 0;

        DescVB.ByteWidth        = _vertexDataSizeInBytes;

        D3D11_SUBRESOURCE_DATA DataVB;
        DataVB.pSysMem          = _vertexData;
        DataVB.SysMemPitch      = _vertexDataSizeInBytes; // not used, can be 0
        DataVB.SysMemSlicePitch = 0;

        g_renderManager->GetDevice( )->CreateBuffer( &DescVB, &DataVB, &m_VB );
    }

    m_VBStride = _vertexDataStride;
    m_VBVertexCount = _vertexDataSizeInBytes / _vertexDataStride;

    if ( _indexDataSizeInBytes > 0 )
    {
        D3D11_BUFFER_DESC DescIB;
        DescIB.Usage            = D3D11_USAGE_IMMUTABLE;
        DescIB.BindFlags        = D3D11_BIND_INDEX_BUFFER;
        DescIB.CPUAccessFlags   = 0;
        DescIB.MiscFlags        = 0;

        DescIB.ByteWidth        = _indexDataSizeInBytes;

        D3D11_SUBRESOURCE_DATA DataIB;
        DataIB.pSysMem          = _indexData;
        DataIB.SysMemPitch      = _indexDataSizeInBytes; // not used, can be 0
        DataIB.SysMemSlicePitch = 0;

        g_renderManager->GetDevice( )->CreateBuffer( &DescIB, &DataIB, &m_IB );
    }

    m_IBIndexCount = _indexDataSizeInBytes / 2; // assume index buffer is on 16 bits
}

//-------------------------------------------------------------------

void Mesh::_DestroyBuffers( )
{
    SAFE_RELEASE( m_VB );
    SAFE_RELEASE( m_IB );
    SAFE_RELEASE( m_vertexLayout );
}

//-------------------------------------------------------------------

void Mesh::_DrawMesh( )
{
    ID3D11DeviceContext* d3dImmediateContext = g_renderManager->GetDeviceContext( );

    //IA setup
    d3dImmediateContext->IASetInputLayout( m_vertexLayout );

    UINT Strides[1];
    UINT Offsets[1];
    ID3D11Buffer* pVB[1];
    pVB[0] = m_VB;
    Strides[0] = ( UINT )m_VBStride;
    Offsets[0] = 0;
    d3dImmediateContext->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );

    d3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    if ( m_IB )
    {
        d3dImmediateContext->IASetIndexBuffer( m_IB, DXGI_FORMAT_R16_UINT, 0 );

        d3dImmediateContext->DrawIndexed( (UINT)m_IBIndexCount, 0, 0 );
    }
    else
    {
        d3dImmediateContext->Draw( (UINT)m_VBVertexCount, 0 );
    }
}

//-------------------------------------------------------------------
