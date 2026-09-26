#include "StdInc.h"

#ifdef LIBRW
// Preliminary librw build: the SA custom pipelines (Pipelines/, from gtasrc/pipes/PC) are not
// ported yet. These stand-ins keep librw's default pipelines on every atomic.
// TODO(librw): port CustomBuilding(DN)Pipeline and CustomCarEnvMapPipeline.

#include "CustomBuildingRenderer.h"
#include "CustomBuildingPipeline.h"
#include "CustomBuildingDNPipeline.h"
#include "CustomCarEnvMapPipeline.h"

bool CCustomBuildingRenderer::Initialise() { return true; }
bool CCustomBuildingRenderer::PluginAttach() { return true; }
void CCustomBuildingRenderer::Shutdown() {}
void CCustomBuildingRenderer::Update() {}
void CCustomBuildingRenderer::InjectHooks() {}
bool CCustomBuildingRenderer::IsCBPCPipelineAttached(RpAtomic*) { return false; }
RpAtomic* CCustomBuildingRenderer::AtomicSetup(RpAtomic* atomic) { return atomic; }

void CCustomBuildingPipeline::InjectHooks() {}

void CCustomBuildingDNPipeline::InjectHooks() {}
void CCustomBuildingDNPipeline::PreRenderUpdate(RpAtomic*, bool) {}
void CCustomBuildingDNPipeline::PreRenderUpdate(RpClump*, bool) {}

bool CCustomCarEnvMapPipeline::RegisterPlugin() { return true; }
bool CCustomCarEnvMapPipeline::CreatePipe() { return true; }
void CCustomCarEnvMapPipeline::DestroyPipe() {}
void CCustomCarEnvMapPipeline::InjectHooks() {}
void CCustomCarEnvMapPipeline::PreRenderUpdate() {}
RpAtomic* CCustomCarEnvMapPipeline::CustomPipeAtomicSetup(RpAtomic* atomic) { return atomic; }
#endif
