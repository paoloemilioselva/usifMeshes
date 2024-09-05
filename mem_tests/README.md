How to run the tests
====================

After you've compiled the usifMeshes plugin and installed it for Houdini-20.5.xxx, 
you can then launch the Houdini commandline tool.

![image](https://github.com/user-attachments/assets/48911066-8c2a-48f6-8cc0-e5fdc187bd51)

Then `cd` in the `mem_tests` folder and launch `hython mem_tests.py`.

This will generate and run 3 renders in `husk` also generating stats via `renderstatsoverlay`.

The `large` test takes about 6mins to run on my machine, and the others take just a few seconds.

When the tests are done, you should see this in the shell

![image](https://github.com/user-attachments/assets/1cf87222-3c6b-4e63-b360-dcaa3aa10d84)

and in the folder you should now see EXRs, JPGs and USD/USDA files.

![image](https://github.com/user-attachments/assets/94515128-1f06-4778-8048-b225ddcf301b)

The `small` and `large` test are producing a mesh up to 1M polys, added as a class then 
inherited by 25 elements in the scene.

The `proc` test is generating the same mesh with 1M polys in a SceneIndex Filtering plugin, 
just for Hydra delegates when asking for it in `GetPrim`.

The `usifMeshes` SceneIndex Filtering is just an example, not at all for production purpose, 
but to show the benefit of generating large data at Hydra time.

This can generally be done via a Hydra PrimAdapter, but I needed to also create a custom Usd-prim 
type, so more than one plugin... the SceneIndex plugin approach was definitely faster... sorry, I've been lazy.

The script generates a camera and a simple animation to go from closeup to a wider view, and 
the render for the `small` test should look like this:

![image](https://github.com/user-attachments/assets/3dd889e5-7a70-491d-a5c5-f33aae72f8fe)

whereas the renders from `large` and `proc` should look like this (with 1M polys per patch, 
filling the whole grid basically)

![image](https://github.com/user-attachments/assets/3c6e1bbb-6221-427e-9f74-b621ef7116a4)

The `renderstatsoverlay` generates the following files, in order, for `small`, `large` and then `proc`.

Small:
![image](https://github.com/user-attachments/assets/c7041b15-f93f-44f2-a6f4-f310ae8d98dd)

Large:
![image](https://github.com/user-attachments/assets/a0f9a604-f4ef-492d-867c-6a171dd2145a)

Proc:
![image](https://github.com/user-attachments/assets/30f2a5c9-adbb-4016-8549-7f09d4b0d4e6)

Timers aren't really accurate at the moment, I'm not completely sure they are distributed properly, 
and I'd like to create a better json/config for the `renderstatsoverlay` 
to be able to report loading the stage, various tasks in between (as much as possible) when 
getting from a usd-prim, through all the delegation steps, then into making hydra-prims
for the render-delegates.
Maybe if you, random developers happened to bump in this project, want to contribute with 
better stats, please!

NOTE: I've had to make a not-ideal "empty-mesh with another mesh as child" because that's the 
setup I've got in `usifMeshes` for now, I might change that later on, if I have time.



