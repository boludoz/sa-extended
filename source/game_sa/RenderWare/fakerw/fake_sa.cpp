#include "StdInc.h"

#ifdef LIBRW
// RW functions the fake layer declares but remiami never needed. Kept apart from fake.cpp
// so that file stays close to upstream.

using namespace rw;

RwReal RwV2dLength(const RwV2d* in) { return length(*in); }
RwReal RwV3dNormalize(RwV3d* out, const RwV3d* in) {
    const auto len = length(*in);
    *out = len > 0.0f ? scale(*in, 1.0f / len) : *in;
    return len;
}
void RwV3dAssign(RwV3d* out, const RwV3d* in) { *out = *in; }
void RwV3dCrossProduct(RwV3d* out, const RwV3d* a, const RwV3d* b) { *out = cross(*a, *b); }
RwV3d* RwV3dTransformVectors(RwV3d* out, const RwV3d* in, RwInt32 n, const RwMatrix* m) {
    V3d::transformVectors(out, in, n, m);
    return out;
}
RwMatrix* RwMatrixMultiply(RwMatrix* out, const RwMatrix* a, const RwMatrix* b) { return Matrix::mult(out, a, b); }

RwInt32 RwFrameCount(RwFrame* frame) { return frame->count(); }
RwFrame* _rwFrameCloneAndLinkClones(RwFrame* root) { return root->cloneHierarchy(); }

RwBool RwIm3DRenderPrimitive(RwPrimitiveType primType) {
    im3d::RenderPrimitive((PrimitiveType)primType);
    return true;
}

RwInt32 RwRasterGetFormat(const RwRaster* raster) { return raster->format; }
RwInt32 RwRasterGetStride(const RwRaster* raster) { return raster->stride; }
RwRaster* RwRasterUnlockPalette(RwRaster* raster) {
    raster->unlockPalette();
    return raster;
}

RwUInt32 RwRGBAToPixel(RwRGBA* c, RwInt32 rasterFormat) {
    switch (rasterFormat & 0xF00) {
    case Raster::C1555: return (c->alpha ? 0x8000 : 0) | (c->red >> 3) << 10 | (c->green >> 3) << 5 | (c->blue >> 3);
    case Raster::C565:  return (c->red >> 3) << 11 | (c->green >> 2) << 5 | (c->blue >> 3);
    case Raster::C4444: return (c->alpha >> 4) << 12 | (c->red >> 4) << 8 | (c->green >> 4) << 4 | (c->blue >> 4);
    case Raster::C888:  return 0xFF000000 | c->red << 16 | c->green << 8 | c->blue;
    default:            return c->alpha << 24 | c->red << 16 | c->green << 8 | c->blue; // C8888
    }
}

RwImage* RwImageSetFromRaster(RwImage* image, RwRaster* raster) {
    // TODO(librw): copies through a temporary image instead of converting in place
    Image* tmp = raster->toImage();
    if (!tmp) {
        return nullptr;
    }
    image->allocate();
    memcpy(image->pixels, tmp->pixels, std::min(image->stride * image->height, tmp->stride * tmp->height));
    tmp->destroy();
    return image;
}

RwTexture* RwTexDictionaryRemoveTexture(RwTexture* texture) {
    if (texture->dict) {
        texture->inDict.remove();
        texture->dict = nullptr;
    }
    return texture;
}

RwBool RwTextureSetReadCallBack(RwTextureCallBackRead cb) {
    Texture::readCB = cb;
    return true;
}

RwInt32 RwTexDictionaryRegisterPlugin(RwInt32 size, RwUInt32 id, RwPluginObjectConstructor ctor, RwPluginObjectDestructor dtor, RwPluginObjectCopy copy) {
    return TexDictionary::registerPlugin(size, id, ctor, dtor, (CopyConstructor)copy);
}
RwInt32 RpGeometryRegisterPlugin(RwInt32 size, RwUInt32 id, RwPluginObjectConstructor ctor, RwPluginObjectDestructor dtor, RwPluginObjectCopy copy) {
    return Geometry::registerPlugin(size, id, ctor, dtor, (CopyConstructor)copy);
}
RwInt32 RpGeometryRegisterPluginStream(RwUInt32 id, RwPluginDataChunkReadCallBack read, RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize) {
    return Geometry::registerPluginStream(id, (StreamRead)read, (StreamWrite)write, (StreamGetSize)getSize);
}
RwInt32 RpAtomicRegisterPluginStream(RwUInt32 id, RwPluginDataChunkReadCallBack read, RwPluginDataChunkWriteCallBack write, RwPluginDataChunkGetSizeCallBack getSize) {
    return Atomic::registerPluginStream(id, (StreamRead)read, (StreamWrite)write, (StreamGetSize)getSize);
}

RwInt32 RpGeometryGetNumMaterials(const RpGeometry* geometry) { return geometry->matList.numMaterials; }
RpMaterial* RpGeometryTriangleGetMaterial(const RpGeometry* geometry, const RpTriangle* triangle) {
    return geometry->matList.materials[triangle->matId];
}
RpMaterial* RpMaterialSetSurfaceProperties(RpMaterial* material, const RwSurfaceProperties* props) {
    material->surfaceProps = *props;
    return material;
}
RwInt32 RpMatFXAtomicQueryEffects(RpAtomic* atomic) { return MatFX::getEffects(atomic); }

RwBool RtQuatConvertFromMatrix(RtQuat* const quat, const RwMatrix* const matrix) {
    *reinterpret_cast<Quat*>(quat) = const_cast<RwMatrix*>(matrix)->getRotation();
    return true;
}
RwV3d* RtQuatTransformVectors(RwV3d* out, const RwV3d* in, const RwInt32 n, const RtQuat* quat) {
    const auto& q = *reinterpret_cast<const Quat*>(quat);
    for (RwInt32 i = 0; i < n; i++) {
        out[i] = rotate(in[i], q);
    }
    return out;
}

// The thunk build installs hooks into the game's RW here; nothing to hook with librw.
void RwCoreInjectHooks() {}
namespace RtAnim {
void InjectHooks() {}
};
#endif
