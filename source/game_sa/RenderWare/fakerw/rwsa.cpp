#include "StdInc.h"

#ifdef LIBRW
#include "rwsa.h"

RsGlobalType RsGlobal;
bool         RwInitialized;
RwRGBAReal   AmbientSaturated = { 1.0f, 1.0f, 1.0f, 1.0f };

IDirect3DDevice9* GetD3D9Device() {
    return reinterpret_cast<IDirect3DDevice9*>(rw::d3d::d3ddevice);
}

void _rpMaterialSetDefaultSurfaceProperties(RwSurfaceProperties* surfProps) {
    surfProps->ambient  = 1.0f;
    surfProps->specular = 1.0f;
    surfProps->diffuse  = 1.0f;
}

// ---------------------------------------------------------------------------
// UV animation dictionaries
// ---------------------------------------------------------------------------
RtDictSchema RpUVAnimDictSchema;

RtDict* RtDictSchemaStreamReadDict(RtDictSchema*, RwStream* stream) {
    return rw::UVAnimDictionary::streamRead(stream);
}

RtDictSchema* RtDictSchemaSetCurrentDict(RtDictSchema* schema, RtDict* dict) {
    rw::currentUVAnimDictionary = dict;
    return schema;
}

RtDict* RtDictSchemaGetCurrentDict(RtDictSchema*) {
    return rw::currentUVAnimDictionary;
}

RwBool RtDictDestroy(RtDict* dict) {
    if (rw::currentUVAnimDictionary == dict) {
        rw::currentUVAnimDictionary = nullptr;
    }
    dict->destroy();
    return true;
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------
static RwTextureCallBackFind s_TextureFindCB;

RwTextureCallBackFind RwTextureSetFindCallBack(RwTextureCallBackFind cb) {
    // TODO(librw): route rw::Texture::read through this callback
    auto old = s_TextureFindCB;
    s_TextureFindCB = cb;
    return old;
}

RwTextureCallBackFind RwTextureGetFindCallBack() {
    return s_TextureFindCB;
}

// Callers have already found the rwID_TEXDICTIONARY chunk header, as with RW.
RwTexDictionary* RwTexDictionaryStreamRead(RwStream* stream) {
    return rw::TexDictionary::streamRead(stream);
}

RwTexDictionary* RwTexDictionaryGtaStreamRead(RwStream* stream) {
    // TODO(librw): SA's reader also skips textures by quality setting
    return RwTexDictionaryStreamRead(stream);
}

// SA splits big TXDs over two frames. librw has no partial reads, so pass 1 reads it all.
RwTexDictionary* RwTexDictionaryGtaStreamRead1(RwStream* stream) {
    return RwTexDictionaryStreamRead(stream);
}

RwTexDictionary* RwTexDictionaryGtaStreamRead2(RwStream*, RwTexDictionary* txd) {
    return txd;
}

RwTexture* GetFirstTexture(RwTexDictionary* txd) {
    auto* lnk = txd->textures.link.next;
    return lnk == txd->textures.end() ? nullptr : rw::Texture::fromDict(lnk);
}


// ---------------------------------------------------------------------------
// D3D9 device
// ---------------------------------------------------------------------------
static rwD3D9DeviceRestoreCallBack s_RestoreCB;

void _rwD3D9DeviceSetRestoreCallback(rwD3D9DeviceRestoreCallBack cb) { s_RestoreCB = cb; }
rwD3D9DeviceRestoreCallBack _rwD3D9DeviceGetRestoreCallback() { return s_RestoreCB; }

RwBool RwD3D9SetRenderState(RwUInt32 state, RwUInt32 value) {
    rw::d3d::setRenderState(state, value);
    return true;
}

RwBool RwD3D9SetTransform(RwUInt32 state, const void* matrix) {
    return SUCCEEDED(rw::d3d::d3ddevice->SetTransform((D3DTRANSFORMSTATETYPE)state, (const D3DMATRIX*)matrix));
}

const void* RwD3D9GetCaps() {
    static D3DCAPS9 caps;
    rw::d3d::d3ddevice->GetDeviceCaps(&caps);
    return &caps;
}

// TODO(librw): multisampling and refresh rate are chosen at device creation in librw
RwBool   RwD3D9ChangeMultiSamplingLevels(RwUInt32) { return true; }
void     RwD3D9EngineSetMultiSamplingLevels(RwUInt32) {}
RwUInt32 RwD3D9EngineGetMaxMultiSamplingLevels() { return 1; }
void     RwD3D9EngineSetRefreshRate(RwUInt32) {}
RwBool   RwD3D9DeviceSupportsDXTTexture() { return true; }
void     RwD3D9SetStencilClear(RwUInt32) {}
RwBool   RwD3D9CameraAttachWindow(void*, void*) { return true; }

// ---------------------------------------------------------------------------
// Skin / hanim
// ---------------------------------------------------------------------------
RpSkin* RpSkinCreate(RwUInt32 numVertices, RwUInt32 numBones, RwMatrixWeights* weights, RwUInt32* indices, RwMatrix* inverseMatrices) {
    auto* skin = rwNewT(rw::Skin, 1, rw::MEMDUR_EVENT | rw::ID_SKIN);
    memset(skin, 0, sizeof(*skin));
    skin->init(numBones, numBones, numVertices);
    memcpy(skin->weights, weights, numVertices * sizeof(RwMatrixWeights));
    memcpy(skin->indices, indices, numVertices * sizeof(RwUInt32));
    for (RwUInt32 i = 0; i < numBones; i++) {
        memcpy(&skin->inverseMatrices[i * 16], &inverseMatrices[i], sizeof(RwMatrix));
    }
    skin->findNumWeights(numVertices);
    skin->findUsedBones(numVertices);
    return skin;
}

RpGeometry* RpSkinGeometrySetSkin(RpGeometry* geometry, RpSkin* skin) {
    rw::Skin::set(geometry, skin);
    return geometry;
}

RpHAnimHierarchy* RpHAnimHierarchyCreateFromHierarchy(RpHAnimHierarchy* hierarchy, RpHAnimHierarchyFlag flags, RwInt32 maxKeyFrameSize) {
    const auto n = hierarchy->numNodes;
    std::vector<rw::int32> nodeFlags(n), nodeIDs(n);
    for (rw::int32 i = 0; i < n; i++) {
        nodeFlags[i] = hierarchy->nodeInfo[i].flags;
        nodeIDs[i]   = hierarchy->nodeInfo[i].id;
    }
    auto* h = rw::HAnimHierarchy::create(n, nodeFlags.data(), nodeIDs.data(), flags, maxKeyFrameSize);
    h->parentFrame = hierarchy->parentFrame;
    h->parentHierarchy = hierarchy;
    return h;
}

// ---------------------------------------------------------------------------
// RtAnim
// ---------------------------------------------------------------------------
RwBool RtAnimInitialize() { return true; } // librw registers the anim module in Engine::init

RwBool RtAnimRegisterInterpolationScheme(RtAnimInterpolatorInfo* info) {
    rw::AnimInterpolatorInfo::registerInterp(info);
    return true;
}

RtAnimAnimation* RtAnimAnimationCreate(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags, RwReal duration) {
    auto* info = rw::AnimInterpolatorInfo::find(typeID);
    return info ? rw::Animation::create(info, numFrames, flags, duration) : nullptr;
}

RwBool RtAnimAnimationDestroy(RtAnimAnimation* animation) {
    animation->destroy();
    return true;
}

RwBool RtAnimInterpolatorSetCurrentAnim(RtAnimInterpolator* interp, RtAnimAnimation* anim) {
    return interp->setCurrentAnim(anim);
}

RwBool RpUVAnimPluginAttach() {
    rw::registerUVAnimPlugin();
    return true;
}

RwBool RwD3D9ChangeVideoMode(RwInt32 mode) {
    return rw::Engine::setVideoMode(mode);
}

// ---------------------------------------------------------------------------
// Clump streaming
// ---------------------------------------------------------------------------
static RpClump* s_PendingClump;

RwBool RpClumpGtaStreamRead1(RwStream* stream) {
    // TODO(librw): SA splits this so pass 2 can run on a later frame. Chunk header already found by the caller.
    RpClumpGtaCancelStream();
    s_PendingClump = RpClumpStreamRead(stream);
    return s_PendingClump != nullptr;
}

RpClump* RpClumpGtaStreamRead2(RwStream*) {
    return std::exchange(s_PendingClump, nullptr);
}

void RpClumpGtaCancelStream() {
    if (s_PendingClump) {
        RpClumpDestroy(std::exchange(s_PendingClump, nullptr));
    }
}

RwStream* _rwStreamInitialize(RwStream*, RwBool, RwStreamType type, RwStreamAccessType access, const void* data) {
    // librw allocates its own stream object; callers only use the returned pointer
    return RwStreamOpen(type, access, data);
}
#endif
