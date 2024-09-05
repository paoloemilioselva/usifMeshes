#include "usifMeshes.h"

PXR_NAMESPACE_USING_DIRECTIVE
TF_DEFINE_PUBLIC_TOKENS(myPrimvars, MY_PRIMVARS);

pxr::HdDataSourceLocator L(const std::string& inputStr)
{
    std::vector<pxr::TfToken> tokens;
    for (const std::string& s : pxr::TfStringSplit(inputStr, "/")) {
        tokens.push_back(pxr::TfToken(s));
    }
    return pxr::HdDataSourceLocator(tokens.size(), tokens.data());
}

#define SET_AND_OVERLAY(data,type,value,what,where) { \
    pxr::HdContainerDataSourceHandle container = pxr::HdContainerDataSourceEditor()\
        .Set(L(what), pxr::HdRetainedTypedSampledDataSource<type>::New(value))\
        .Finish();\
    data = pxr::HdContainerDataSourceEditor(data)\
        .Overlay(where, container)\
        .Finish(); \
}

usifMeshesRefPtr usifMeshes::New(
    const pxr::HdSceneIndexBaseRefPtr& inputSceneIndex,
    const pxr::HdContainerDataSourceHandle& inputArgs)
{
    return pxr::TfCreateRefPtr( new usifMeshes(inputSceneIndex, inputArgs) );
}

usifMeshes::usifMeshes(
    const pxr::HdSceneIndexBaseRefPtr& inputSceneIndex,
    const pxr::HdContainerDataSourceHandle& inputArgs)
    : pxr::HdSingleInputFilteringSceneIndexBase(inputSceneIndex)
{
    
    INFO("usifMeshes SceneIndex plugin loaded");
}

usifMeshes::~usifMeshes()
{
}

void usifMeshes::initialize()
{
}

void usifMeshes::shutdown()
{
}

pxr::HdSceneIndexPrim usifMeshes::GetPrim(const pxr::SdfPath& primPath) const
{
    pxr::HdSceneIndexPrim prim = _GetInputSceneIndex()->GetPrim(primPath);

    {
        pxr::SdfPath orig;
        for (auto it = m_labelledPrims.begin(); it != m_labelledPrims.end(); ++it)
        {
            if (it->second == primPath)
            {
                orig = it->first;
                break;
            }
        }
        if (!orig.IsEmpty())
        {
            auto origPathText = orig.GetString();
            pxr::HdSceneIndexPrim origPrim = _GetInputSceneIndex()->GetPrim(orig);

            auto editXform = pxr::HdXformSchema::GetFromParent(origPrim.dataSource).GetMatrix().get()->GetValue(0.0).Get<pxr::GfMatrix4d>();

            pxr::VtIntArray faceVertexCounts;
            pxr::VtIntArray faceVertexIndices;
            pxr::VtVec3fArray points;

            int sx = 1000;
            int sy = 1000;
            int blades = sx*sy;
            faceVertexCounts.resize(blades);
            faceVertexIndices.resize(blades*3);
            points.resize(blades*3);

            for (size_t y = 0; y < sy; ++y)
                for (size_t x = 0; x < sx; ++x)
                {
                    int i = y * sx + x;
                    faceVertexCounts[i] = 3;
                    faceVertexIndices[i * 3 + 0] = i * 3 + 0;
                    faceVertexIndices[i * 3 + 1] = i * 3 + 1;
                    faceVertexIndices[i * 3 + 2] = i * 3 + 2;
                    points[i * 3 + 0] = editXform.Transform(GfVec3f(-0.5+x, 0.0, 0.0+y));
                    points[i * 3 + 1] = editXform.Transform(GfVec3f(0.0+x, 10.0, 0.0+y));
                    points[i * 3 + 2] = editXform.Transform(GfVec3f(0.5+x, 0.0, 0.0+y));
                }

            prim.dataSource = pxr::HdRetainedContainerDataSource::New(

                pxr::HdPrimvarsSchemaTokens->primvars,
                pxr::HdRetainedContainerDataSource::New(
                    pxr::HdTokens->points,
                    pxr::HdPrimvarSchema::Builder()
                    .SetPrimvarValue(pxr::HdRetainedTypedSampledDataSource<pxr::VtVec3fArray>::New(points))
                    .SetInterpolation(pxr::HdPrimvarSchema::BuildInterpolationDataSource(pxr::HdPrimvarSchemaTokens->vertex))
                    .SetRole(pxr::HdPrimvarSchema::BuildRoleDataSource(pxr::HdPrimvarSchemaTokens->point))
                    .Build(),
                    pxr::HdTokens->displayColor,
                    pxr::HdPrimvarSchema::Builder()
                    .SetPrimvarValue(pxr::HdRetainedTypedSampledDataSource<VtVec3fArray>::New(pxr::VtVec3fArray{ {1.0f,0.0f,0.0f} }))
                    .SetInterpolation(pxr::HdPrimvarSchema::BuildInterpolationDataSource(pxr::HdPrimvarSchemaTokens->constant))
                    .SetRole(pxr::HdPrimvarSchema::BuildRoleDataSource(pxr::HdPrimvarSchemaTokens->color))
                    .Build(),
                    pxr::HdTokens->displayOpacity,
                    pxr::HdPrimvarSchema::Builder()
                    .SetPrimvarValue(pxr::HdRetainedTypedSampledDataSource<VtFloatArray>::New(pxr::VtFloatArray{ 1.0f }))
                    .SetInterpolation(pxr::HdPrimvarSchema::BuildInterpolationDataSource(pxr::HdPrimvarSchemaTokens->constant))
                    .Build()
                ),
                pxr::HdMeshSchemaTokens->mesh,
                pxr::HdMeshSchema::Builder()
                .SetTopology(pxr::HdMeshTopologySchema::Builder()
                    .SetFaceVertexCounts(pxr::HdRetainedTypedSampledDataSource<pxr::VtIntArray>::New(faceVertexCounts))
                    .SetFaceVertexIndices(pxr::HdRetainedTypedSampledDataSource<pxr::VtIntArray>::New(faceVertexIndices))
                    .SetOrientation(pxr::HdMeshTopologySchema::BuildOrientationDataSource(pxr::HdMeshTopologySchemaTokens->leftHanded))
                    .Build())
                .Build(),

                pxr::HdXformSchemaTokens->xform,
                pxr::HdXformSchema::Builder()
                .SetMatrix(pxr::HdRetainedTypedSampledDataSource<pxr::GfMatrix4d>::New(pxr::GfMatrix4d().SetIdentity()))
                .Build()
            );
        }
    }

    return prim;
}

pxr::SdfPathVector usifMeshes::GetChildPrimPaths(const pxr::SdfPath& primPath) const
{
    return _GetInputSceneIndex()->GetChildPrimPaths(primPath);
}

void usifMeshes::_PrimsAdded(
    const pxr::HdSceneIndexBase& sender,
    const pxr::HdSceneIndexObserver::AddedPrimEntries& entries)
{
    pxr::HdSceneIndexObserver::AddedPrimEntries newEntries;

    for (const pxr::HdSceneIndexObserver::AddedPrimEntry& entry : entries)
    {
        const pxr::SdfPath& path = entry.primPath;
        auto primType = _GetInputSceneIndex()->GetPrim(path).primType;
        pxr::HdContainerDataSourceHandle dataSource = _GetInputSceneIndex()->GetPrim(path).dataSource;

        bool createNewPrim = false;
        GetPrimvar(dataSource, myPrimvars->test, createNewPrim);
        if (createNewPrim && path.GetString().find("/_new_/") == std::string::npos)
        {
            m_labelledPrims[path] = pxr::SdfPath();
        }
    }

    if (!newEntries.empty())
    {
        _SendPrimsAdded(newEntries);
    }

    _SendPrimsAdded(entries);
}

void usifMeshes::_PrimsRemoved(
    const pxr::HdSceneIndexBase& sender,
    const pxr::HdSceneIndexObserver::RemovedPrimEntries& entries)
{
    if (!_IsObserved()) {
        return;
    }
    _SendPrimsRemoved(entries);
}

void usifMeshes::_PrimsDirtied(
    const pxr::HdSceneIndexBase& sender,
    const pxr::HdSceneIndexObserver::DirtiedPrimEntries& entries)
{
    pxr::HdSceneIndexObserver::DirtiedPrimEntries dirtiedEntries;
    pxr::HdSceneIndexObserver::AddedPrimEntries newEntries;

    for (const pxr::HdSceneIndexObserver::DirtiedPrimEntry& entry : entries)
    {
        const pxr::SdfPath& path = entry.primPath;
        auto primType = _GetInputSceneIndex()->GetPrim(path).primType;

        for (auto it = m_labelledPrims.begin(); it != m_labelledPrims.end(); ++it)
        {
            m_labelledPrims[it->first] = pxr::SdfPath( it->first.GetString() + "/_new_/mesh");
            newEntries.emplace_back(
                pxr::HdSceneIndexObserver::AddedPrimEntry(
                    m_labelledPrims[it->first],
                    HdMeshSchemaTokens->mesh
                )
            );
        }

        for (auto it = m_labelledPrims.begin(); it != m_labelledPrims.end(); ++it)
        {
            if (!it->second.IsEmpty())
            {
                dirtiedEntries.emplace_back(it->second,
                    pxr::HdDataSourceLocatorSet{
                        pxr::HdXformSchema::GetDefaultLocator(),
                        pxr::HdLightSchema::GetDefaultLocator(),
                        pxr::HdMaterialSchema::GetDefaultLocator(),
                        pxr::HdPrimvarsSchema::GetDefaultLocator(),
                        pxr::HdMeshSchema::GetDefaultLocator(),
                        pxr::HdDependenciesSchema::GetDefaultLocator(),
                        pxr::HdExtComputationPrimvarsSchema::GetDefaultLocator(),
                        pxr::HdExtComputationSchema::GetDefaultLocator(),
                        pxr::HdSystemSchema::GetDefaultLocator(),
                        pxr::HdInstancedBySchema::GetDefaultLocator(),
                        pxr::HdMaterialBindingsSchema::GetDefaultLocator(),
                        pxr::HdVisibilitySchema::GetDefaultLocator(),
                        pxr::HdSceneGlobalsSchema::GetDefaultLocator()
                    }
                );
            }
        }
    }

    _SendPrimsDirtied(entries);

    if (!newEntries.empty())
    {
        _SendPrimsAdded(newEntries);
    }

    if (!dirtiedEntries.empty())
    {
        _SendPrimsDirtied(dirtiedEntries);
    }
}