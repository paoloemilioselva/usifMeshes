#include "usifMeshesPlugin.h"
#include "usifMeshes.h"

#include <pxr/imaging/hd/sceneIndexPluginRegistry.h>
#include <pxr/imaging/hd/tokens.h>

PXR_NAMESPACE_USING_DIRECTIVE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens, ((sceneIndexPluginName, "usifMeshesPlugin")));

TF_REGISTRY_FUNCTION(TfType)
{
    HdSceneIndexPluginRegistry::Define<usifMeshesPlugin>();
}

TF_REGISTRY_FUNCTION(HdSceneIndexPlugin)
{
    const pxr::HdSceneIndexPluginRegistry::InsertionPhase insertionPhase = 0;
    const pxr::HdSceneIndexPluginRegistry::InsertionOrder insertionOrder = pxr::HdSceneIndexPluginRegistry::InsertionOrderAtStart;
    const std::string rendererDisplayName = "";
    pxr::HdSceneIndexPluginRegistry::GetInstance().RegisterSceneIndexForRenderer(
        rendererDisplayName, _tokens->sceneIndexPluginName, nullptr,
        insertionPhase, insertionOrder);
}

usifMeshesPlugin::usifMeshesPlugin() = default;
usifMeshesPlugin::~usifMeshesPlugin() = default;

pxr::HdSceneIndexBaseRefPtr usifMeshesPlugin::_AppendSceneIndex(
    const pxr::HdSceneIndexBaseRefPtr& inputSceneIndex,
    const pxr::HdContainerDataSourceHandle& inputArgs)
{
    TF_UNUSED(inputArgs);
    return usifMeshes::New( inputSceneIndex, {});
}
