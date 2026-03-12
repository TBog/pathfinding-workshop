#include "pch.h"

#include "Object.h"

#include "..\Render\TexturesManager.h"
#include "..\Render\RenderManager.h"

#include "Utils.h"
#include "Texture.h"
#include "Material.h"
#include "Mesh.h"

#include "..\extern\tiny_obj_loader.h"

//-------------------------------------------------------------------

Object::Object( const WCHAR *_szFileName, float _scale )
    : m_szFileName      ( NULL )
    , m_materialsList   ( 4, 8 )
    , m_meshesList      ( 4, 8 )
    , m_local_boundingSphere_center( 0.f, 0.f, 0.f )
    , m_local_boundingSphere_radius( 0.f )
    , m_local_boundingAABB_min( 0.f, 0.f, 0.f )
    , m_local_boundingAABB_max( 0.f, 0.f, 0.f )
{
    Load( _szFileName, _scale );
}

//-------------------------------------------------------------------

Object::~Object( )
{
    Unload( );
}

//-------------------------------------------------------------------

static void CalcNormal( float N[3], float v0[3], float v1[3], float v2[3] )
{
    float v10[3];
    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    float v20[3];
    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v20[1] * v10[2] - v20[2] * v10[1];
    N[1] = v20[2] * v10[0] - v20[0] * v10[2];
    N[2] = v20[0] * v10[1] - v20[1] * v10[0];

    float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if ( len2 > 0.0f )
    {
        float len = sqrtf( len2 );

        N[0] /= len;
        N[1] /= len;
        N[2] /= len;
    }
}

//-------------------------------------------------------------------

// Compute the tangent for the 3 vertices of a triangle based on the positions and uv coords
static void ComputeTangents( const float pos[3][3], const float uv[3][2], float outTangent[3] )
{
    D3DXVECTOR3 pos1( (float*)&pos[0][0] ), pos2( (float*)&pos[1][0] ), pos3( (float*)&pos[2][0] );
    D3DXVECTOR2 tex1( (float*)&uv[0][0] ), tex2( (float*)&uv[1][0] ), tex3( (float*)&uv[2][0] );
    D3DXVECTOR3 tangent;

    // Determine surface orientation by calculating triangles edges
    D3DXVECTOR3 edge1 = pos2 - pos1;
    D3DXVECTOR3 edge2 = pos3 - pos1;
    D3DXVec3Normalize( &edge1, &edge1 );
    D3DXVec3Normalize( &edge2, &edge2 );

    // Do the same in texture space
    D3DXVECTOR2 texEdge1 = tex2 - tex1;
    D3DXVECTOR2 texEdge2 = tex3 - tex1;
    D3DXVec2Normalize( &texEdge1, &texEdge1 );
    D3DXVec2Normalize( &texEdge2, &texEdge2 );

    // A determinant returns the orientation of the surface
    float det = ( texEdge1.x * texEdge2.y ) - ( texEdge1.y * texEdge2.x );

    // Account for imprecision
    D3DXVECTOR3 bitangenttest;
    if ( fabsf(det) < 1e-6f )
    {
        // Equal to zero (almost) means the surface lies flat on its back
        tangent.x = 1.0f;
        tangent.y = 0.0f;
        tangent.z = 0.0f;
    }
    else {
        det = 1.0f / det;

        tangent.x = (texEdge2.y * edge1.x - texEdge1.y * edge2.x) * det;
        tangent.y = (texEdge2.y * edge1.y - texEdge1.y * edge2.y) * det;
        tangent.z = (texEdge2.y * edge1.z - texEdge1.y * edge2.z) * det;

        D3DXVec3Normalize( &tangent, &tangent );
    }

    outTangent[0] = tangent.x;
    outTangent[1] = tangent.y;
    outTangent[2] = tangent.z;
}

//-------------------------------------------------------------------

// Check if `mesh_t` contains smoothing group id.
static bool hasSmoothingGroup( const tinyobj::shape_t& shape )
{
    for ( size_t i = 0; i < shape.mesh.smoothing_group_ids.size( ); i++ )
    {
        if ( shape.mesh.smoothing_group_ids[i] > 0 )
        {
            return true;
        }
    }

    return false;
}

//-------------------------------------------------------------------

static void computeSmoothingNormals( const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape, std::map<int, D3DXVECTOR3>& smoothVertexNormals )
{
    smoothVertexNormals.clear( );
    std::map<int, D3DXVECTOR3>::iterator iter;

    for ( size_t f = 0; f < shape.mesh.indices.size( ) / 3; f++ )
    {
        // Get the three indexes of the face (all faces are triangular)
        tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
        tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
        tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

        // Get the three vertex indexes and coordinates
        int vi[3];      // indexes
        float v[3][3];  // coordinates

        for ( int k = 0; k < 3; k++ )
        {
            vi[0] = idx0.vertex_index;
            vi[1] = idx1.vertex_index;
            vi[2] = idx2.vertex_index;
            myAssert( vi[0] >= 0, L"computeSmoothingNormals() invalid index!" );
            myAssert( vi[1] >= 0, L"computeSmoothingNormals() invalid index!" );
            myAssert( vi[2] >= 0, L"computeSmoothingNormals() invalid index!" );

            v[0][k] = attrib.vertices[3 * vi[0] + k];
            v[1][k] = attrib.vertices[3 * vi[1] + k];
            v[2][k] = attrib.vertices[3 * vi[2] + k];
        }

        // Compute the normal of the face
        float normal[3];
        CalcNormal( normal, v[0], v[1], v[2] );

        // Add the normal to the three vertexes
        for ( size_t i = 0; i < 3; ++i )
        {
            iter = smoothVertexNormals.find( vi[i] );
            if ( iter != smoothVertexNormals.end( ) )
            {
                // add
                iter->second.x += normal[0];
                iter->second.y += normal[1];
                iter->second.z += normal[2];
            }
            else
            {
                smoothVertexNormals[vi[i]].x = normal[0];
                smoothVertexNormals[vi[i]].y = normal[1];
                smoothVertexNormals[vi[i]].z = normal[2];
            }
        }

    }  // f

    // Normalize the normals, that is, make them unit vectors
    for ( iter = smoothVertexNormals.begin( ); iter != smoothVertexNormals.end( ); iter++ )
    {
        D3DXVec3Normalize( &iter->second, &iter->second );
    }

}  // computeSmoothingNormals

//-------------------------------------------------------------------

bool Object::Load( const WCHAR *_szFileName, float _scale )
{
    Unload( );

    m_szFileName = _wcsdup( _szFileName );

    // convert file name to char*
    char szFileNameChar[256];
    ConvertWdieStringToString( m_szFileName, szFileNameChar );

    // extract dir to use for mtl path
    char szMtlFolderNameChar[256];
    strcpy_s( szMtlFolderNameChar, 256, szFileNameChar );
    char *t1 = strrchr( szMtlFolderNameChar, '/' );
    char *t2 = strrchr( szMtlFolderNameChar, '\\' );
    char *t = ( t1 > t2 ) ? t1 : t2;
    if ( t != NULL )
    {
        t[0] = 0;
    }

    // load the data from the obj and mtl file
    std::string err;
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    bool wasLoaded = tinyobj::LoadObj( &attrib, &shapes, &materials, &err, szFileNameChar, szMtlFolderNameChar, true );
    if ( !wasLoaded )
    {
        WCHAR szErrorMsg[256];
        swprintf_s( szErrorMsg, L"Object::Load(%ws) failed! - %hs", m_szFileName, err.c_str() );
        myAssert( false, szErrorMsg );

        return false;
    }

    // Append `default` material
    materials.push_back( tinyobj::material_t( ) );

    // Load textures
    {
        char szTmpFullFileName[512];

        for ( size_t m = 0; m < materials.size( ); m++ )
        {
            tinyobj::material_t* mp = &materials[m];

            Material *material = new Material();
            m_materialsList.Add( material );
            material->AddRef( );

            if ( mp->diffuse_texname.length( ) > 0 )
            {
                sprintf_s( szTmpFullFileName, 512, "%s/%s", szMtlFolderNameChar, mp->diffuse_texname.c_str() );
                
                Texture *diffuseTexture = g_texturesManager->Load( szTmpFullFileName );
                material->SetTexture( eObjectTextureSlotType_Diffuse, diffuseTexture );
            }

            if ( mp->normal_texname.length() > 0 )
            {
                sprintf_s(szTmpFullFileName, 512, "%s/%s", szMtlFolderNameChar, mp->normal_texname.c_str());

                Texture *normalTexture = g_texturesManager->Load( szTmpFullFileName );
                material->SetTexture( eObjectTextureSlotType_Normal, normalTexture );
            }
            else if ( mp->bump_texname.length() > 0 )
            {
                sprintf_s(szTmpFullFileName, 512, "%s/%s", szMtlFolderNameChar, mp->bump_texname.c_str());

                Texture *normalTexture = g_texturesManager->Load( szTmpFullFileName );
                material->SetTexture( eObjectTextureSlotType_Normal, normalTexture );
            }

            if (mp->roughness_texname.length() > 0)
            {
                sprintf_s(szTmpFullFileName, 512, "%s/%s", szMtlFolderNameChar, mp->roughness_texname.c_str());

                Texture *roughnessTexture = g_texturesManager->Load( szTmpFullFileName );
                material->SetTexture( eObjectTextureSlotType_Roughness, roughnessTexture );
            }

            if (mp->specular_texname.length() > 0)
            {
                sprintf_s(szTmpFullFileName, 512, "%s/%s", szMtlFolderNameChar, mp->specular_texname.c_str());

                Texture *specularTexture = g_texturesManager->Load( szTmpFullFileName );
                material->SetTexture( eObjectTextureSlotType_Specular, specularTexture );
            }
        }
    }

    D3DXVECTOR3 bmin( FLT_MAX, FLT_MAX, FLT_MAX );
    D3DXVECTOR3 bmax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    {
        for ( size_t s = 0; s < shapes.size( ); s++ )
        {
            std::vector<float> buffer;  // pos(3float), normal(3float), tangent(3float), color(3float), texcoord(2float)

            // Check for smoothing group and compute smoothing normals
            std::map<int, D3DXVECTOR3> smoothVertexNormals;
            if ( hasSmoothingGroup( shapes[s] ) )
            {
                computeSmoothingNormals( attrib, shapes[s], smoothVertexNormals );
            }

            for ( size_t f = 0; f < shapes[s].mesh.indices.size( ) / 3; f++ )
            {
                tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];

                int current_material_id = shapes[s].mesh.material_ids[f];

                if ( ( current_material_id < 0 ) || ( current_material_id >= static_cast<int>( materials.size( ) ) ) )
                {
                    // Invalid material ID. Use default material.
                    current_material_id = materials.size( ) - 1;  // Default material is added to the last item in `materials`.
                }

                float tc[3][2];
                if ( attrib.texcoords.size( ) > 0 )
                {
                    if ( ( idx0.texcoord_index < 0 ) || ( idx1.texcoord_index < 0 ) || ( idx2.texcoord_index < 0 ) )
                    {
                        // face does not contain valid uv index.
                        tc[0][0] = 0.0f;
                        tc[0][1] = 0.0f;
                        tc[1][0] = 0.0f;
                        tc[1][1] = 0.0f;
                        tc[2][0] = 0.0f;
                        tc[2][1] = 0.0f;
                    }
                    else
                    {
                        myAssert( attrib.texcoords.size( ) > size_t( 2 * idx0.texcoord_index + 1 ), L"Object::Load() - texcoord_index outside of range!" );
                        myAssert( attrib.texcoords.size( ) > size_t( 2 * idx1.texcoord_index + 1 ), L"Object::Load() - texcoord_index outside of range!" );
                        myAssert( attrib.texcoords.size( ) > size_t( 2 * idx2.texcoord_index + 1 ), L"Object::Load() - texcoord_index outside of range!" );

                        // Flip Y coord.
                        tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                        tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                        tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                        tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                        tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                        tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                    }
                }
                else
                {
                    tc[0][0] = 0.0f;
                    tc[0][1] = 0.0f;
                    tc[1][0] = 0.0f;
                    tc[1][1] = 0.0f;
                    tc[2][0] = 0.0f;
                    tc[2][1] = 0.0f;
                }

                float v[3][3];
                float color[3][3];   // using the same indices from position for vertex color
                for ( int k = 0; k < 3; k++ )
                {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    myAssert( f0 >= 0, L"Object::Load() - vertex_index outside of range!" );
                    myAssert( f1 >= 0, L"Object::Load() - vertex_index outside of range!" );
                    myAssert( f2 >= 0, L"Object::Load() - vertex_index outside of range!" );

                    v[0][k] = attrib.vertices[3 * f0 + k] * _scale;
                    v[1][k] = attrib.vertices[3 * f1 + k] * _scale;
                    v[2][k] = attrib.vertices[3 * f2 + k] * _scale;
                    bmin[k] = __min( v[0][k], bmin[k] );
                    bmin[k] = __min( v[1][k], bmin[k] );
                    bmin[k] = __min( v[2][k], bmin[k] );
                    bmax[k] = __max( v[0][k], bmax[k] );
                    bmax[k] = __max( v[1][k], bmax[k] );
                    bmax[k] = __max( v[2][k], bmax[k] );

                    color[0][k] = attrib.colors[3 * f0 + k];
                    color[1][k] = attrib.colors[3 * f1 + k];
                    color[2][k] = attrib.colors[3 * f2 + k];
                }

                float n[3][3];
                {
                    bool invalid_normal_index = false;
                    if ( attrib.normals.size( ) > 0 )
                    {
                        int nf0 = idx0.normal_index;
                        int nf1 = idx1.normal_index;
                        int nf2 = idx2.normal_index;

                        if ( ( nf0 < 0 ) || ( nf1 < 0 ) || ( nf2 < 0 ) )
                        {
                            // normal index is missing from this face.
                            invalid_normal_index = true;
                        }
                        else
                        {
                            for ( int k = 0; k < 3; k++ )
                            {
                                myAssert( size_t( 3 * nf0 + k ) < attrib.normals.size( ), L"Object::Load() - normal_index outside of range!" );
                                myAssert( size_t( 3 * nf1 + k ) < attrib.normals.size( ), L"Object::Load() - normal_index outside of range!" );
                                myAssert( size_t( 3 * nf2 + k ) < attrib.normals.size( ), L"Object::Load() - normal_index outside of range!" );
                                n[0][k] = attrib.normals[3 * nf0 + k];
                                n[1][k] = attrib.normals[3 * nf1 + k];
                                n[2][k] = attrib.normals[3 * nf2 + k];
                            }
                        }
                    }
                    else
                    {
                        invalid_normal_index = true;
                    }

                    if ( invalid_normal_index && !smoothVertexNormals.empty( ) )
                    {
                        // Use smoothing normals
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;

                        if ( f0 >= 0 && f1 >= 0 && f2 >= 0 )
                        {
                            n[0][0] = smoothVertexNormals[f0].x;
                            n[0][1] = smoothVertexNormals[f0].y;
                            n[0][2] = smoothVertexNormals[f0].z;

                            n[1][0] = smoothVertexNormals[f1].x;
                            n[1][1] = smoothVertexNormals[f1].y;
                            n[1][2] = smoothVertexNormals[f1].z;

                            n[2][0] = smoothVertexNormals[f2].x;
                            n[2][1] = smoothVertexNormals[f2].y;
                            n[2][2] = smoothVertexNormals[f2].z;

                            invalid_normal_index = false;
                        }
                    }

                    if ( invalid_normal_index )
                    {
                        // compute geometric normal
                        CalcNormal( n[0], v[0], v[1], v[2] );
                        n[1][0] = n[0][0];
                        n[1][1] = n[0][1];
                        n[1][2] = n[0][2];
                        n[2][0] = n[0][0];
                        n[2][1] = n[0][1];
                        n[2][2] = n[0][2];
                    }
                }

                float t[3][3];
                for (int k = 0; k < 3; k++)
                {
                    ComputeTangents( v, tc, t[0] );
                    ComputeTangents( v, tc, t[1] );
                    ComputeTangents( v, tc, t[2] );
                }

                for ( int k = 0; k < 3; k++ )
                {
                    buffer.push_back( v[k][0] );
                    buffer.push_back( v[k][1] );
                    buffer.push_back( v[k][2] );

                    buffer.push_back( n[k][0] );
                    buffer.push_back( n[k][1] );
                    buffer.push_back( n[k][2] );

                    buffer.push_back( t[k][0] );
                    buffer.push_back( t[k][1] );
                    buffer.push_back( t[k][2] );

                    buffer.push_back( color[k][0] );
                    buffer.push_back( color[k][1] );
                    buffer.push_back( color[k][2] );

                    buffer.push_back( tc[k][0] );
                    buffer.push_back( tc[k][1] );
                }
            }

            Mesh *mesh = new Mesh( (BYTE*)&buffer.at(0), buffer.size( ) * sizeof( float ), (3 + 3 + 3 + 3 + 2) * sizeof( float ), NULL, 0, g_renderManager->GetDefaultObjectVertexLayout() );  // 3:vtx, 3:normal, 3:tangent, 3:col, 2:texcoord
            m_meshesList.Add( mesh );

            int material_id = materials.size( ) - 1;    // = ID for default material.
            if ( shapes[s].mesh.material_ids.size( ) > 0 )
            {
                if ( (shapes[s].mesh.material_ids[0] >= 0) && (shapes[s].mesh.material_ids[0] < (int)materials.size( )) )
                    material_id = shapes[s].mesh.material_ids[0];  // use the material ID of the first face.
            }

            mesh->SetMaterial( m_materialsList[material_id] );
        }
    }

    _UpdateLocalBoundingAABB( bmin, bmax );

    return true;
}

//-------------------------------------------------------------------

void Object::Unload( )
{
    for ( int i = 0; i < m_meshesList.GetSize(); i++ )
    {
        SAFE_DELETE( m_meshesList[i] );
    }
    m_meshesList.Clear( );

    for ( int i = 0; i < m_materialsList.GetSize(); i++ )
    {
        SAFE_RELEASE( m_materialsList[i] );
    }
    m_materialsList.Clear( );

    SAFE_FREE( m_szFileName );
}

//-------------------------------------------------------------------

void Object::_UpdateLocalBoundingAABB( const D3DXVECTOR3 &_AABB_min, const D3DXVECTOR3 &_AABB_max )
{
    m_local_boundingAABB_min = _AABB_min;
    m_local_boundingAABB_max = _AABB_max;

    m_local_boundingSphere_center = ( m_local_boundingAABB_min + m_local_boundingAABB_max ) * 0.5f;
    D3DXVECTOR3 diag = m_local_boundingAABB_max - m_local_boundingAABB_min;
    m_local_boundingSphere_radius = D3DXVec3Length( &diag ) * 0.5f;
}

//-------------------------------------------------------------------
