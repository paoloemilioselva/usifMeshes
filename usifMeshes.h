#ifndef USIFMANAGER_H
#define USIFMANAGER_H

#include <pxr/pxr.h>
#include <pxr/usdImaging/usdImaging/api.h>
#include <pxr/imaging/hd/filteringSceneIndex.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>

//#include <pxr/usdImaging/usdImaging/modelSchema.h>
#include <pxr/usdImaging/usdImaging/usdPrimInfoSchema.h>
#include <pxr/imaging/hd/primvarsSchema.h>
#include <pxr/imaging/hd/xformSchema.h>
#include <pxr/imaging/hd/lightSchema.h>
#include <pxr/imaging/hd/meshSchema.h>
#include <pxr/imaging/hd/materialSchema.h>
#include <pxr/imaging/hd/dependenciesSchema.h>
#include <pxr/imaging/hd/systemSchema.h>
#include <pxr/imaging/hd/extComputationSchema.h>
#include <pxr/imaging/hd/extComputationPrimvarsSchema.h>
#include <pxr/imaging/hd/sceneGlobalsSchema.h>
#include <pxr/imaging/hd/primOriginSchema.h>
#include <pxr/imaging/hd/instancedBySchema.h>
#include <pxr/imaging/hd/meshTopologySchema.h>
#include <pxr/imaging/hd/tokens.h>

#include <pxr/base/gf/rotation.h>
#include <pxr/imaging/hd/retainedDataSource.h>
#include <pxr/imaging/hd/visibilitySchema.h>
#include <pxr/imaging/hd/overlayContainerDataSource.h>
#include <pxr/imaging/hd/containerDataSourceEditor.h>
#include <pxr/imaging/hd/dataSourceMaterialNetworkInterface.h>
#include <pxr/imaging/hd/materialBindingsSchema.h>
#include <pxr/imaging/hd/materialConnectionSchema.h>
#include <pxr/imaging/hd/invalidatableContainerDataSource.h>
#include <pxr/imaging/hd/retainedSceneIndex.h>

#include <pxr/usd/usdLux/lightAPI.h>

#include <pxr/base/vt/types.h>
#include <pxr/imaging/hd/meshUtil.h>

#include <iostream>

#include <map>


PXR_NAMESPACE_USING_DIRECTIVE

#define MY_PRIMVARS \
    ((test,  "usifMeshes:test"))\
    ((test2, "usifMeshes:test2"))

TF_DECLARE_PUBLIC_TOKENS(myPrimvars, MY_PRIMVARS);

TF_DECLARE_REF_PTRS(usifMeshes);

#define INFO(what) std::cout << "{usifMeshes} info : " << what << std::endl;
#define WARNING(what) std::cout << "{usifMeshes} warning : " << what << std::endl;
#define ERROR(what) std::cout << "{usifMeshes} error : " << what << std::endl;

template<typename T>
bool GetPrimvar(pxr::HdContainerDataSourceHandle& i_dataSource, pxr::TfToken& i_primVarName, T& o_value)
{
    if (pxr::HdSampledDataSourceHandle countDs =
        pxr::HdPrimvarsSchema::GetFromParent(i_dataSource)
        .GetPrimvar(i_primVarName)
        .GetPrimvarValue())
    {
        VtValue v = countDs->GetValue(0.0f);
        if (v.IsHolding<T>())
        {
            o_value = v.Get<T>();
            return true;
        }
    }
    return false;
}

class usifMeshes : public pxr::HdSingleInputFilteringSceneIndexBase
{
public:
    static usifMeshesRefPtr New(
        const pxr::HdSceneIndexBaseRefPtr& inputSceneIndex,
        const pxr::HdContainerDataSourceHandle& inputArgs);

    virtual ~usifMeshes() override;

    virtual pxr::HdSceneIndexPrim GetPrim(const pxr::SdfPath& primPath) const override;
    virtual pxr::SdfPathVector GetChildPrimPaths(const pxr::SdfPath& primPath) const override; 

    void initialize();
    void shutdown();

protected:
    usifMeshes(
        const pxr::HdSceneIndexBaseRefPtr& inputSceneIndex,
        const pxr::HdContainerDataSourceHandle& inputArgs);

    void _PrimsAdded(
        const pxr::HdSceneIndexBase& sender,
        const pxr::HdSceneIndexObserver::AddedPrimEntries& entries) override;

    void _PrimsRemoved(
        const pxr::HdSceneIndexBase& sender,
        const pxr::HdSceneIndexObserver::RemovedPrimEntries& entries) override;

    void _PrimsDirtied(
        const pxr::HdSceneIndexBase& sender,
        const pxr::HdSceneIndexObserver::DirtiedPrimEntries& entries) override;

private:
    bool m_labels;
    std::map< pxr::SdfPath, pxr::SdfPath > m_labelledPrims;
};

#endif