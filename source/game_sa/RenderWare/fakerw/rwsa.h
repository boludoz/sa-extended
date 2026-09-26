#pragma once

// San Andreas additions on top of the fake RW layer (librw build only).
// Globals that the thunk build reads from game memory live here as real objects.

#include "skeleton.h"

extern RsGlobalType RsGlobal;
extern bool         RwInitialized;
extern RwRGBAReal   AmbientSaturated;

struct IDirect3DDevice9;
IDirect3DDevice9* GetD3D9Device();

void _rpMaterialSetDefaultSurfaceProperties(RwSurfaceProperties* surfProps);

// Names the SA headers use that the fake layer does not provide.
typedef RwMatrix  RwMatrixTag;
typedef rw::LLLink RwLLLink;
typedef RwUInt16  RxVertexIndex;
typedef RwIm2DVertex RwD3D9Vertex; // same layout: x, y, z, rhw, colour, u, v
typedef RwTexture* (*RwTextureCallBackFind)(const RwChar* name);
typedef rw::Plane RwPlane;
typedef RwIm3DVertex RxObjSpace3DVertex;

// In RW this is a macro that accepts any object whose parent is a frame.
inline RwFrame* RwFrameGetParent(const RwObject* object) {
    return static_cast<RwFrame*>(object->parent);
}
typedef rw::Animation         RtAnimAnimation;
typedef rw::AnimInterpolator  RtAnimInterpolator;

// ---------------------------------------------------------------------------
// Math / misc macros from the RW SDK
// ---------------------------------------------------------------------------
#define RWRAD2DEG(x)   ((x) * (180.0f / 3.14159265358979323846f))
#define RwRealMAXVAL   3.40282347e+38f
#define rwMATRIXINTERNALIDENTITY rw::Matrix::IDENTITY

inline void RwV3dTransformPoint(RwV3d* out, const RwV3d* in, const RwMatrix* m) { rw::V3d::transformPoints(out, in, 1, m); }
inline void RwV3dTransformVector(RwV3d* out, const RwV3d* in, const RwMatrix* m) { rw::V3d::transformVectors(out, in, 1, m); }

// ---------------------------------------------------------------------------
// Objects
// ---------------------------------------------------------------------------
#define rwObjectSetFlags(o, f)        (((RwObject*)(o))->flags = (RwUInt8)(f))
#define rwObjectTestFlags(o, f)       (((const RwObject*)(o))->flags & (f))
#define rwObjectHasFrameSetFrame(o, f) (((rw::ObjectWithFrame*)(o))->setFrame(f))

// ---------------------------------------------------------------------------
// Vertices. On D3D9 RxObjSpace3DVertex and RwIm3DVertex are the same struct.
// ---------------------------------------------------------------------------
typedef RwIm3DVertex RxObjSpace3DLitVertex;

inline void RxObjSpace3DVertexSetPos(RxObjSpace3DVertex* v, const RwV3d* p) { RwIm3DVertexSetPos(v, p->x, p->y, p->z); }
inline void RxObjSpace3DVertexSetU(RxObjSpace3DVertex* v, RwReal u) { RwIm3DVertexSetU(v, u); }
inline void RxObjSpace3DVertexSetV(RxObjSpace3DVertex* v, RwReal t) { RwIm3DVertexSetV(v, t); }
inline void RxObjSpace3DVertexSetPreLitColor(RxObjSpace3DVertex* v, const RwRGBA* c) { RwIm3DVertexSetRGBA(v, c->red, c->green, c->blue, c->alpha); }
inline RwV3d* RwIm3DVertexGetPos(RwIm3DVertex* v) { return &v->objVertex; }
inline void RwIm2DVertexSetRealRGBA(RwIm2DVertex* v, RwReal r, RwReal g, RwReal b, RwReal a) {
    RwIm2DVertexSetIntRGBA(v, (RwUInt8)r, (RwUInt8)g, (RwUInt8)b, (RwUInt8)a);
}
#define RwIm2DGetNearScreenZMacro() RwIm2DGetNearScreenZ()

// ---------------------------------------------------------------------------
// Rasters
// ---------------------------------------------------------------------------
enum {
    rwRASTERLOCKREADWRITE    = rw::Raster::LOCKREAD | rw::Raster::LOCKWRITE,
    rwRASTERPIXELLOCKEDWRITE = rw::Raster::PRIVATELOCK_WRITE,
};

// ---------------------------------------------------------------------------
// Streams / chunks
// ---------------------------------------------------------------------------
typedef rw::ChunkHeaderInfo RwChunkHeaderInfo;
inline RwStream* RwStreamReadChunkHeaderInfo(RwStream* s, RwChunkHeaderInfo* info) {
    return rw::readChunkHeaderInfo(s, info) ? s : nullptr;
}

// ---------------------------------------------------------------------------
// Geometry / materials
// ---------------------------------------------------------------------------
typedef rw::MaterialList RpMaterialList;
inline RwInt32 _rpMaterialListAppendMaterial(RpMaterialList* list, RpMaterial* mat) { return list->appendMaterial(mat); }
inline void _rpMaterialListDeinitialize(RpMaterialList* list) { list->deinit(); }

// ---------------------------------------------------------------------------
// Animation
// ---------------------------------------------------------------------------
typedef rw::AnimInterpolatorInfo RtAnimInterpolatorInfo;
#define rtANIMGETINTERPFRAME(interp, n) ((interp)->getInterpFrame(n))
inline RwMatrix* RpHAnimHierarchyGetNodeMatrix(RpHAnimHierarchy* h, RwInt32 i) { return &RpHAnimHierarchyGetMatrixArray(h)[i]; }

// ---------------------------------------------------------------------------
// UV animation dictionaries (RtDict in the SDK)
// ---------------------------------------------------------------------------
typedef rw::UVAnimDictionary RtDict;
struct RtDictSchema {};
extern RtDictSchema RpUVAnimDictSchema;
enum { rwID_UVANIMDICT = rw::ID_UVANIMDICT };
RtDict* RtDictSchemaStreamReadDict(RtDictSchema* schema, RwStream* stream);
RtDictSchema* RtDictSchemaSetCurrentDict(RtDictSchema* schema, RtDict* dict);
RtDict* RtDictSchemaGetCurrentDict(RtDictSchema* schema);
RwBool RtDictDestroy(RtDict* dict);

// ---------------------------------------------------------------------------
// Skinning
// ---------------------------------------------------------------------------
enum RpSkinType { rpSKINTYPENASKINTYPE = 0, rpSKINTYPEGENERIC, rpSKINTYPEMATFX, rpSKINTYPETOON, rpSKINTYPEMATFXTOON };
inline RpAtomic* RpSkinAtomicSetType(RpAtomic* atomic, RpSkinType) { return atomic; }

// ---------------------------------------------------------------------------
// Engine / debug
// ---------------------------------------------------------------------------
inline RwInt32 RwEngineGetVersion() { return 0x36003; }
// TODO(librw): hook librw's error reporting up to the skeleton's debug handler
template<typename F> inline F RwDebugSetHandler(F) { return nullptr; }
inline void RwDebugSetTraceState(RwBool) {}

typedef RwTexture* (*RwTextureCallBackRead)(const RwChar* name, const RwChar* mask);
RwTextureCallBackFind RwTextureSetFindCallBack(RwTextureCallBackFind cb);
RwTextureCallBackFind RwTextureGetFindCallBack();
RwTexDictionary* RwTexDictionaryGtaStreamRead(RwStream* stream);
RwTexDictionary* RwTexDictionaryGtaStreamRead1(RwStream* stream);
RwTexDictionary* RwTexDictionaryGtaStreamRead2(RwStream* stream, RwTexDictionary* txd);

// ---------------------------------------------------------------------------
// D3D9 device access (librw owns the device)
// ---------------------------------------------------------------------------
#define _RwD3DDevice (rw::d3d::d3ddevice)
using rw::d3d::d3ddevice;
inline void* RwD3D9GetCurrentD3DDevice() { return rw::d3d::d3ddevice; }

typedef void (*rwD3D9DeviceRestoreCallBack)();
void _rwD3D9DeviceSetRestoreCallback(rwD3D9DeviceRestoreCallBack cb);
rwD3D9DeviceRestoreCallBack _rwD3D9DeviceGetRestoreCallback();

RwBool RwD3D9SetRenderState(RwUInt32 state, RwUInt32 value);
RwBool RwD3D9SetTransform(RwUInt32 state, const void* matrix);
const void* RwD3D9GetCaps();
RwBool RwD3D9ChangeMultiSamplingLevels(RwUInt32 levels);
void   RwD3D9EngineSetMultiSamplingLevels(RwUInt32 levels);
RwUInt32 RwD3D9EngineGetMaxMultiSamplingLevels();
void   RwD3D9EngineSetRefreshRate(RwUInt32 rate);
RwBool RwD3D9DeviceSupportsDXTTexture();
void   RwD3D9SetStencilClear(RwUInt32 value);
RwBool RwD3D9CameraAttachWindow(void* camera, void* hwnd);

// SA's own D3D resource helpers (D3DResourceSystem is not built with librw)
RwTexture* GetFirstTexture(RwTexDictionary* txd);

// ---------------------------------------------------------------------------
// Field accessors shared with the thunk build (see RenderWare.h for the RW side)
// ---------------------------------------------------------------------------
inline RwInt32   RpHAnimHierarchyGetNodeID(const RpHAnimHierarchy* h, RwInt32 i)    { return h->nodeInfo[i].id; }
inline RwInt32   RpHAnimHierarchyGetNodeFlags(const RpHAnimHierarchy* h, RwInt32 i) { return h->nodeInfo[i].flags; }
inline RtAnimInterpolator* RpHAnimHierarchyGetInterpolator(const RpHAnimHierarchy* h) { return h->interpolator; }
inline rw::MeshHeader* RpGeometryGetMeshHeader(const RpGeometry* g) { return g->meshHeader; }
inline RtAnimAnimation*& RtAnimInterpolatorCurrentAnim(RtAnimInterpolator* i) { return i->currentAnim; }

// ---------------------------------------------------------------------------
// RW implements these as macros, so SA passes any object (or a const matrix).
// ---------------------------------------------------------------------------
inline const RwV3d* RwMatrixGetPos(const RwMatrix* m) { return &m->pos; }
inline RwUInt8  RwObjectGetType(const void* o) { return static_cast<const RwObject*>(o)->type; }
inline RwFrame* RpClumpGetFrame(const RwObject* o) { return static_cast<RwFrame*>(o->parent); }
inline RwFrame* RpAtomicGetFrame(const RwObject* o) { return static_cast<RwFrame*>(o->parent); }
inline RpAtomic* RpAtomicSetFlags(RwObject* o, RwUInt32 f) { return RpAtomicSetFlags(reinterpret_cast<RpAtomic*>(o), f); }
inline RwUInt32 RpAtomicGetFlags(const RwObject* o) { return RpAtomicGetFlags(reinterpret_cast<const RpAtomic*>(o)); }
inline RwTexture* RwTextureSetAddressingU(RwTexture* t, RwInt32 m) { return RwTextureSetAddressingU(t, (RwTextureAddressMode)m); }
inline RwTexture* RwTextureSetAddressingV(RwTexture* t, RwInt32 m) { return RwTextureSetAddressingV(t, (RwTextureAddressMode)m); }
inline RwTexture* RwTextureSetAddressing(RwTexture* t, RwInt32 m) { return RwTextureSetAddressing(t, (RwTextureAddressMode)m); }

// ---------------------------------------------------------------------------
// Second batch: skin, hanim, RtAnim, UV anim, meshes, clump streaming
// ---------------------------------------------------------------------------
typedef rw::ObjPipeline RxPipeline;
typedef rw::Mesh        RpMesh;
typedef rw::Skin        RpSkin;

RpSkin*     RpSkinCreate(RwUInt32 numVertices, RwUInt32 numBones, RwMatrixWeights* weights, RwUInt32* indices, RwMatrix* inverseMatrices);
RpGeometry* RpSkinGeometrySetSkin(RpGeometry* geometry, RpSkin* skin);
RpHAnimHierarchy* RpHAnimHierarchyCreateFromHierarchy(RpHAnimHierarchy* hierarchy, RpHAnimHierarchyFlag flags, RwInt32 maxKeyFrameSize);

RwBool           RtAnimInitialize();
RwBool           RtAnimRegisterInterpolationScheme(RtAnimInterpolatorInfo* info);
RtAnimAnimation* RtAnimAnimationCreate(RwInt32 typeID, RwInt32 numFrames, RwInt32 flags, RwReal duration);
RwBool           RtAnimAnimationDestroy(RtAnimAnimation* animation);
RwBool           RtAnimInterpolatorSetCurrentAnim(RtAnimInterpolator* interp, RtAnimAnimation* anim);

RwBool      RpUVAnimPluginAttach();
inline RwBool RpMaterialUVAnimExists(const RpMaterial* m) { return rw::UVAnim::exists(const_cast<RpMaterial*>(m)); }
inline RpMaterial* RpMaterialUVAnimAddAnimTime(RpMaterial* m, RwReal t) { rw::UVAnim::addTime(m, t); return m; }
inline RpMaterial* RpMaterialUVAnimApplyUpdate(RpMaterial* m) { rw::UVAnim::applyUpdate(m); return m; }

#define RpAtomicRenderMacro(a) RpAtomicRender(a)
inline RpMesh* RpGeometryGetMesh(RpGeometry* g, RwUInt32 i) { return &g->meshHeader->getMeshes()[i]; }

RwBool RwD3D9ChangeVideoMode(RwInt32 mode);

// SA's two-pass clump loader (ClumpLoad.cpp). librw has no partial loads, so pass 1 reads
// the whole clump and pass 2 hands it over.
RwBool   RpClumpGtaStreamRead1(RwStream* stream);
RpClump* RpClumpGtaStreamRead2(RwStream* stream);
void     RpClumpGtaCancelStream();

RwStream* _rwStreamInitialize(RwStream* stream, RwBool rwOwned, RwStreamType type, RwStreamAccessType access, const void* data);

// ---------------------------------------------------------------------------
// Third batch
// ---------------------------------------------------------------------------
#define RWPLUGINOFFSETCONST(_type, _base, _offset) ((const _type*)((const RwUInt8*)(_base) + (_offset)))

struct RwResEntry; // pipeline instance data; the SA pipelines are not ported yet

inline void RwV2dSub(RwV2d* out, const RwV3d* a, const RwV3d* b) { out->x = a->x - b->x; out->y = a->y - b->y; }

// D3DResourceSystem pools D3D textures/index buffers behind RW's back; librw manages its own.
class D3DResourceSystem {
public:
    static void CancelBuffering() {}
    static void Init() {}
    static void Shutdown() {}
    static void SetUseD3DResourceBuffering(bool) {}
    static void TidyUpD3DIndexBuffers(uint32) {}
    static void TidyUpD3DTextures(uint32) {}
};
#define rwMatrixSetFlags(m, f) ((m)->flags = (f))
typedef RwBool (*RxD3D9AllInOneInstanceCallBack)(void* object, RwResEntry* resEntry, RwBool reinstance);
typedef RwBool (*RxD3D9AllInOneReinstanceCallBack)(void* object, RwResEntry* resEntry, void* instanceCallback);
typedef void   (*RxD3D9AllInOneRenderCallBack)(RwResEntry* repEntry, void* object, RwUInt8 type, RwUInt32 flags);
inline RwTexCoords* RpGeometryGetVertexTexCoords(const RpGeometry* g, RwInt32 i) { return RpGeometryGetVertexTexCoords(g, (RwTextureCoordinateIndex)i); }
