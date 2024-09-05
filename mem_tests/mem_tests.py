import os
import sys
import tempfile
import math
import numpy

from pxr import Usd, UsdGeom, UsdLux, Sdf, Gf

def create_furrypatch_geo(mesh, sx, sy):
    faceVertexCounts = []
    faceVertexIndices = []
    points = []
    displayColor = [(1.0,0.0,0.0)]
    displayOpacity = [1.0]
    blades = sx * sy
    faceVertexCounts = numpy.full(blades, 3)
    faceVertexIndices = numpy.full(blades*3, 0)
    points = numpy.full( (blades*3,3), 0.0 )

    for y in range(sy):
        for x in range(sx):
            i = y*sx+x
            faceVertexIndices[i*3+0] = i*3+0
            faceVertexIndices[i*3+1] = i*3+1
            faceVertexIndices[i*3+2] = i*3+2
            points[i*3+0] = (-0.5+x,0.0,0.0+y)
            points[i*3+1] = (0.0+x,10.0,0.0+y)
            points[i*3+2] = (0.5+x,0.0,0.0+y)
    mesh.GetFaceVertexCountsAttr().Set(faceVertexCounts)
    mesh.GetFaceVertexIndicesAttr().Set(faceVertexIndices)
    mesh.GetPointsAttr().Set(points)
    mesh.GetDisplayColorAttr().Set(displayColor)
    mesh.GetDisplayOpacityAttr().Set(displayOpacity)

tests = ["small", "large", "proc"]

generate_files = False
render_tests = False
usd_memory_stats = True

if generate_files:
    stage = Usd.Stage.CreateNew("./furrypatch_small_geo.usd")
    main_prim = stage.DefinePrim("/main")
    mesh = UsdGeom.Mesh.Define(stage,main_prim.GetPath().AppendChild("mesh"))
    create_furrypatch_geo(mesh, 300, 300)
    stage.SetDefaultPrim(main_prim)
    stage.GetRootLayer().Save()

    stage = Usd.Stage.CreateNew("./furrypatch_large_geo.usd")
    main_prim = stage.DefinePrim("/main")
    mesh = UsdGeom.Mesh.Define(stage,main_prim.GetPath().AppendChild("mesh"))
    create_furrypatch_geo(mesh, 1000, 1000)
    stage.SetDefaultPrim(main_prim)
    stage.GetRootLayer().Save()

    for test in tests:
        stage_filename = f"./main_{test}.usda"
        stage = Usd.Stage.CreateNew(stage_filename)
        stage.SetStartTimeCode( 0.0 )
        stage.SetEndTimeCode( 120.0 )
        stage.SetFramesPerSecond( 24.0 )
        stage.SetTimeCodesPerSecond( 24.0 )

        # class prim for furry-patch definition
        #
        class_prim = stage.CreateClassPrim("/_class_")
        furrypatch_prim = stage.DefinePrim(class_prim.GetPath().AppendChild("furryPatch"))
        xform = UsdGeom.Xform.Define(stage,furrypatch_prim.GetPath().AppendChild("geo"))
        shape = UsdGeom.Mesh.Define(stage,xform.GetPath().AppendChild("shape"))
        if test in ["small","large"]:
            shape.GetPrim().GetReferences().AddReference(f"./furrypatch_{test}_geo.usd")
        else:
            shape.GetPrim().CreateAttribute("primvars:usifMeshes:test", Sdf.ValueTypeNames.Bool).Set(True)

        # elements inheriting from class definition
        # NOTE: not instanced for these tests to mimic scenario with
        #       hero-creatures and fur-patches
        #
        root_prim = UsdGeom.Scope.Define( stage, "/World" )
        px = 5
        pz = 5
        for z in range(pz):
            for x in range(px):
                xform = UsdGeom.Xform.Define( stage, root_prim.GetPath().AppendChild(f"prim_{x}_{z}") )
                xform.GetPrim().GetInherits().AddInherit( furrypatch_prim.GetPath() )
                xform.AddTransformOp().Set(
                    Gf.Matrix4d(
                        (1,0,0,0), 
                        (0,1,0,0), 
                        (0,0,1,0), 
                        (x*1000,0,z*1000)
                    ) 
                )

        # camera in a very specific transform/position
        #
        cam_xform = UsdGeom.Xform.Define(stage, "/camera_rig")
        translateOp = cam_xform.AddTranslateOp( opSuffix="zoomedIn" )
        rotateYOp = cam_xform.AddRotateYOp( opSuffix="zoomedIn" )
        rotateXOp = cam_xform.AddRotateXOp( opSuffix="zoomedIn" )
        translateOp.Set( Gf.Vec3d(121,36,121), 0.0)
        rotateYOp.Set( 45, 0.0 )
        rotateXOp.Set( -15, 0.0 )

        translateOp.Set( Gf.Vec3d(12140,3652,12140), 120.0)
        rotateYOp.Set( 45, 120.0 )
        rotateXOp.Set( -15, 120.0 )

        cam = stage.DefinePrim("/camera_rig/mono", "Camera")

        # just a dome-light
        # NOTE: empty texture will report error, don't worry about that, add a texture if you prefer
        #
        lights = UsdGeom.Xform.Define(stage, "/lights")
        sky = UsdLux.DomeLight.Define(stage, "/light/sky")
        sky.GetColorAttr().Set((1.0,1.0,1.0))
        sky.GetDiffuseAttr().Set(1.0)
        sky.GetEnableColorTemperatureAttr().Set(False)
        sky.GetExposureAttr().Set(1.0)
        sky.GetIntensityAttr().Set(1.0)

        stage.GetRootLayer().Save()


if render_tests:
    # render stages via husk with stats
    #
    for test in tests:
        stage_filename = f"./main_{test}.usda"
        output_filename = f"./main_{test}.exr"
        output_stats_filename = f"./main_{test}.stats.jpg"
        #os.environ["PXR_WORK_THREAD_LIMIT"] = "1"
        os.system( f"husk -f 120 -c /camera_rig/mono -R Karma -o {output_filename} --usd-input {stage_filename}" )
        os.system( f'renderstatsoverlay --align "top left" --overlay {output_filename} {output_stats_filename}' )

if usd_memory_stats:
    import psutil
    bytes_to_gb = 1024.0 * 1024.0 * 1024.0
    initial_avail = psutil.virtual_memory().available / bytes_to_gb
    def mem_snapshot(text):
        avail_gb = psutil.virtual_memory().available / bytes_to_gb
        total_gb = psutil.virtual_memory().total / bytes_to_gb
        curr_gb = initial_avail - avail_gb
        print( f"[{text:<40} curr:{curr_gb:.02f}gb total:{total_gb:.02f}gb ]"  )

    mem_snapshot("initial")
    for test in tests:
        stage_filename = f"./main_{test}.usda"
        mem_snapshot(f"Loading {stage_filename}")
        stage = Usd.Stage.Open(stage_filename)
        polys = 0
        meshes = 0
        for prim in stage.Traverse():
            if prim.GetTypeName() == "Mesh":
                meshes += 1
                polys += UsdGeom.Mesh(prim).GetFaceCount()
        mem_snapshot(f" - meshes:{meshes} polys:{polys}")
