
::cmake -G "Visual Studio 16 2019" .. -DCMAKE_PREFIX_PATH="C:\Program Files\Side Effects Software\Houdini 19.0.498\toolkit\cmake"

set parm_DCMAKE_PREFIX_PATH = "C:\Program Files\Side Effects Software\Houdini 19.0.498\toolkit\cmake"
cd SOP\SOP_FeE_Measure_3_0\build
cmake -G "Visual Studio 16 2019" .. -DCMAKE_PREFIX_PATH=parm_DCMAKE_PREFIX_PATH 


pause
