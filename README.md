# Project Helltech
A futuristic fast-paced boomer shooter, built in UE5.
Made as part of the Indie Project for the masters program at UCM. 

## Contributing
This repository has some guidelines that should be followed to maintain consistency in the codebase.

### Guidelines
- The `main` branch isn't protected, but avoid pushing to it directly, create branches and pull requests to add changes to it.
- Code reviews are not enforced, but heavily recommended, please wait to have at least one approval by a codeowner before merging.

#### Content
- Follow Unreal Engine's official [Asset Naming Conventions](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects)
- Everything that isn't final should go in the `/Content/Game/Prototypes/` folder, under a specific folder with your prefered name.
- All prototyping assets should end with the initial of the developers name, as to clearly differentiate them in the Unreal Editor. 

#### Source
- Snake case should be used for all variables and function names, as to differentiate them from the UE5 standard.
  - `int foo_bar;`
  - `void foo_bar()`
- Function parameters should use `_` to symbolize whether they are input, output, or both, such as: 
  - `void foobar(int& _foo, const string& bar_, float& _foobar_);`
- All prototyping class names should end with the initial of the developers name, as to clearly differentiate them from final classes.
- The architecture of the project is based on the widespread usage of the Gameplay Ability System (GAS), please respect this when implementing any new features or changing existing ones.
