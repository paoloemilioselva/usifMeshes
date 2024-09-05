#ifndef USIFMESHESPLUGIN_H
#define USIFMESHESPLUGIN_H

#include <pxr/imaging/hd/sceneIndexPlugin.h>
#include <pxr/pxr.h>

class usifMeshesPlugin: public pxr::HdSceneIndexPlugin
{
public:
    usifMeshesPlugin();
    ~usifMeshesPlugin() override;

protected:
    pxr::HdSceneIndexBaseRefPtr _AppendSceneIndex(
        const pxr::HdSceneIndexBaseRefPtr& inputScene,
        const pxr::HdContainerDataSourceHandle& inputArgs) override;
};

#endif